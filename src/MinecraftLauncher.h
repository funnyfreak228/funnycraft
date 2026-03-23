#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <wx/wx.h>
#include "AccountManager.h"
#include "InstanceManager.h"

class MinecraftLauncher {
public:
    static bool launch(Instance* instance, Account* account, wxWindow* parent = nullptr);
    
private:
    static bool checkNeedsDownload(const std::string& version);
    static bool doLaunch(Instance* instance, Account* account);
    static bool ensureVersionFiles(const std::string& version);
    static nlohmann::json getVersionInfo(const std::string& version);
    static bool downloadLibraries(const std::string& version, const nlohmann::json& versionJson);
    static bool downloadAssets(const std::string& version, const nlohmann::json& versionJson);
    static std::string buildClasspath(const std::string& version);
    static bool extractNatives(const std::string& version, const std::string& destDir);
    static std::string getMainClass(const std::string& version);
    static std::string getAssetIndex(const std::string& version);
};
