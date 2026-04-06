#include "AppBridge.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <string>

// ── Platform-specific file dialog headers ──────────────────────────
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <shobjidl.h>   // IFileOpenDialog / IFileSaveDialog
#endif

AppBridge::AppBridge(ModeManager& manager) : _manager(manager) {}

// ── JSON helpers ────────────────────────────────────────────────────

// Escape a string value for embedding inside a JSON string literal.
static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else                out += c;
    }
    return out;
}

std::string AppBridge::getFullStateJSON() const {
    const auto& modes = _manager.getModes();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"modes\":[";
    for (size_t i = 0; i < modes.size(); ++i) {
        ss << "{"
           << "\"name\":\""     << modes[i].name      << "\","
           << "\"cost\":"       << modes[i].cost       << ","
           << "\"rtp\":"        << modes[i].rtp        << ","
           << "\"simulated\":"  << (modes[i].simulated ? "true" : "false") << ","
           << "\"multipliers\":[";
        for (size_t j = 0; j < modes[i].multipliers.size(); ++j) {
            ss << "{\"m\":" << modes[i].multipliers[j].multiplier
               << ",\"w\":" << modes[i].multipliers[j].weight << "}";
            if (j < modes[i].multipliers.size() - 1) ss << ",";
        }
        ss << "],\"freeSpins\":{"
           << "\"enabled\":"        << (modes[i].freeSpins.enabled      ? "true" : "false") << ","
           << "\"triggerWeight\":"  << modes[i].freeSpins.triggerWeight  << ","
           << "\"count\":"          << modes[i].freeSpins.count          << ","
           << "\"multiplierBoost\":" << modes[i].freeSpins.multiplierBoost << ","
           << "\"canRetrigger\":"   << (modes[i].freeSpins.canRetrigger  ? "true" : "false")
           << "},\"simCount\":"  << modes[i].simCount
           << ",\"stats\":{"
           << "\"calculated\":"   << (modes[i].stats.calculated ? "true" : "false") << ","
           << "\"meanPayout\":"   << modes[i].stats.meanPayout   << ","
           << "\"variance\":"     << modes[i].stats.variance     << ","
           << "\"stdDeviation\":" << modes[i].stats.stdDeviation << ","
           << "\"volatility\":"   << modes[i].stats.volatility   << ","
           << "\"hitFrequency\":" << modes[i].stats.hitFrequency << ","
           << "\"minPayout\":"    << modes[i].stats.minPayout    << ","
           << "\"maxPayout\":"    << modes[i].stats.maxPayout
           << "}}";
        if (i < modes.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// ── File dialog implementations ─────────────────────────────────────

#ifdef _WIN32

// Modern Vista+ Common Item Dialog
static std::string winPickFile(bool isSave) {
    std::string result;
    IFileDialog* pfd = nullptr;
    CLSID clsid = isSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;

    if (FAILED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&pfd))))
        return result;

    // File type filter
    COMDLG_FILTERSPEC fs[] = {
        { L"JSON Files", L"*.json" },
        { L"All Files",  L"*.*"   }
    };
    pfd->SetFileTypes(2, fs);
    pfd->SetFileTypeIndex(1);
    pfd->SetDefaultExtension(L"json");

    if (pfd->Show(nullptr) == S_OK) {
        IShellItem* item = nullptr;
        if (pfd->GetResult(&item) == S_OK) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                int n = WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                            nullptr, 0, nullptr, nullptr);
                if (n > 1) {
                    result.resize(n - 1);
                    WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                       result.data(), n, nullptr, nullptr);
                }
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }
    pfd->Release();
    return result;
}

static std::string winPickFolder() {
    std::string result;
    IFileOpenDialog* pfd = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        return result;

    DWORD opts = 0;
    pfd->GetOptions(&opts);
    pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

    if (pfd->Show(nullptr) == S_OK) {
        IShellItem* item = nullptr;
        if (pfd->GetResult(&item) == S_OK) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                int n = WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                            nullptr, 0, nullptr, nullptr);
                if (n > 1) {
                    result.resize(n - 1);
                    WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                       result.data(), n, nullptr, nullptr);
                }
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }
    pfd->Release();
    return result;
}

#else  // Linux — GTK file chooser

#  include <gtk/gtk.h>

static std::string gtkPickFile(bool isSave) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        isSave ? "Save Config" : "Open Config",
        NULL,
        isSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        isSave ? "_Save" : "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );
    if (isSave)
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "JSON files (*.json)");
    gtk_file_filter_add_pattern(filter, "*.json");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    GtkFileFilter* all = gtk_file_filter_new();
    gtk_file_filter_set_name(all, "All files");
    gtk_file_filter_add_pattern(all, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all);

    std::string result;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        result = fn;
        g_free(fn);
    }
    gtk_widget_destroy(dialog);
    return result;
}

static std::string gtkPickFolder() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Select Output Directory",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Select", GTK_RESPONSE_ACCEPT,
        NULL
    );

    std::string result;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        result = fn;
        g_free(fn);
    }
    gtk_widget_destroy(dialog);
    return result;
}

#endif // _WIN32

// ── Binding registration ────────────────────────────────────────────

void AppBridge::registerBindings(webview::webview& w) {

    w.bind("getModes",   [this](std::string const&) { return getFullStateJSON(); });
    w.bind("addMode",    [this](std::string const&) { _manager.addDefaultMode(); return getFullStateJSON(); });
    w.bind("removeMode", [this](std::string const&) { _manager.removeLastMode(); return getFullStateJSON(); });

    w.bind("runSims", [this](std::string const& param) {
        try { _manager.runAllSimulations(std::stoi(param)); return getFullStateJSON(); }
        catch (...) { return std::string("{\"error\":\"Invalid count\"}"); }
    });

    w.bind("updateModeName", [this](std::string const& arg) {
        size_t sep = arg.find('|');
        if (sep != std::string::npos) {
            int idx = std::stoi(arg.substr(0, sep));
            if (idx >= 0 && (size_t)idx < _manager.getModes().size())
                strncpy(_manager.getModes()[idx].name, arg.substr(sep + 1).c_str(), 63);
        }
        return getFullStateJSON();
    });

    w.bind("updateModeCost", [this](std::string const& arg) {
        size_t sep = arg.find('|');
        if (sep != std::string::npos) {
            int idx = std::stoi(arg.substr(0, sep));
            if (idx >= 0 && (size_t)idx < _manager.getModes().size())
                _manager.getModes()[idx].cost = std::stof(arg.substr(sep + 1));
        }
        return getFullStateJSON();
    });

    w.bind("addMultiplier", [this](std::string const& idx_str) {
        int idx = std::stoi(idx_str);
        if (idx >= 0 && (size_t)idx < _manager.getModes().size())
            _manager.getModes()[idx].multipliers.push_back({1.0f, 100});
        return getFullStateJSON();
    });

    w.bind("removeMultiplier", [this](std::string const& arg) {
        size_t sep = arg.find('|');
        if (sep != std::string::npos) {
            int mIdx    = std::stoi(arg.substr(0, sep));
            int multIdx = std::stoi(arg.substr(sep + 1));
            if (mIdx >= 0 && (size_t)mIdx < _manager.getModes().size()) {
                auto& mults = _manager.getModes()[mIdx].multipliers;
                if (multIdx >= 0 && (size_t)multIdx < mults.size())
                    mults.erase(mults.begin() + multIdx);
            }
        }
        return getFullStateJSON();
    });

    w.bind("updateMultiplier", [this](std::string const& arg) {
        std::vector<std::string> parts;
        std::stringstream ss(arg);
        std::string part;
        while (std::getline(ss, part, '|')) parts.push_back(part);
        if (parts.size() == 4) {
            int mIdx    = std::stoi(parts[0]);
            int multIdx = std::stoi(parts[1]);
            if (mIdx >= 0 && (size_t)mIdx < _manager.getModes().size()) {
                auto& mults = _manager.getModes()[mIdx].multipliers;
                if (multIdx >= 0 && (size_t)multIdx < mults.size()) {
                    mults[multIdx].multiplier = std::stof(parts[2]);
                    mults[multIdx].weight     = std::stoi(parts[3]);
                }
            }
        }
        return getFullStateJSON();
    });

    w.bind("updateFreeSpins", [this](std::string const& arg) {
        std::vector<std::string> parts;
        std::stringstream ss(arg);
        std::string part;
        while (std::getline(ss, part, '|')) parts.push_back(part);
        if (parts.size() == 6) {
            int idx = std::stoi(parts[0]);
            if (idx >= 0 && (size_t)idx < _manager.getModes().size()) {
                auto& fs = _manager.getModes()[idx].freeSpins;
                fs.enabled          = (parts[1] == "1");
                fs.triggerWeight    = std::stoi(parts[2]);
                fs.count            = std::stoi(parts[3]);
                fs.multiplierBoost  = std::stof(parts[4]);
                fs.canRetrigger     = (parts[5] == "1");
            }
        }
        return getFullStateJSON();
    });

    w.bind("save",   [this](std::string const& path) {
        _manager.saveConfig(path.c_str());
        return std::string("{\"status\":\"ok\"}");
    });
    w.bind("load",   [this](std::string const& path) {
        return _manager.loadConfig(path.c_str()) ? getFullStateJSON() : "{\"error\":\"Load failed\"}";
    });
    w.bind("export", [this](std::string const& dir) {
        return _manager.exportFiles(dir.c_str()) ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}";
    });

    // ── Browse for a config file (open or save) ─────────────────────
    w.bind("browseConfig", [](std::string const& mode) -> std::string {
        bool isSave = (mode == "save");
#ifdef _WIN32
        std::string path = winPickFile(isSave);
#else
        std::string path = gtkPickFile(isSave);
#endif
        return "{\"configPath\":\"" + jsonEscape(path) + "\"}";
    });

    // ── Browse for an output directory ──────────────────────────────
    w.bind("browseOutput", [](std::string const&) -> std::string {
#ifdef _WIN32
        std::string path = winPickFolder();
#else
        std::string path = gtkPickFolder();
#endif
        return "{\"outputDir\":\"" + jsonEscape(path) + "\"}";
    });
}
