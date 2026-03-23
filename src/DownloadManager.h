#pragma once

#include <string>
#include <functional>

class DownloadManager {
public:
    using ProgressCallback = std::function<void(const std::string& filename, size_t downloaded, size_t total)>;
    
    static constexpr const char* VERSION_MANIFEST_URL = "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json";
    static constexpr const char* ASSETS_BASE_URL = "https://resources.download.minecraft.net";
    static constexpr const char* LIBRARIES_BASE_URL = "https://libraries.minecraft.net";
    
    static bool downloadFile(const std::string& url, const std::string& destPath, 
                           ProgressCallback callback = nullptr);
    static std::string downloadString(const std::string& url);
    static bool fileExists(const std::string& path);
    static size_t getFileSize(const std::string& path);
};
