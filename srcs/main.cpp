#include "webview.h"
#include "ModeManager.hpp"
#include "AppBridge.hpp"
#include <iostream>

int main() {
    ModeManager manager;
    AppBridge bridge(manager);
    
    webview::webview w(true, nullptr);
    w.set_title("MathSDK - Engine Editor");
    w.set_size(1024, 768, WEBVIEW_HINT_NONE);
    
    bridge.registerBindings(w);

    std::cout << "[DEBUG] Webview starting..." << std::endl;
    w.navigate("file:///mnt/e/C++/math-sdk/frontend/index.html");
    w.run();
    
    return 0;
}
