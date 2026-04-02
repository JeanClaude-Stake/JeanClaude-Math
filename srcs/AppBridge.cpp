#include "AppBridge.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>

AppBridge::AppBridge(ModeManager& manager) : _manager(manager) {}

std::string AppBridge::getFullStateJSON() const {
    const auto& modes = _manager.getModes();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"modes\":[";
    for(size_t i = 0; i < modes.size(); ++i) {
        ss << "{"
           << "\"name\":\"" << modes[i].name << "\","
           << "\"cost\":" << modes[i].cost << ","
           << "\"rtp\":" << modes[i].rtp << ","
           << "\"simulated\":" << (modes[i].simulated ? "true" : "false") << ","
           << "\"multipliers\":[";
        for(size_t j = 0; j < modes[i].multipliers.size(); ++j) {
            ss << "{\"m\":" << modes[i].multipliers[j].multiplier 
               << ",\"w\":" << modes[i].multipliers[j].weight << "}";
            if(j < modes[i].multipliers.size() - 1) ss << ",";
        }
        ss << "],\"freeSpins\":{"
           << "\"enabled\":" << (modes[i].freeSpins.enabled ? "true" : "false") << ","
           << "\"triggerWeight\":" << modes[i].freeSpins.triggerWeight << ","
           << "\"count\":" << modes[i].freeSpins.count << ","
           << "\"multiplierBoost\":" << modes[i].freeSpins.multiplierBoost << ","
           << "\"canRetrigger\":" << (modes[i].freeSpins.canRetrigger ? "true" : "false")
           << "}}";
        if (i < modes.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

void AppBridge::registerBindings(webview::webview& w) {
    w.bind("getModes", [this](std::string const&) { return getFullStateJSON(); });
    w.bind("addMode", [this](std::string const&) { _manager.addDefaultMode(); return getFullStateJSON(); });
    w.bind("removeMode", [this](std::string const&) { _manager.removeLastMode(); return getFullStateJSON(); });
    
    w.bind("runSims", [this](std::string const& param) {
        try { _manager.runAllSimulations(std::stoi(param)); return getFullStateJSON(); }
        catch (...) { return std::string("{\"error\":\"Invalid count\"}"); }
    });

    w.bind("updateModeName", [this](std::string const& arg) {
        size_t sep = arg.find("|");
        if (sep != std::string::npos) {
            int idx = std::stoi(arg.substr(0, sep));
            if (idx >= 0 && (size_t)idx < _manager.getModes().size())
                strncpy(_manager.getModes()[idx].name, arg.substr(sep + 1).c_str(), 63);
        }
        return getFullStateJSON();
    });

    w.bind("updateModeCost", [this](std::string const& arg) {
        size_t sep = arg.find("|");
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
        size_t sep = arg.find("|");
        if (sep != std::string::npos) {
            int mIdx = std::stoi(arg.substr(0, sep));
            int multIdx = std::stoi(arg.substr(sep + 1));
            if (mIdx >= 0 && (size_t)mIdx < _manager.getModes().size()) {
                auto& mults = _manager.getModes()[mIdx].multipliers;
                if (multIdx >= 0 && (size_t)multIdx < mults.size()) mults.erase(mults.begin() + multIdx);
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
            int mIdx = std::stoi(parts[0]);
            int multIdx = std::stoi(parts[1]);
            if (mIdx >= 0 && (size_t)mIdx < _manager.getModes().size()) {
                auto& mults = _manager.getModes()[mIdx].multipliers;
                if (multIdx >= 0 && (size_t)multIdx < mults.size()) {
                    mults[multIdx].multiplier = std::stof(parts[2]);
                    mults[multIdx].weight = std::stoi(parts[3]);
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
                fs.enabled = (parts[1] == "1");
                fs.triggerWeight = std::stoi(parts[2]);
                fs.count = std::stoi(parts[3]);
                fs.multiplierBoost = std::stof(parts[4]);
                fs.canRetrigger = (parts[5] == "1");
            }
        }
        return getFullStateJSON();
    });

    w.bind("save", [this](std::string const& path) { _manager.saveConfig(path.c_str()); return std::string("{\"status\":\"ok\"}"); });
    w.bind("load", [this](std::string const& path) { return _manager.loadConfig(path.c_str()) ? getFullStateJSON() : "{\"error\":\"Load failed\"}"; });
    w.bind("export", [this](std::string const& dir) { return _manager.exportFiles(dir.c_str()) ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}"; });
}
