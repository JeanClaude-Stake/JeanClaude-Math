#include "webview.h"

#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface WebviewDelegate : NSObject <WKScriptMessageHandler, NSWindowDelegate>
@property (nonatomic, assign) void* webviewPtr;
@end

@implementation WebviewDelegate
- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)message {
    if ([message.name isEqualToString:@"external"]) {
        std::string s = [[message body] UTF8String];
        static_cast<webview::webview*>(_webviewPtr)->dispatch(s);
    }
}
- (void)windowWillClose:(NSNotification *)notification {
    [NSApp stop:nil];
}
@end

namespace webview {

webview::webview(bool /*debug*/, void* /*window*/) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSRect frame = NSMakeRect(0, 0, 1440, 900);
    NSWindow* win = [[NSWindow alloc] initWithContentRect:frame
                                         styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [win cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
    window_ = (void*)win;
    
    WebviewDelegate* delegate = [[WebviewDelegate alloc] init];
    delegate.webviewPtr = this;
    [win setDelegate:delegate];
    delegate_ = (void*)delegate;

    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    [[config userContentController] addScriptMessageHandler:delegate name:@"external"];
    
    WKWebView* wv = [[WKWebView alloc] initWithFrame:frame configuration:config];
    [win setContentView:wv];
    webview_ = (void*)wv;

    // Inject bridge JS
    NSString *js = @"window.external = { invoke: function(msg) { window.webkit.messageHandlers.external.postMessage(msg); } };"
                   "window._backendCallback = function(data) { const ev = new CustomEvent('backend-response', { detail: data }); window.dispatchEvent(ev); };";
    WKUserScript *script = [[WKUserScript alloc] initWithSource:js injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [[config userContentController] addUserScript:script];
}

void webview::bind(const std::string& name, std::function<std::string(std::string)> fn) {
    bindings_[name] = fn;
}

void webview::set_title(const std::string& title) {
    [(NSWindow*)window_ setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

void webview::set_size(int width, int height, int /*hint*/) {
    NSRect frame = [(NSWindow*)window_ frame];
    frame.size.width = width;
    frame.size.height = height;
    [(NSWindow*)window_ setFrame:frame display:YES];
}

void webview::navigate(const std::string& url) {
    NSString *nsurl = [NSString stringWithUTF8String:url.c_str()];
    [(WKWebView*)webview_ loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:nsurl]]];
}

void webview::run() {
    [(NSWindow*)window_ makeKeyAndOrderFront:nil];
    [NSApp run];
}

void webview::resolve(const std::string& json) {
    NSString *js = [NSString stringWithFormat:@"window._backendCallback(%@);", 
                    [NSString stringWithUTF8String:json.c_str()]];
    [(WKWebView*)webview_ evaluateJavaScript:js completionHandler:nil];
}

void webview::dispatch(const std::string& s) {
    std::string fn, arg;
    size_t fk = s.find("\"fn\":\"");
    if (fk != std::string::npos) {
        size_t st = fk + 6, en = s.find('"', st);
        if (en != std::string::npos) fn = s.substr(st, en - st);
    }
    size_t ak = s.find("\"arg\":");
    if (ak != std::string::npos) {
        size_t st = ak + 6;
        while (st < s.size() && s[st] == ' ') st++;
        if (st < s.size()) {
            if (s[st] == '"') {
                st++;
                size_t en = s.find('"', st);
                if (en != std::string::npos) arg = s.substr(st, en - st);
            } else {
                size_t en = s.find_first_of(",}", st);
                arg = s.substr(st, en - st);
            }
        }
    }

    if (!fn.empty() && bindings_.count(fn)) {
        std::string result = bindings_[fn](arg);
        if (!result.empty()) resolve(result);
    }
}

} // namespace webview

#endif
