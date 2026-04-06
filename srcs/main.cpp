#include "webview.h"
#include "ModeManager.hpp"
#include "AppBridge.hpp"
#include <string>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
// Force GUI subsystem — suppresses the console window on double-click
#  pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#else
#  include <unistd.h>
#  include <climits>
#endif

static std::string getExeDir() {
#ifdef _WIN32
    char buf[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string s(buf);
    size_t pos = s.rfind('\\');
    if (pos != std::string::npos) s = s.substr(0, pos);
    for (char& c : s) if (c == '\\') c = '/';
    return s;
#else
    char buf[PATH_MAX] = {};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len < 0) return ".";
    std::string s(buf, len);
    size_t pos = s.rfind('/');
    if (pos != std::string::npos) s = s.substr(0, pos);
    return s;
#endif
}

int main() {
    ModeManager manager;
    AppBridge   bridge(manager);

    webview::webview w(true, nullptr);
    w.set_title("JeanClaude Math Editor");
    w.set_size(1440, 900, WEBVIEW_HINT_NONE);

    bridge.registerBindings(w);

    std::string frontendUrl = "file:///" + getExeDir() + "/frontend/index.html";
    w.navigate(frontendUrl);

    w.run();
    return 0;
}
