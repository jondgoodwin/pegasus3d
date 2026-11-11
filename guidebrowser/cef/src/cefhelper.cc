// cefhelper.exe: the program CEF starts for its renderer, GPU and utility processes, so the incubator's own
// executable is never relaunched. It does nothing but run CEF's subprocess code.
#include <windows.h>
#include "include/cef_app.h"

int APIENTRY wWinMain(HINSTANCE inst, HINSTANCE, LPWSTR, int) {
  CefMainArgs args(inst);
  return CefExecuteProcess(args, nullptr, nullptr);
}
