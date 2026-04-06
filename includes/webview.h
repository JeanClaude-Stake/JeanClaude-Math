#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <string>
#include <functional>
#include <map>
#include <iostream>

/* ═══════════════════════════════════════════════════════════════════
   Windows implementation — Microsoft Edge WebView2
   Requires: libs/webview2/include + libs/webview2/x64/WebView2Loader.lib
   ═══════════════════════════════════════════════════════════════════ */
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <sstream>

using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;

namespace webview {

// ── UTF-8 ↔ UTF-16 helpers ──────────────────────────────────────────
static std::wstring to_wstr(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring r(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, r.data(), n);
    return r;
}
static std::string to_str(const std::wstring& ws) {
    if (ws.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string r(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, r.data(), n, nullptr, nullptr);
    return r;
}

class webview {
public:
    webview(bool /*debug*/, void*) {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        HINSTANCE hInst = GetModuleHandle(nullptr);

        WNDCLASSEXW wc    = {};
        wc.cbSize         = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc    = WndProc;
        wc.hInstance      = hInst;
        wc.lpszClassName  = L"MathSDKWindow";
        wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
        RegisterClassExW(&wc);

        hwnd_ = CreateWindowExW(
            0, L"MathSDKWindow", L"MathSDK",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1440, 900,
            nullptr, nullptr, hInst, this
        );
        if (!hwnd_) { std::cerr << "[WEBVIEW] Failed to create window\n"; return; }
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
    }

    ~webview() {
        if (webview_)    webview_->Release();
        if (controller_) controller_->Release();
        CoUninitialize();
    }

    void bind(const std::string& name, std::function<std::string(std::string)> fn) {
        std::cout << "[WEBVIEW] Binding: " << name << "\n";
        bindings_[name] = fn;
    }

    void set_title(const std::string& t) {
        SetWindowTextA(hwnd_, t.c_str());
    }

    void set_size(int w, int h, int) {
        // Account for non-client area so the *client* area matches the requested size
        RECT rc = { 0, 0, w, h };
        AdjustWindowRectEx(&rc, GetWindowLong(hwnd_, GWL_STYLE), FALSE,
                           GetWindowLong(hwnd_, GWL_EXSTYLE));
        SetWindowPos(hwnd_, nullptr, CW_USEDEFAULT, CW_USEDEFAULT,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }

    void navigate(const std::string& url) {
        url_ = url;
        // If WebView2 is already ready, navigate immediately
        if (webview_) webview_->Navigate(to_wstr(url_).c_str());
    }

    // Called by AppBridge to send JSON back to the frontend
    void resolve(const std::string& json) {
        if (!webview_) return;
        std::wstring js = L"window._backendCallback(" + to_wstr(json) + L");";
        webview_->ExecuteScript(js.c_str(), nullptr);
    }

    void run() {
        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);

        // ── Async WebView2 initialisation ─────────────────────────────
        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this](HRESULT hr, ICoreWebView2Environment* env) -> HRESULT {
                    if (FAILED(hr)) {
                        std::cerr << "[WEBVIEW] Env creation failed: 0x"
                                  << std::hex << hr << "\n";
                        PostQuitMessage(1);
                        return hr;
                    }
                    return env->CreateCoreWebView2Controller(
                        hwnd_,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this](HRESULT hr, ICoreWebView2Controller* ctrl) -> HRESULT {
                                if (FAILED(hr)) {
                                    std::cerr << "[WEBVIEW] Controller creation failed: 0x"
                                              << std::hex << hr << "\n";
                                    PostQuitMessage(1);
                                    return hr;
                                }

                                controller_ = ctrl;
                                controller_->AddRef();
                                controller_->get_CoreWebView2(&webview_);

                                // Resize WebView2 to fill the window
                                resize_webview();

                                // ── Inject JS bridge at document start ──
                                // window.external.invoke() posts to C++
                                // window._backendCallback() dispatches 'backend-response'
                                std::wstring bridge =
                                    L"window.external = {"
                                    L"  invoke: function(msg) {"
                                    L"    window.chrome.webview.postMessage(msg);"
                                    L"  }"
                                    L"};"
                                    L"window._backendCallback = function(data) {"
                                    L"  var ev = new CustomEvent('backend-response', { detail: data });"
                                    L"  window.dispatchEvent(ev);"
                                    L"};";
                                webview_->AddScriptToExecuteOnDocumentCreated(
                                    bridge.c_str(), nullptr);

                                // ── Listen for JS → C++ messages ─────────
                                webview_->add_WebMessageReceived(
                                    Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                        [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args)
                                            -> HRESULT {
                                            LPWSTR raw = nullptr;
                                            args->TryGetWebMessageAsString(&raw);
                                            if (raw) {
                                                dispatch(to_str(std::wstring(raw)));
                                                CoTaskMemFree(raw);
                                            }
                                            return S_OK;
                                        }).Get(), nullptr);

                                // ── Navigate now that WebView2 is ready ──
                                if (!url_.empty())
                                    webview_->Navigate(to_wstr(url_).c_str());

                                return S_OK;
                            }).Get());
                }).Get());

        if (FAILED(hr)) {
            MessageBoxA(hwnd_,
                "WebView2 Runtime not found.\n\n"
                "Please install the Microsoft Edge WebView2 Runtime:\n"
                "https://developer.microsoft.com/microsoft-edge/webview2/",
                "MathSDK — Missing WebView2", MB_ICONERROR | MB_OK);
            return;
        }

        // ── Win32 message loop ────────────────────────────────────────
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

private:
    HWND                 hwnd_       = nullptr;
    ICoreWebView2Controller* controller_ = nullptr;
    ICoreWebView2*           webview_    = nullptr;
    std::string          url_;
    std::map<std::string, std::function<std::string(std::string)>> bindings_;

    void resize_webview() {
        if (!controller_ || !hwnd_) return;
        RECT rc;
        GetClientRect(hwnd_, &rc);
        controller_->put_Bounds(rc);
    }

    // ── Same mini-JSON parser as the Linux side ──────────────────────
    void dispatch(const std::string& s) {
        std::cout << "[WEBVIEW] Message from JS: " << s << "\n";

        std::string fn, arg;
        size_t fk = s.find("\"fn\":\"");
        if (fk != std::string::npos) {
            size_t st = fk + 6, en = s.find('"', st);
            fn = s.substr(st, en - st);
        }
        size_t ak = s.find("\"arg\":");
        if (ak != std::string::npos) {
            size_t st = ak + 6;
            while (st < s.size() && s[st] == ' ') st++;
            if (s[st] == '"') {
                st++;
                size_t en = s.find('"', st);
                arg = s.substr(st, en - st);
            } else {
                size_t en = s.find_first_of(",}", st);
                arg = s.substr(st, en - st);
            }
        }

        if (!fn.empty() && bindings_.count(fn)) {
            std::cout << "[WEBVIEW] Calling binding: " << fn << "\n";
            std::string result = bindings_[fn](arg);
            if (!result.empty()) resolve(result);
        }
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        webview* self = nullptr;
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = static_cast<webview*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
        } else {
            self = reinterpret_cast<webview*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        switch (msg) {
        case WM_SIZE:
            if (self) self->resize_webview();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
};

} // namespace webview

/* ═══════════════════════════════════════════════════════════════════
   Linux implementation — GTK3 + WebKit2GTK  (unchanged)
   ═══════════════════════════════════════════════════════════════════ */
#else

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

namespace webview {
    class webview {
    public:
        webview(bool /*debug*/, void* /*window*/) {
            gtk_init(NULL, NULL);
            window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);

            WebKitUserContentManager* manager = webkit_user_content_manager_new();
            webkit_user_content_manager_register_script_message_handler(manager, "external");

            webview_ = webkit_web_view_new_with_user_content_manager(manager);
            gtk_container_add(GTK_CONTAINER(window_), webview_);

            g_signal_connect(manager, "script-message-received::external",
                             G_CALLBACK(on_message), this);
            g_signal_connect(window_, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        }

        void bind(const std::string& name, std::function<std::string(std::string)> fn) {
            std::cout << "[WEBVIEW] Binding function: " << name << std::endl;
            bindings_[name] = fn;
        }

        void set_title(const std::string& title) {
            gtk_window_set_title(GTK_WINDOW(window_), title.c_str());
        }
        void set_size(int width, int height, int /*hint*/) {
            gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
        }
        void navigate(const std::string& url) {
            webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview_), url.c_str());
        }

        void run() {
            gtk_widget_show_all(window_);

            std::string js =
                "window.external = { "
                "  invoke: function(msg) { "
                "    console.log('[JS] Invoking backend with:', msg);"
                "    window.webkit.messageHandlers.external.postMessage(msg); "
                "  }"
                "};"
                "window._backendCallback = function(data) {"
                "  console.log('[JS] Received backend callback data:', data);"
                "  const event = new CustomEvent('backend-response', { detail: data });"
                "  window.dispatchEvent(event);"
                "};";

            WebKitUserContentManager* manager =
                webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(webview_));
            webkit_user_content_manager_add_script(manager,
                webkit_user_script_new(js.c_str(),
                    WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                    WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
                    NULL, NULL));
            gtk_main();
        }

        void resolve(const std::string& json_result) {
            std::string js = "window._backendCallback(" + json_result + ");";
            webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(webview_),
                js.c_str(), -1, NULL, NULL, NULL, NULL, NULL);
        }

    private:
        GtkWidget* window_;
        GtkWidget* webview_;
        std::map<std::string, std::function<std::string(std::string)>> bindings_;

        static void on_message(WebKitUserContentManager* /*manager*/,
                               WebKitJavascriptResult* res, gpointer data) {
            webview* self = static_cast<webview*>(data);
            JSCValue* val = webkit_javascript_result_get_js_value(res);
            if (val && jsc_value_is_string(val)) {
                char* msg_ptr = jsc_value_to_string(val);
                std::string s(msg_ptr);
                g_free(msg_ptr);

                std::cout << "[WEBVIEW] Received message from JS: " << s << std::endl;

                std::string fn, arg;
                size_t fn_key = s.find("\"fn\":\"");
                if (fn_key != std::string::npos) {
                    size_t start = fn_key + 6, end = s.find('"', start);
                    fn = s.substr(start, end - start);
                }
                size_t arg_key = s.find("\"arg\":");
                if (arg_key != std::string::npos) {
                    size_t start = arg_key + 6;
                    while (start < s.length() && s[start] == ' ') start++;
                    if (s[start] == '"') {
                        start++;
                        size_t end = s.find('"', start);
                        arg = s.substr(start, end - start);
                    } else {
                        size_t end = s.find_first_of(",}", start);
                        arg = s.substr(start, end - start);
                    }
                }

                if (!fn.empty() && self->bindings_.count(fn)) {
                    std::cout << "[WEBVIEW] Executing binding: " << fn
                              << " with arg: " << arg << std::endl;
                    std::string result = self->bindings_[fn](arg);
                    if (!result.empty()) self->resolve(result);
                }
            }
            webkit_javascript_result_unref(res);
        }
    };
}

#endif // _WIN32

#define WEBVIEW_HINT_NONE 0
#endif // WEBVIEW_H
