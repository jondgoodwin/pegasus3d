// deskshim.dll: the laptop's primary screen for the incubator's desk panel (DXGI Desktop Duplication), and the real pointer
// (SendInput) when the panel is allowed to control it. A flat C API for Cone: ints and a byte pointer only.
//
//   desk_open(output)        1 on success; output -1 = the output holding the desktop's origin (the primary), else the nth
//   desk_update(timeout_ms)  1 a new picture is in desk_pixels(), 0 nothing changed, -1 lost (reopened on the next call)
//   desk_pixels()            width*height BGRA bytes (sRGB), the cursor drawn in; valid until the next desk_update
//   desk_width/height, desk_origin_x/y (the output's place on the virtual desktop), desk_cursor_x/y/visible
//   desk_update_us()         microseconds the last desk_update took (acquire + copy to the CPU + cursor)
//   desk_set_dry_run(1)      desk_move/desk_button/desk_wheel only PRINT what they would send (no input is made)
//   desk_move(px,py)         the real pointer to pixel (px,py) of the captured output
//   desk_button(b,down)      b 0 left, 1 right, 2 middle;  desk_wheel(notches)  one notch = 120
//   desk_close()
#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DESK_API __declspec(dllexport)

static ID3D11Device *dev;
static ID3D11DeviceContext *ctx;
static IDXGIOutputDuplication *dupl;
static ID3D11Texture2D *stage;
static int W, H, OX, OY, curOutput = -1;
static unsigned char *outbuf;               // the screen as copied, with the cursor drawn in
static int curX, curY, curVis, haveShape, shapeW, shapeH, shapeType, shapePitch;
static unsigned char *shape;
static int shapeCap;
static unsigned char *meta;                 // the frame's dirty and move rectangles
static int metaCap;
static LARGE_INTEGER freq;
static double lastUs;
static int dryRun, haveFrame;

static void release_all(void) {
  if (stage) { ID3D11Texture2D_Release(stage); stage = 0; }
  if (dupl) { IDXGIOutputDuplication_Release(dupl); dupl = 0; }
  if (ctx) { ID3D11DeviceContext_Release(ctx); ctx = 0; }
  if (dev) { ID3D11Device_Release(dev); dev = 0; }
}

static int open_dup(int output) {
  SetProcessDPIAware();
  QueryPerformanceFrequency(&freq);
  release_all();
  curOutput = output;
  IDXGIFactory1 *fac = 0;
  if (FAILED(CreateDXGIFactory1(&IID_IDXGIFactory1, (void **)&fac))) return 0;
  IDXGIAdapter1 *ad = 0;
  IDXGIOutput *out = 0;
  int n = 0, found = 0;
  for (UINT a = 0; !found && IDXGIFactory1_EnumAdapters1(fac, a, &ad) == S_OK; a++) {
    for (UINT o = 0; !found && IDXGIAdapter1_EnumOutputs(ad, o, &out) == S_OK; o++) {
      DXGI_OUTPUT_DESC d;
      IDXGIOutput_GetDesc(out, &d);
      int want = output < 0 ? (d.DesktopCoordinates.left == 0 && d.DesktopCoordinates.top == 0) : (n == output);
      n++;
      if (want && d.AttachedToDesktop) {
        found = 1;
        OX = d.DesktopCoordinates.left; OY = d.DesktopCoordinates.top;
        W = d.DesktopCoordinates.right - OX; H = d.DesktopCoordinates.bottom - OY;
        break;
      }
      IDXGIOutput_Release(out);
    }
    if (!found) IDXGIAdapter1_Release(ad);
  }
  IDXGIFactory1_Release(fac);
  if (!found) return 0;
  D3D_FEATURE_LEVEL fl;
  HRESULT hr = D3D11CreateDevice((IDXGIAdapter *)ad, D3D_DRIVER_TYPE_UNKNOWN, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 0, 0,
                                 D3D11_SDK_VERSION, &dev, &fl, &ctx);
  IDXGIAdapter1_Release(ad);
  if (FAILED(hr)) { IDXGIOutput_Release(out); return 0; }
  IDXGIOutput1 *out1 = 0;
  hr = IDXGIOutput_QueryInterface(out, &IID_IDXGIOutput1, (void **)&out1);
  IDXGIOutput_Release(out);
  if (FAILED(hr)) return 0;
  hr = IDXGIOutput1_DuplicateOutput(out1, (IUnknown *)dev, &dupl);
  IDXGIOutput1_Release(out1);
  if (FAILED(hr)) { fprintf(stderr, "deskshim: DuplicateOutput failed 0x%08lx\n", (unsigned long)hr); fflush(stderr); return 0; }
  D3D11_TEXTURE2D_DESC td;
  memset(&td, 0, sizeof td);
  td.Width = W; td.Height = H; td.MipLevels = 1; td.ArraySize = 1; td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  td.SampleDesc.Count = 1; td.Usage = D3D11_USAGE_STAGING; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  if (FAILED(ID3D11Device_CreateTexture2D(dev, &td, 0, &stage))) return 0;
  free(outbuf);
  outbuf = (unsigned char *)calloc((size_t)W * H, 4);
  haveFrame = 0;
  return 1;
}

// The pixels the cursor covers are kept, so the next change puts them back (no whole-screen copy to redraw the cursor)
static unsigned char *under;
static int underX, underY, underW, underH, underOk;

static void restore_under(void) {
  if (!underOk) return;
  underOk = 0;
  if (underX < 0 || underY < 0 || underX + underW > W || underY + underH > H) return;   // the output was reopened at another size
  for (int y = 0; y < underH; y++) memcpy(outbuf + ((size_t)(underY + y) * W + underX) * 4, under + (size_t)y * underW * 4, (size_t)underW * 4);
  underOk = 0;
}

static void compose_cursor(void) {
  if (!curVis || !haveShape) return;
  int mono = shapeType == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME;
  int sh = mono ? shapeH / 2 : shapeH;
  {
    int x0 = curX < 0 ? 0 : curX, y0 = curY < 0 ? 0 : curY;
    int x1 = curX + shapeW > W ? W : curX + shapeW, y1 = curY + sh > H ? H : curY + sh;
    if (x1 <= x0 || y1 <= y0) return;
    underX = x0; underY = y0; underW = x1 - x0; underH = y1 - y0;
    free(under);
    under = (unsigned char *)malloc((size_t)underW * underH * 4);
    for (int y = 0; y < underH; y++) memcpy(under + (size_t)y * underW * 4, outbuf + ((size_t)(underY + y) * W + underX) * 4, (size_t)underW * 4);
    underOk = 1;
  }
  for (int y = 0; y < sh; y++) {
    int dy = curY + y;
    if (dy < 0 || dy >= H) continue;
    for (int x = 0; x < shapeW; x++) {
      int dx = curX + x;
      if (dx < 0 || dx >= W) continue;
      unsigned char *d = outbuf + ((size_t)dy * W + dx) * 4;
      if (mono) {
        int bit = 0x80 >> (x & 7);
        int a = (shape[y * shapePitch + (x >> 3)] & bit) != 0;
        int xr = (shape[(y + sh) * shapePitch + (x >> 3)] & bit) != 0;
        for (int c = 0; c < 3; c++) {
          int v = d[c];
          if (a == 0) v = xr ? 255 : 0; else if (xr) v = 255 - v;
          d[c] = (unsigned char)v;
        }
      } else {
        unsigned char *s = shape + (size_t)y * shapePitch + x * 4;
        if (shapeType == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) {
          int al = s[3];
          for (int c = 0; c < 3; c++) d[c] = (unsigned char)((s[c] * al + d[c] * (255 - al)) / 255);
        } else {   // masked colour: alpha byte 0 replaces, 0xff xors
          for (int c = 0; c < 3; c++) d[c] = s[3] ? (unsigned char)(d[c] ^ s[c]) : s[c];
        }
      }
    }
  }
}

// One turn of the capture thread's work: wait up to timeout_ms for the screen to change, and copy what changed into 'outbuf'
static int step(int timeout_ms) {
  LARGE_INTEGER t0, t1;
  QueryPerformanceCounter(&t0);
  if (!dupl && !open_dup(curOutput)) return -1;
  DXGI_OUTDUPL_FRAME_INFO info;
  IDXGIResource *res = 0;
  HRESULT hr = IDXGIOutputDuplication_AcquireNextFrame(dupl, (UINT)timeout_ms, &info, &res);
  if (hr == DXGI_ERROR_WAIT_TIMEOUT) return 0;
  if (FAILED(hr)) { release_all(); return -1; }       // access lost (a mode change, a secure desktop): reopened on the next call
  QueryPerformanceCounter(&t0);                       // (the cost counted is the work, not the wait for a picture)
  int changed = 0;
  restore_under();
  if (info.LastPresentTime.QuadPart != 0) {
    ID3D11Texture2D *tex = 0;
    if (SUCCEEDED(IDXGIResource_QueryInterface(res, &IID_ID3D11Texture2D, (void **)&tex))) {
      // the changed rectangles only (the whole screen for the first picture, or if anything moved)
      int full = !haveFrame, nd = 0;
      if (!full && info.TotalMetadataBufferSize > 0) {
        if ((int)info.TotalMetadataBufferSize > metaCap) { free(meta); metaCap = (int)info.TotalMetadataBufferSize; meta = (unsigned char *)malloc(metaCap); }
        UINT mb = 0, db = 0;
        if (FAILED(IDXGIOutputDuplication_GetFrameMoveRects(dupl, metaCap, (DXGI_OUTDUPL_MOVE_RECT *)meta, &mb)) || mb > 0) full = 1;
        else if (FAILED(IDXGIOutputDuplication_GetFrameDirtyRects(dupl, metaCap, (RECT *)meta, &db))) full = 1;
        else nd = (int)(db / sizeof(RECT));
      } else if (!full) full = 1;
      RECT *dr = (RECT *)meta;
      if (full) {
        ID3D11DeviceContext_CopyResource(ctx, (ID3D11Resource *)stage, (ID3D11Resource *)tex);
      } else {
        for (int i = 0; i < nd; i++) {
          D3D11_BOX b; b.left = dr[i].left; b.top = dr[i].top; b.right = dr[i].right; b.bottom = dr[i].bottom; b.front = 0; b.back = 1;
          ID3D11DeviceContext_CopySubresourceRegion(ctx, (ID3D11Resource *)stage, 0, b.left, b.top, 0, (ID3D11Resource *)tex, 0, &b);
        }
      }
      ID3D11Texture2D_Release(tex);
      D3D11_MAPPED_SUBRESOURCE m;
      if (SUCCEEDED(ID3D11DeviceContext_Map(ctx, (ID3D11Resource *)stage, 0, D3D11_MAP_READ, 0, &m))) {
        if (full) {
          for (int y = 0; y < H; y++) memcpy(outbuf + (size_t)y * W * 4, (unsigned char *)m.pData + (size_t)y * m.RowPitch, (size_t)W * 4);
        } else {
          for (int i = 0; i < nd; i++) {
            int x0 = dr[i].left < 0 ? 0 : dr[i].left, x1 = dr[i].right > W ? W : dr[i].right;
            int y0 = dr[i].top < 0 ? 0 : dr[i].top, y1 = dr[i].bottom > H ? H : dr[i].bottom;
            for (int y = y0; y < y1 && x1 > x0; y++)
              memcpy(outbuf + ((size_t)y * W + x0) * 4, (unsigned char *)m.pData + (size_t)y * m.RowPitch + (size_t)x0 * 4, (size_t)(x1 - x0) * 4);
          }
        }
        ID3D11DeviceContext_Unmap(ctx, (ID3D11Resource *)stage, 0);
        changed = 1; haveFrame = 1;
      }
    }
  }
  if (info.LastMouseUpdateTime.QuadPart != 0) {
    curVis = info.PointerPosition.Visible;
    curX = info.PointerPosition.Position.x;
    curY = info.PointerPosition.Position.y;
    changed = 1;
  }
  if (info.PointerShapeBufferSize > 0) {
    if ((int)info.PointerShapeBufferSize > shapeCap) {
      free(shape);
      shapeCap = (int)info.PointerShapeBufferSize;
      shape = (unsigned char *)malloc(shapeCap);
    }
    UINT need;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO si;
    if (SUCCEEDED(IDXGIOutputDuplication_GetFramePointerShape(dupl, shapeCap, shape, &need, &si))) {
      shapeW = si.Width; shapeH = si.Height; shapeType = si.Type; shapePitch = si.Pitch; haveShape = 1;
      changed = 1;
    }
  }
  IDXGIResource_Release(res);
  IDXGIOutputDuplication_ReleaseFrame(dupl);
  if (changed && haveFrame) compose_cursor();
  QueryPerformanceCounter(&t1);
  lastUs = (double)(t1.QuadPart - t0.QuadPart) * 1e6 / (double)freq.QuadPart;
  return changed && haveFrame;
}

// ---- the capture thread and the hand-over ----
// The thread waits for the screen, copies it, and at most 30 times a second publishes a finished picture with its mip chain
// (level 0 first, each level half the one before, tightly packed: what the GPU upload wants). Three buffers move by swap
// under a lock: 'spare' (the thread's), 'pubbuf' (the latest finished) and 'held' (the render loop's, until its next update).
typedef struct { unsigned char *p; size_t cap; } Buf;
static Buf spare, pubbuf, held;
static int pubW, pubH, heldW, heldH, fresh, pubUs, heldUs, pubLevels, heldLevels;
static CRITICAL_SECTION cs;
static HANDLE thr;
static volatile LONG stopping;
static int csReady;
#define MIN_INTERVAL_MS 33

static int levels_of(int w, int h) { int n = 1; while (w > 1 || h > 1) { w = w > 1 ? w / 2 : 1; h = h > 1 ? h / 2 : 1; n++; } return n; }

static size_t chain_bytes(int w, int h) {
  size_t t = 0;
  for (;;) { t += (size_t)w * h * 4; if (w <= 1 && h <= 1) break; w = w > 1 ? w / 2 : 1; h = h > 1 ? h / 2 : 1; }
  return t;
}

// Each level from the one before: the mean of 2x2 texels (in the stored sRGB values, which is cheap and close enough)
static void build_chain(unsigned char *dst, const unsigned char *src, int w, int h) {
  memcpy(dst, src, (size_t)w * h * 4);
  const unsigned char *prev = dst;
  unsigned char *cur = dst + (size_t)w * h * 4;
  while (w > 1 || h > 1) {
    int nw = w > 1 ? w / 2 : 1, nh = h > 1 ? h / 2 : 1;
    for (int y = 0; y < nh; y++) {
      int y0 = y * 2, y1 = y0 + 1 < h ? y0 + 1 : y0;
      for (int x = 0; x < nw; x++) {
        int x0 = x * 2, x1 = x0 + 1 < w ? x0 + 1 : x0;
        const unsigned char *a = prev + ((size_t)y0 * w + x0) * 4, *b = prev + ((size_t)y0 * w + x1) * 4;
        const unsigned char *c = prev + ((size_t)y1 * w + x0) * 4, *d = prev + ((size_t)y1 * w + x1) * 4;
        unsigned char *o = cur + ((size_t)y * nw + x) * 4;
        for (int k = 0; k < 4; k++) o[k] = (unsigned char)((a[k] + b[k] + c[k] + d[k] + 2) >> 2);
      }
    }
    prev = cur; cur += (size_t)nw * nh * 4; w = nw; h = nh;
  }
}

static void publish(double stepUs) {
  LARGE_INTEGER t0, t1;
  QueryPerformanceCounter(&t0);
  int w = W, h = H;
  size_t need = chain_bytes(w, h);
  if (spare.cap < need) { free(spare.p); spare.p = (unsigned char *)malloc(need); spare.cap = spare.p ? need : 0; }
  if (!spare.p) return;
  build_chain(spare.p, outbuf, w, h);
  QueryPerformanceCounter(&t1);
  EnterCriticalSection(&cs);
  Buf t = spare; spare = pubbuf; pubbuf = t;
  pubW = w; pubH = h; pubLevels = levels_of(w, h); fresh = 1;
  pubUs = (int)(stepUs + (double)(t1.QuadPart - t0.QuadPart) * 1e6 / (double)freq.QuadPart);
  LeaveCriticalSection(&cs);
}

static DWORD WINAPI capture_loop(LPVOID unused) {
  LARGE_INTEGER last, now;
  last.QuadPart = 0;
  int pending = 0;
  double stepUs = 0;
  (void)unused;
  while (!stopping) {
    int r = step(MIN_INTERVAL_MS);
    if (r < 0) { Sleep(100); continue; }
    if (r == 1) { pending = 1; stepUs += lastUs; }
    if (pending) {
      QueryPerformanceCounter(&now);
      if (last.QuadPart == 0 || (now.QuadPart - last.QuadPart) * 1000 / freq.QuadPart >= MIN_INTERVAL_MS) {
        publish(stepUs);
        pending = 0; stepUs = 0; last = now;
      }
    }
  }
  return 0;
}

DESK_API int desk_open(int output) {
  if (thr) return 1;
  if (!open_dup(output)) return 0;
  if (!csReady) { InitializeCriticalSection(&cs); csReady = 1; }
  stopping = 0;
  thr = CreateThread(0, 0, capture_loop, 0, 0, 0);
  return thr != 0;
}

// The render loop's call: 1 if a finished picture was waiting (now in desk_pixels), else 0. Never waits for the screen.
DESK_API int desk_update(int timeout_ms) {
  int r = 0;
  (void)timeout_ms;
  if (!thr) return -1;
  EnterCriticalSection(&cs);
  if (fresh) {
    Buf t = held; held = pubbuf; pubbuf = t;
    heldW = pubW; heldH = pubH; heldLevels = pubLevels; heldUs = pubUs; fresh = 0; r = 1;
  }
  LeaveCriticalSection(&cs);
  return r;
}

DESK_API const unsigned char *desk_pixels(void) { return held.p; }
DESK_API int desk_levels(void) { return heldLevels; }
DESK_API int desk_width(void) { return heldW; }
DESK_API int desk_height(void) { return heldH; }
DESK_API int desk_origin_x(void) { return OX; }
DESK_API int desk_origin_y(void) { return OY; }
DESK_API int desk_cursor_x(void) { return curX; }
DESK_API int desk_cursor_y(void) { return curY; }
DESK_API int desk_cursor_visible(void) { return curVis; }
DESK_API int desk_update_us(void) { return heldUs; }   // the capture thread's work for the picture last taken (copy and mip chain)
DESK_API void desk_set_dry_run(int on) { dryRun = on; }

DESK_API void desk_move(int px, int py) {
  int sx = OX + px, sy = OY + py;
  int vx = GetSystemMetrics(SM_XVIRTUALSCREEN), vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN), vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  long nx = (long)(((long long)(sx - vx) * 65535 + (vw - 1) / 2) / (vw > 1 ? vw - 1 : 1));
  long ny = (long)(((long long)(sy - vy) * 65535 + (vh - 1) / 2) / (vh > 1 ? vh - 1 : 1));
  if (dryRun) {
    printf("deskshim: dry-run move to pixel (%d, %d) = screen (%d, %d) = absolute (%ld, %ld) of 65535 over the virtual desktop %dx%d at (%d, %d)\n",
           px, py, sx, sy, nx, ny, vw, vh, vx, vy);
    fflush(stdout);
    return;
  }
  INPUT in;
  memset(&in, 0, sizeof in);
  in.type = INPUT_MOUSE;
  in.mi.dx = nx; in.mi.dy = ny;
  in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
  SendInput(1, &in, sizeof(INPUT));
}

DESK_API void desk_button(int b, int down) {
  DWORD f = b == 1 ? (down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP)
          : b == 2 ? (down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP)
                   : (down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
  if (dryRun) { printf("deskshim: dry-run button %d %s\n", b, down ? "down" : "up"); fflush(stdout); return; }
  INPUT in;
  memset(&in, 0, sizeof in);
  in.type = INPUT_MOUSE;
  in.mi.dwFlags = f;
  SendInput(1, &in, sizeof(INPUT));
}

DESK_API void desk_wheel(int notches) {
  if (dryRun) { printf("deskshim: dry-run wheel %d notches\n", notches); fflush(stdout); return; }
  INPUT in;
  memset(&in, 0, sizeof in);
  in.type = INPUT_MOUSE;
  in.mi.dwFlags = MOUSEEVENTF_WHEEL;
  in.mi.mouseData = (DWORD)(notches * 120);
  SendInput(1, &in, sizeof(INPUT));
}

// A key of the keyboard (virtual-key code vk), pressed or released. The arrows and the keys beside them are the extended ones.
DESK_API void desk_key(int vk, int down) {
  if (dryRun) { printf("deskshim: dry-run key vk %d %s\n", vk, down ? "down" : "up"); fflush(stdout); return; }
  INPUT in;
  memset(&in, 0, sizeof in);
  in.type = INPUT_KEYBOARD;
  in.ki.wVk = (WORD)vk;
  in.ki.wScan = (WORD)MapVirtualKeyW((UINT)vk, MAPVK_VK_TO_VSC);
  in.ki.dwFlags = (down ? 0 : KEYEVENTF_KEYUP);
  if ((vk >= 33 && vk <= 40) || vk == 45 || vk == 46) in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
  SendInput(1, &in, sizeof(INPUT));
}

// One character (a UTF-16 unit), typed whatever the layout: down then up as a Unicode key
DESK_API void desk_put(int cp) {
  if (dryRun) { printf("deskshim: dry-run char U+%04X\n", cp); fflush(stdout); return; }
  INPUT in[2];
  memset(in, 0, sizeof in);
  in[0].type = INPUT_KEYBOARD; in[0].ki.wScan = (WORD)cp; in[0].ki.dwFlags = KEYEVENTF_UNICODE;
  in[1].type = INPUT_KEYBOARD; in[1].ki.wScan = (WORD)cp; in[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
  SendInput(2, in, sizeof(INPUT));
}

DESK_API void desk_close(void) {
  if (thr) { stopping = 1; WaitForSingleObject(thr, 2000); CloseHandle(thr); thr = 0; }
  free(spare.p); free(pubbuf.p); free(held.p);
  spare.p = pubbuf.p = held.p = 0; spare.cap = pubbuf.cap = held.cap = 0; fresh = 0;
  release_all();
  free(outbuf); free(shape); free(meta); free(under);
  outbuf = shape = meta = under = 0;
  metaCap = 0; underOk = 0; haveFrame = 0;
  shapeCap = 0;
}

// ---- small helpers of the page editor (incubator notes) ----
// desk_local_part(k): the local time's year (0), month (1), day (2), hour (3) or minute (4)
DESK_API int desk_local_part(int k) {
  SYSTEMTIME t; GetLocalTime(&t);
  switch (k) { case 0: return t.wYear; case 1: return t.wMonth; case 2: return t.wDay; case 3: return t.wHour; default: return t.wMinute; }
}

// desk_replace_file(from, to): rename 'from' over 'to' in one step (Windows will not rename over a file otherwise); 1 on success
DESK_API int desk_replace_file(const char *from, const char *to) {
  return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) ? 1 : 0;
}
