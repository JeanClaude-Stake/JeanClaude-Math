#ifndef APPBRIDGE_HPP
# define APPBRIDGE_HPP

# include "webview.h"
# include "ModeManager.hpp"
# include <string>

class AppBridge {
public:
    AppBridge(ModeManager& manager);
    void registerBindings(webview::webview& w);

private:
    ModeManager& _manager;
    std::string getFullStateJSON() const;
};

#endif
