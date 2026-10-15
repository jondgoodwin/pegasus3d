/* A headless stand-in for libSDL3, for the Linux server only (`browser --serve`).
 *
 * WHY. The server is the same program as the browser, and the browser's packages (frame, window, audiodevice,
 * sdl) name SDL3 at link time. SDL3 is not in Ubuntu 24.04's archive, and the server needs none of what SDL is
 * for: no window, no GPU, no sound, no input. What `--serve` really asks of SDL is five things: SDL_Init
 * (events only), SDL_GetTicksNS and the performance counter (the clock), SDL_PollEvent (to hear a quit) and
 * SDL_Quit. Those are implemented here for real. Everything else (windows, displays, Vulkan surfaces, audio
 * streams) is unreachable in the server; it answers "failed" with a reason in SDL_GetError, so a program that
 * tries one (a client run on this machine by mistake) fails cleanly instead of crashing.
 *
 * SIGTERM and SIGINT become an SDL_EVENT_QUIT, as real SDL does, so `systemctl stop` ends the run in order.
 *
 * Build:  cc -O2 -fPIC -shared -o libSDL3.so sdl3shim.c      (see SERVER.md)
 * It exports exactly the SDL_ symbols sdl.cone's externs use that the browser's objects reference.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stddef.h>
#include <unistd.h>

#define EXPORT __attribute__((visibility("default")))
#define EVENT_QUIT 0x100u
#define INIT_EVENTS 0x4000u

typedef struct { uint32_t type, reserved; uint64_t timestamp; uint8_t buffer[112]; } Event; /* 128 bytes */
typedef struct { int x, y, w, h; } Rect;

static char errorText[128] = "";
static uint64_t startNs = 0;
static volatile sig_atomic_t quitAsked = 0;
static uint32_t initialised = 0;

static uint64_t nowNs(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}

static void setError(const char *what) {
  strncpy(errorText, what, sizeof errorText - 1);
  errorText[sizeof errorText - 1] = 0;
}

static bool unavailable(const char *what) {
  setError(what);
  return false;
}

static void onSignal(int sig) { (void)sig; quitAsked = 1; }

EXPORT const char *SDL_GetError(void) { return errorText; }
EXPORT bool SDL_ClearError(void) { errorText[0] = 0; return true; }

/* ---- the clock ---- */
EXPORT uint64_t SDL_GetTicksNS(void) {
  if (startNs == 0) startNs = nowNs();
  return nowNs() - startNs;
}
EXPORT uint64_t SDL_GetPerformanceCounter(void) { return nowNs(); }

/* ---- the program's folder, as real SDL gives it (with a trailing '/'): the bake cache stamps itself with the program ---- */
EXPORT const char *SDL_GetBasePath(void) {
  static char path[4096] = "";
  if (path[0] == 0) {
    ssize_t n = readlink("/proc/self/exe", path, sizeof path - 1);
    if (n <= 0) { path[0] = 0; unavailable("no program path"); return NULL; }
    path[n] = 0;
    char *slash = strrchr(path, '/');
    if (slash == NULL) { path[0] = 0; return NULL; }
    slash[1] = 0;
  }
  return path;
}
EXPORT uint64_t SDL_GetPerformanceFrequency(void) { return 1000000000ull; }
EXPORT void SDL_Delay(uint32_t ms) {
  struct timespec t = { ms / 1000, (long)(ms % 1000) * 1000000L };
  nanosleep(&t, NULL);
}

/* ---- starting and stopping: events only ---- */
EXPORT bool SDL_Init(uint32_t flags) {
  if (flags & ~INIT_EVENTS) return unavailable("this is the headless server build of SDL3: only SDL_INIT_EVENTS exists");
  SDL_GetTicksNS();
  if (!(initialised & INIT_EVENTS)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = onSignal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
  }
  initialised |= flags;
  return true;
}
EXPORT bool SDL_InitSubSystem(uint32_t flags) { return SDL_Init(flags); }
EXPORT void SDL_QuitSubSystem(uint32_t flags) { initialised &= ~flags; }
EXPORT void SDL_Quit(void) { initialised = 0; }

/* ---- events: a quit, once a signal asked for one ---- */
EXPORT bool SDL_PollEvent(Event *e) {
  if (quitAsked && e) {
    quitAsked = 0;
    memset(e, 0, sizeof *e);
    e->type = EVENT_QUIT;
    e->timestamp = SDL_GetTicksNS();
    return true;
  }
  return false;
}
EXPORT bool SDL_PushEvent(Event *e) { (void)e; return unavailable("no event queue in the headless server build of SDL3"); }

/* ---- what a server does not have ---- */
#define NOPE "not in the headless server build of SDL3 (no window, display, Vulkan or audio)"
EXPORT void *SDL_CreateWindow(const char *t, int w, int h, uint64_t f) { (void)t; (void)w; (void)h; (void)f; unavailable(NOPE); return NULL; }
EXPORT void SDL_DestroyWindow(void *w) { (void)w; }
EXPORT uint64_t SDL_GetWindowFlags(void *w) { (void)w; return 0; }
EXPORT bool SDL_GetWindowSize(void *w, int *x, int *y) { (void)w; (void)x; (void)y; return unavailable(NOPE); }
EXPORT bool SDL_GetWindowSizeInPixels(void *w, int *x, int *y) { (void)w; (void)x; (void)y; return unavailable(NOPE); }
EXPORT float SDL_GetWindowPixelDensity(void *w) { (void)w; return 1.0f; }
EXPORT float SDL_GetWindowDisplayScale(void *w) { (void)w; return 1.0f; }
EXPORT bool SDL_SetWindowTitle(void *w, const char *t) { (void)w; (void)t; return unavailable(NOPE); }
EXPORT bool SDL_SetWindowPosition(void *w, int x, int y) { (void)w; (void)x; (void)y; return unavailable(NOPE); }
EXPORT bool SDL_SetWindowSize(void *w, int x, int y) { (void)w; (void)x; (void)y; return unavailable(NOPE); }
EXPORT bool SDL_MinimizeWindow(void *w) { (void)w; return unavailable(NOPE); }
EXPORT bool SDL_RestoreWindow(void *w) { (void)w; return unavailable(NOPE); }
EXPORT bool SDL_SetWindowFullscreen(void *w, bool f) { (void)w; (void)f; return unavailable(NOPE); }
EXPORT bool SDL_StartTextInput(void *w) { (void)w; return unavailable(NOPE); }
EXPORT bool SDL_StopTextInput(void *w) { (void)w; return unavailable(NOPE); }
EXPORT uint32_t SDL_GetPrimaryDisplay(void) { unavailable(NOPE); return 0; }
EXPORT bool SDL_GetDisplayBounds(uint32_t d, Rect *r) { (void)d; (void)r; return unavailable(NOPE); }
EXPORT bool SDL_Vulkan_LoadLibrary(const char *p) { (void)p; return unavailable(NOPE); }
EXPORT void SDL_Vulkan_UnloadLibrary(void) {}
EXPORT const char *const *SDL_Vulkan_GetInstanceExtensions(uint32_t *n) { if (n) *n = 0; unavailable(NOPE); return NULL; }
EXPORT bool SDL_Vulkan_CreateSurface(void *w, void *i, void *a, uint64_t *s) { (void)w; (void)i; (void)a; (void)s; return unavailable(NOPE); }
EXPORT void *SDL_OpenAudioDeviceStream(uint32_t d, const void *spec, void *cb, void *u) { (void)d; (void)spec; (void)cb; (void)u; unavailable(NOPE); return NULL; }
EXPORT bool SDL_ResumeAudioStreamDevice(void *s) { (void)s; return unavailable(NOPE); }
EXPORT bool SDL_PutAudioStreamData(void *s, const void *b, int n) { (void)s; (void)b; (void)n; return unavailable(NOPE); }
EXPORT int SDL_GetAudioStreamQueued(void *s) { (void)s; return 0; }
EXPORT void SDL_DestroyAudioStream(void *s) { (void)s; }
