// cefshim.dll: a flat C API over the Chromium Embedded Framework's windowless (off-screen) browser, for the
// Cone incubator's web panel (guidebrowser/src/webpanel.cone). One thread (the incubator's main thread) calls it all;
// web_pump() runs CEF's message loop work once per frame.
#include <windows.h>
#include <string>
#include <vector>
#include <cstring>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_render_handler.h"
#include "include/cef_display_handler.h"
#include "include/wrapper/cef_helpers.h"

namespace {

struct View;
std::vector<View*> g_views;
bool g_inited = false;

class Client;

struct View {
  CefRefPtr<CefBrowser> browser;
  CefRefPtr<Client> client;
  int w = 0, h = 0;
  std::vector<unsigned char> bgra;   // the latest full frame
  int fw = 0, fh = 0;
  bool dirty = false;
  int dx0 = 0, dy0 = 0, dx1 = 0, dy1 = 0;   // the union of the rectangles painted since the last read (empty: dx1 <= dx0)
  bool closed = false;
  bool loading = false;
  bool wantsText = false;            // a text field holds the page's focus (OnVirtualKeyboardRequested says so)
  unsigned long paints = 0;
  std::string url;
  std::string title;
};

class App : public CefApp, public CefBrowserProcessHandler {
 public:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
  void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> cl) override {
    cl->AppendSwitch("mute-audio");
    cl->AppendSwitch("disable-smooth-scrolling");   // a wheel notch moves the page at once, by a known step: the panel shows that step before the paint (webpanel.cone)
    cl->AppendSwitch("disable-background-networking");
    cl->AppendSwitch("no-first-run");
    cl->AppendSwitch("disable-extensions");
    cl->AppendSwitchWithValue("autoplay-policy", "user-gesture-required");
  }
  IMPLEMENT_REFCOUNTING(App);
};

class Client : public CefClient, public CefRenderHandler, public CefLifeSpanHandler, public CefLoadHandler,
               public CefDisplayHandler {
 public:
  explicit Client(View* v) : v_(v) {}
  CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }

  void GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect) override { rect = CefRect(0, 0, v_->w, v_->h); }
  bool GetScreenInfo(CefRefPtr<CefBrowser>, CefScreenInfo& info) override {
    info.device_scale_factor = 1.0f;
    info.rect = CefRect(0, 0, v_->w, v_->h);
    info.available_rect = info.rect;
    return true;
  }
  void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width,
               int height) override {
    if (type != PET_VIEW) return;
    {
      int x0 = width, y0 = height, x1 = 0, y1 = 0;
      for (const CefRect& r : dirtyRects) {
        if (r.x < x0) x0 = r.x;
        if (r.y < y0) y0 = r.y;
        if (r.x + r.width > x1) x1 = r.x + r.width;
        if (r.y + r.height > y1) y1 = r.y + r.height;
      }
      if (x0 < 0) x0 = 0;
      if (y0 < 0) y0 = 0;
      if (x1 > width) x1 = width;
      if (y1 > height) y1 = height;
      if (x1 <= x0 || y1 <= y0 || v_->fw != width || v_->fh != height) { x0 = 0; y0 = 0; x1 = width; y1 = height; }
      if (v_->dirty && v_->dx1 > v_->dx0) {
        if (v_->dx0 < x0) x0 = v_->dx0;
        if (v_->dy0 < y0) y0 = v_->dy0;
        if (v_->dx1 > x1) x1 = v_->dx1;
        if (v_->dy1 > y1) y1 = v_->dy1;
      }
      v_->dx0 = x0; v_->dy0 = y0; v_->dx1 = x1; v_->dy1 = y1;
    }
    size_t n = (size_t)width * height * 4;
    if (v_->bgra.size() != n) v_->bgra.resize(n);
    memcpy(v_->bgra.data(), buffer, n);
    v_->fw = width; v_->fh = height; v_->dirty = true; v_->paints++;
  }
  // a link that wants a new window loads in this view instead
  bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame>, int, const CefString& target_url,
                     const CefString&, CefLifeSpanHandler::WindowOpenDisposition, bool, const CefPopupFeatures&,
                     CefWindowInfo&, CefRefPtr<CefClient>&, CefBrowserSettings&, CefRefPtr<CefDictionaryValue>&,
                     bool*) override {
    browser->GetMainFrame()->LoadURL(target_url);
    return true;
  }
  // a text field took the focus (any input mode but none) or lost it: the virtual keyboard follows (webpanel.cone, vkeys.cone)
  void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser>, TextInputMode mode) override { v_->wantsText = (mode != CEF_TEXT_INPUT_MODE_NONE); }
  void OnBeforeClose(CefRefPtr<CefBrowser>) override { v_->closed = true; }
  void OnLoadingStateChange(CefRefPtr<CefBrowser>, bool isLoading, bool, bool) override { v_->loading = isLoading; }
  void OnAddressChange(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame> frame, const CefString& url) override {
    if (frame->IsMain()) v_->url = url.ToString();
  }
  void OnTitleChange(CefRefPtr<CefBrowser>, const CefString& title) override { v_->title = title.ToString(); }
 private:
  View* v_;
  IMPLEMENT_REFCOUNTING(Client);
};

View* get(int i) { return (i >= 0 && i < (int)g_views.size()) ? g_views[i] : nullptr; }
CefRefPtr<CefBrowserHost> host(int i) {
  View* v = get(i);
  return (v && v->browser) ? v->browser->GetHost() : nullptr;
}

}  // namespace

extern "C" {

#define API __declspec(dllexport)

// 1 on success. cacheDir holds every file CEF writes; helperPath is cefhelper.exe.
API int web_init(const char* cacheDir, const char* helperPath) {
  if (g_inited) return 1;
  HINSTANCE inst = GetModuleHandle(nullptr);
  CefMainArgs args(inst);
  CefSettings s;
  s.no_sandbox = true;
  s.windowless_rendering_enabled = true;
  s.multi_threaded_message_loop = false;
  s.log_severity = LOGSEVERITY_WARNING;
  char helperFull[MAX_PATH];
  GetFullPathNameA(helperPath, MAX_PATH, helperFull, nullptr);
  CefString(&s.browser_subprocess_path) = helperFull;
  char full[MAX_PATH];
  GetFullPathNameA(cacheDir, MAX_PATH, full, nullptr);
  CreateDirectoryA(full, nullptr);
  CefString(&s.root_cache_path) = full;
  CefString(&s.cache_path) = full;
  std::string log = std::string(full) + "\\cef.log";
  CefString(&s.log_file) = log;
  CefRefPtr<App> app(new App);
  g_inited = CefInitialize(args, s, app, nullptr);
  return g_inited ? 1 : 0;
}

API int web_open(const char* url, int w, int h) {
  if (!g_inited) return -1;
  View* v = new View;
  v->w = w; v->h = h;
  v->client = new Client(v);
  CefWindowInfo wi;
  wi.SetAsWindowless(nullptr);
  CefBrowserSettings bs;
  bs.windowless_frame_rate = 60;
  v->browser = CefBrowserHost::CreateBrowserSync(wi, v->client, url, bs, nullptr, nullptr);
  if (!v->browser) { delete v; return -1; }
  v->browser->GetHost()->SetFocus(true);
  g_views.push_back(v);
  return (int)g_views.size() - 1;
}

API void web_pump() { if (g_inited) CefDoMessageLoopWork(); }

// BGRA of the latest paint into dst (w*h*4 bytes; swapped to RGBA); returns the byte count if there is a new
// frame, else 0. *w and *h give its size.
API int web_frame(int view, unsigned char* dst, int cap, int* w, int* h) {
  View* v = get(view);
  if (!v || !v->dirty) return 0;
  size_t n = (size_t)v->fw * v->fh * 4;
  if ((int)n > cap) return 0;
  // BGRA to RGBA, and rows bottom to top: the incubator's page pass has v = 0 at the bottom
  const unsigned char* base = v->bgra.data();
  size_t row = (size_t)v->fw * 4;
  for (int y = 0; y < v->fh; y++) {
    const unsigned char* s = base + (size_t)(v->fh - 1 - y) * row;
    unsigned char* d = dst + (size_t)y * row;
    for (size_t i = 0; i < row; i += 4) { d[i] = s[i + 2]; d[i + 1] = s[i + 1]; d[i + 2] = s[i]; d[i + 3] = 255; }
  }
  *w = v->fw; *h = v->fh;
  v->dirty = false;
  return (int)n;
}

// Only the rectangle painted since the last read, as RGBA rows from the bottom of the view (the texture's row 0), packed
// (w*4 bytes a row) into dst. *x, *y are the rectangle's corner in TEXTURE pixels (y counted from the bottom), *w, *h its size;
// *fw, *fh the whole frame. Returns the byte count, or 0 if nothing new (or dst too small).
API int web_frame_rect(int view, unsigned char* dst, int cap, int* x, int* y, int* w, int* h, int* fw, int* fh) {
  View* v = get(view);
  if (!v || !v->dirty) return 0;
  int rw = v->dx1 - v->dx0, rh = v->dy1 - v->dy0;
  if (rw <= 0 || rh <= 0) { v->dirty = false; return 0; }
  if ((long long)rw * rh * 4 > cap) return 0;
  const unsigned char* base = v->bgra.data();
  size_t row = (size_t)v->fw * 4;
  for (int j = 0; j < rh; j++) {
    int srcRow = v->dy1 - 1 - j;           // the bottom row of the rectangle first
    const unsigned char* s = base + (size_t)srcRow * row + (size_t)v->dx0 * 4;
    unsigned char* d = dst + (size_t)j * rw * 4;
    for (int i = 0; i < rw * 4; i += 4) { d[i] = s[i + 2]; d[i + 1] = s[i + 1]; d[i + 2] = s[i]; d[i + 3] = 255; }
  }
  *x = v->dx0; *y = v->fh - v->dy1; *w = rw; *h = rh; *fw = v->fw; *fh = v->fh;
  v->dirty = false; v->dx0 = v->dy0 = v->dx1 = v->dy1 = 0;
  return rw * rh * 4;
}

// SDL only sends text events while text input is on for the window with the keyboard focus (SDL3.dll is the
// incubator's own; found at run time so the shim does not link it)
API void web_text_input(int on) {
  HMODULE m = GetModuleHandleA("SDL3.dll");
  if (!m) return;
  typedef void* (*GetFocus_t)();
  typedef bool (*Text_t)(void*);
  GetFocus_t gf = (GetFocus_t)GetProcAddress(m, "SDL_GetKeyboardFocus");
  Text_t f = (Text_t)GetProcAddress(m, on ? "SDL_StartTextInput" : "SDL_StopTextInput");
  if (gf && f) f(gf());
}

API unsigned long web_paints(int view) { View* v = get(view); return v ? v->paints : 0; }
API int web_wants_text(int view) { View* v = get(view); return v && v->wantsText ? 1 : 0; }
API int web_loading(int view) { View* v = get(view); return v && v->loading ? 1 : 0; }

static int mods_to_cef(int m) {
  int r = 0;
  if (m & 1) r |= EVENTFLAG_SHIFT_DOWN;
  if (m & 2) r |= EVENTFLAG_CONTROL_DOWN;
  if (m & 4) r |= EVENTFLAG_ALT_DOWN;
  return r;
}

API void web_mouse_move(int view, int x, int y, int leave) {
  auto h = host(view); if (!h) return;
  CefMouseEvent e; e.x = x; e.y = y; e.modifiers = 0;
  h->SendMouseMoveEvent(e, leave != 0);
}
// button: 1 left, 2 middle, 3 right
API void web_mouse_button(int view, int x, int y, int button, int up, int clicks) {
  auto h = host(view); if (!h) return;
  CefMouseEvent e; e.x = x; e.y = y; e.modifiers = 0;
  CefBrowserHost::MouseButtonType t = button == 3 ? MBT_RIGHT : button == 2 ? MBT_MIDDLE : MBT_LEFT;
  h->SetFocus(true);
  // (CEF says when a text field wants the keyboard, never when it stops: each press clears it, and a press on a field says it again)
  if (!up) { View* v = get(view); if (v) v->wantsText = false; }
  h->SendMouseClickEvent(e, t, up != 0, clicks);
}
API void web_wheel(int view, int x, int y, int dx, int dy) {
  auto h = host(view); if (!h) return;
  CefMouseEvent e; e.x = x; e.y = y; e.modifiers = 0;
  h->SendMouseWheelEvent(e, dx, dy);
}
// a key by Windows virtual-key code; mods: 1 shift, 2 ctrl, 4 alt
API void web_key(int view, int vk, int down, int mods) {
  auto h = host(view); if (!h) return;
  CefKeyEvent e;
  e.type = down ? KEYEVENT_RAWKEYDOWN : KEYEVENT_KEYUP;
  e.windows_key_code = vk;
  e.native_key_code = (MapVirtualKey(vk, MAPVK_VK_TO_VSC) << 16) | 1;
  e.modifiers = mods_to_cef(mods);
  h->SendKeyEvent(e);
}
// a typed character (UTF-16 unit)
API void web_char(int view, int ch, int mods) {
  auto h = host(view); if (!h) return;
  CefKeyEvent e;
  e.type = KEYEVENT_CHAR;
  e.windows_key_code = ch;
  e.native_key_code = 0;
  e.character = (char16_t)ch;
  e.unmodified_character = (char16_t)ch;
  e.modifiers = mods_to_cef(mods);
  h->SendKeyEvent(e);
}
API void web_go(int view, const char* url) { View* v = get(view); if (v && v->browser) v->browser->GetMainFrame()->LoadURL(url); }
API void web_back(int view) { View* v = get(view); if (v && v->browser && v->browser->CanGoBack()) v->browser->GoBack(); }
API void web_forward(int view) { View* v = get(view); if (v && v->browser && v->browser->CanGoForward()) v->browser->GoForward(); }
API int web_can_back(int view) { View* v = get(view); return v && v->browser && v->browser->CanGoBack(); }
API int web_can_forward(int view) { View* v = get(view); return v && v->browser && v->browser->CanGoForward(); }
// the address now shown, UTF-8, into out (cap bytes); returns its length
API int web_url(int view, char* out, int cap) {
  View* v = get(view);
  if (!v || cap < 1) return 0;
  int n = (int)v->url.size(); if (n > cap - 1) n = cap - 1;
  memcpy(out, v->url.data(), n); out[n] = 0;
  return n;
}
API int web_title(int view, char* out, int cap) {
  View* v = get(view);
  if (!v || cap < 1) return 0;
  int n = (int)v->title.size(); if (n > cap - 1) n = cap - 1;
  memcpy(out, v->title.data(), n); out[n] = 0;
  return n;
}
API void web_close(int view) {
  View* v = get(view);
  if (v && v->browser && !v->closed) v->browser->GetHost()->CloseBrowser(true);
}
// closes every view, pumps until they are gone, and shuts CEF down
API void web_shutdown() {
  if (!g_inited) return;
  for (View* v : g_views) if (v->browser && !v->closed) v->browser->GetHost()->CloseBrowser(true);
  for (int i = 0; i < 500; i++) {
    bool all = true;
    for (View* v : g_views) if (!v->closed) all = false;
    if (all) break;
    CefDoMessageLoopWork();
    Sleep(10);
  }
  for (View* v : g_views) { v->browser = nullptr; v->client = nullptr; }
  for (int i = 0; i < 20; i++) { CefDoMessageLoopWork(); Sleep(10); }
  CefShutdown();
  g_inited = false;
}

}  // extern "C"
