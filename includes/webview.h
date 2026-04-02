#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <functional>
#include <map>
#include <iostream>

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
            
            g_signal_connect(manager, "script-message-received::external", G_CALLBACK(on_message), this);
            g_signal_connect(window_, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        }

        void bind(const std::string& name, std::function<std::string(std::string)> fn) {
            std::cout << "[WEBVIEW] Binding function: " << name << std::endl;
            bindings_[name] = fn;
        }

        void set_title(const std::string& title) { gtk_window_set_title(GTK_WINDOW(window_), title.c_str()); }
        void set_size(int width, int height, int /*hint*/) { gtk_window_set_default_size(GTK_WINDOW(window_), width, height); }
        void navigate(const std::string& url) { webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview_), url.c_str()); }
        
        void run() {
            gtk_widget_show_all(window_);
            // Inject bridge and a global callback handler
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
            WebKitUserContentManager* manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(webview_));
            webkit_user_content_manager_add_script(manager, 
                webkit_user_script_new(js.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL, NULL));
            gtk_main();
        }

        void resolve(const std::string& json_result) {
            std::string js = "window._backendCallback(" + json_result + ");";
            webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(webview_), js.c_str(), -1, NULL, NULL, NULL, NULL, NULL);
        }

    private:
        GtkWidget* window_;
        GtkWidget* webview_;
        std::map<std::string, std::function<std::string(std::string)>> bindings_;

        static void on_message(WebKitUserContentManager* /*manager*/, WebKitJavascriptResult* res, gpointer data) {
            webview* self = static_cast<webview*>(data);
            JSCValue* val = webkit_javascript_result_get_js_value(res);
            if (val && jsc_value_is_string(val)) {
                char* msg_ptr = jsc_value_to_string(val);
                std::string s(msg_ptr);
                g_free(msg_ptr);
                
                std::cout << "[WEBVIEW] Received message from JS: " << s << std::endl;

                // Manual JSON mini-parser for {"fn":"...", "arg":"..."}
                std::string fn, arg;
                size_t fn_key = s.find("\"fn\":\"");
                if (fn_key != std::string::npos) {
                    size_t start = fn_key + 6;
                    size_t end = s.find("\"", start);
                    fn = s.substr(start, end - start);
                }
                
                size_t arg_key = s.find("\"arg\":");
                if (arg_key != std::string::npos) {
                    size_t start = arg_key + 6;
                    while(start < s.length() && s[start] == ' ') start++;
                    if (s[start] == '"') {
                        start++;
                        size_t end = s.find("\"", start);
                        arg = s.substr(start, end - start);
                    } else {
                        size_t end = s.find_first_of(",}", start);
                        arg = s.substr(start, end - start);
                    }
                }

                if (!fn.empty() && self->bindings_.count(fn)) {
                    std::cout << "[WEBVIEW] Executing binding: " << fn << " with arg: " << arg << std::endl;
                    std::string result = self->bindings_[fn](arg);
                    if (!result.empty()) self->resolve(result);
                }
            }
            webkit_javascript_result_unref(res);
        }
    };
}
#define WEBVIEW_HINT_NONE 0

#endif
