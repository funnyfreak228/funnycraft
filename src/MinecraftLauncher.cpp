#include "MinecraftLauncher.h"
#include "DownloadManager.h"
#include "PathUtils.h"
#include "ProgressDialog.h"
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>

namespace fs = std::filesystem;
using json = nlohmann::json;

// Check if Java is available
static bool checkJava(const std::string& javaPath) {
    std::string cmd = javaPath + " -version 2>&1";
    return std::system(cmd.c_str()) == 0;
}

// Global progress dialog pointer for callbacks
static ProgressDialog* gProgressDialog = nullptr;

// Progress callback that updates the dialog
static void progressCallback(const std::string& filename, int64_t downloaded, int64_t total) {
    if (gProgressDialog) {
        gProgressDialog->updateFileInfo(filename, downloaded, total);
    }
}

bool MinecraftLauncher::launch(Instance* instance, Account* account, wxWindow* parent) {
    if (!instance || !account) {
        std::cerr << "Error: No instance or account selected" << std::endl;
        return false;
    }
    
    // Use system Java
    std::string javaPath = "java";
    
    // Check if downloads are needed
    bool needsDownload = checkNeedsDownload(instance->version);
    
    // Create progress dialog only if downloads are needed and we have a parent
    ProgressDialog* dialog = nullptr;
    std::thread downloadThread;
    std::atomic<bool> downloadSuccess(true);
    std::atomic<bool> downloadComplete(false);
    
    if (needsDownload && parent) {
        dialog = new ProgressDialog(parent, "Downloading Minecraft " + instance->version);
        gProgressDialog = dialog;
        
        // Run downloads in background thread
        downloadThread = std::thread([instance, &downloadSuccess, &downloadComplete]() {
            downloadSuccess = ensureVersionFiles(instance->version);
            downloadComplete = true;
            // Signal dialog to close when done
            if (gProgressDialog) {
                gProgressDialog->finish();
            }
        });
        
        // Show dialog modally - it will close when finish() is called or user cancels
        int result = dialog->ShowModal();
        
        // Wait for download thread to finish
        if (downloadThread.joinable()) {
            downloadThread.join();
        }
        
        gProgressDialog = nullptr;
        
        // Check if cancelled
        if (dialog->isCancelled() || result == wxID_CANCEL) {
            delete dialog;
            return false;
        }
        
        delete dialog;
        
        if (!downloadSuccess) {
            wxMessageBox("Failed to download required files. Check your internet connection.", 
                        "Download Error", wxOK | wxICON_ERROR, parent);
            return false;
        }
    } else if (needsDownload) {
        // No parent window, just download silently
        if (!ensureVersionFiles(instance->version)) {
            std::cerr << "Failed to download required files" << std::endl;
            return false;
        }
    }
    
    // Continue with launch
    return doLaunch(instance, account);
}

bool MinecraftLauncher::checkNeedsDownload(const std::string& version) {
    // Get version info from manifest
    json versionInfo = getVersionInfo(version);
    if (versionInfo.is_null()) {
        return true; // Need to download to get info
    }
    
    std::string versionDir = PathUtils::getMinecraftDir() + "/versions/" + version;
    std::string jsonPath = versionDir + "/" + version + ".json";
    std::string clientPath = versionDir + "/" + version + ".jar";
    
    // Check version JSON
    if (!DownloadManager::fileExists(jsonPath)) {
        return true;
    }
    
    // Check client JAR
    if (!DownloadManager::fileExists(clientPath)) {
        return true;
    }
    
    // Parse version JSON to check libraries and assets
    json versionJson;
    if (DownloadManager::fileExists(jsonPath)) {
        std::ifstream f(jsonPath);
        f >> versionJson;
    }
    
    if (!versionJson.is_null()) {
        // Check client JAR size
        if (versionJson.contains("downloads") && versionJson["downloads"].contains("client")) {
            auto client = versionJson["downloads"]["client"];
            size_t expectedSize = client.value("size", 0);
            if (DownloadManager::getFileSize(clientPath) != expectedSize) {
                return true;
            }
        }
        
        // Check libraries
        if (versionJson.contains("libraries")) {
            std::string librariesDir = PathUtils::getMinecraftDir() + "/libraries";
            for (const auto& lib : versionJson["libraries"]) {
                // Check rules
                if (lib.contains("rules")) {
                    bool allowed = false;
                    for (const auto& rule : lib["rules"]) {
                        std::string action = rule["action"];
                        if (rule.contains("os")) {
                            std::string osName = rule["os"]["name"];
                            #ifdef __linux__
                            if (osName == "linux") allowed = (action == "allow");
                            #elif __APPLE__
                            if (osName == "osx") allowed = (action == "allow");
                            #elif _WIN32
                            if (osName == "windows") allowed = (action == "allow");
                            #endif
                        } else {
                            allowed = (action == "allow");
                        }
                    }
                    if (!allowed) continue;
                }
                
                // Check artifact
                if (lib.contains("downloads") && lib["downloads"].contains("artifact")) {
                    auto artifact = lib["downloads"]["artifact"];
                    std::string path = artifact["path"];
                    size_t size = artifact.value("size", 0);
                    std::string fullPath = librariesDir + "/" + path;
                    
                    if (!DownloadManager::fileExists(fullPath) ||
                        DownloadManager::getFileSize(fullPath) != size) {
                        return true;
                    }
                }
            }
        }
        
        // Check assets
        if (versionJson.contains("assetIndex")) {
            auto assetIndex = versionJson["assetIndex"];
            std::string indexId = assetIndex["id"];
            std::string indexPath = PathUtils::getMinecraftDir() + "/assets/indexes/" + indexId + ".json";
            
            if (!DownloadManager::fileExists(indexPath)) {
                return true;
            }
            
            // Parse asset index
            std::ifstream f(indexPath);
            json indexJson;
            f >> indexJson;
            
            if (indexJson.contains("objects")) {
                std::string objectsDir = PathUtils::getMinecraftDir() + "/assets/objects";
                for (const auto& [name, obj] : indexJson["objects"].items()) {
                    std::string hash = obj["hash"];
                    size_t size = obj["size"];
                    std::string prefix = hash.substr(0, 2);
                    std::string objectPath = objectsDir + "/" + prefix + "/" + hash;
                    
                    if (!DownloadManager::fileExists(objectPath) ||
                        DownloadManager::getFileSize(objectPath) != size) {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool MinecraftLauncher::doLaunch(Instance* instance, Account* account) {
    // Use system Java
    std::string javaPath = "java";
    std::cout << "Using system Java" << std::endl;
    
    // Build classpath
    std::string classpath = buildClasspath(instance->version);
    if (classpath.empty()) {
        std::cerr << "Error: Could not build classpath" << std::endl;
        return false;
    }
    
    // Extract natives
    std::string nativesDir = instance->gameDir + "/.minecraft/bin/natives";
    fs::create_directories(nativesDir);
    if (!extractNatives(instance->version, nativesDir)) {
        std::cerr << "Warning: Could not extract all natives" << std::endl;
    }
    
    // Get main class from version JSON
    std::string mainClass = getMainClass(instance->version);
    if (mainClass.empty()) {
        mainClass = "net.minecraft.client.main.Main"; // Default
    }
    
    // Get asset index
    std::string assetIndex = getAssetIndex(instance->version);
    
    // Build command with proper quoting
    std::stringstream cmd;
    cmd << "\"" << javaPath << "\"";
    cmd << " " << instance->jvmArgs;
    cmd << " -Xms" << instance->memoryMB << "m";
    cmd << " -Xmx" << instance->memoryMB << "m";
    cmd << " -Djava.library.path=\"" << nativesDir << "\"";
    cmd << " -cp \"" << classpath << "\"";
    cmd << " " << mainClass;
    cmd << " --username \"" << account->username << "\"";
    cmd << " --version \"" << instance->version << "\"";
    cmd << " --gameDir \"" << instance->gameDir << "/.minecraft\"";
    cmd << " --assetsDir \"" << PathUtils::getMinecraftDir() << "/assets\"";
    cmd << " --assetIndex \"" << assetIndex << "\"";
    cmd << " --uuid \"" << account->uuid << "\"";
    cmd << " --accessToken \"" << account->accessToken << "\"";
    cmd << " --userType legacy";
    cmd << " --versionType release";
    
    std::string command = cmd.str();
    std::cout << "Launch command: " << command << std::endl;
    
    int result = std::system((command + " &").c_str());
    if (result != 0) {
        std::cerr << "Failed to launch Minecraft (exit code: " << result << ")" << std::endl;
        return false;
    }
    
    return true;
}

bool MinecraftLauncher::ensureVersionFiles(const std::string& version) {
    // Get version info from manifest
    json versionInfo = getVersionInfo(version);
    if (versionInfo.is_null()) {
        return false;
    }
    
    // Download version JSON
    std::string versionUrl = versionInfo.value("url", "");
    std::string versionDir = PathUtils::getMinecraftDir() + "/versions/" + version;
    std::string jsonPath = versionDir + "/" + version + ".json";
    
    if (!DownloadManager::fileExists(jsonPath) && !versionUrl.empty()) {
        if (gProgressDialog) gProgressDialog->updateMessage("Downloading version info...");
        if (!DownloadManager::downloadFile(versionUrl, jsonPath, progressCallback)) {
            return false;
        }
    }
    
    // Parse version JSON
    json versionJson;
    if (DownloadManager::fileExists(jsonPath)) {
        std::ifstream f(jsonPath);
        f >> versionJson;
    }
    
    // Download client JAR
    if (!versionJson.is_null() && versionJson.contains("downloads")) {
        auto downloads = versionJson["downloads"];
        if (downloads.contains("client")) {
            std::string clientUrl = downloads["client"]["url"];
            std::string clientPath = versionDir + "/" + version + ".jar";
            
            // Check SHA1
            std::string expectedSha1 = downloads["client"].value("sha1", "");
            size_t expectedSize = downloads["client"].value("size", 0);
            
            if (!DownloadManager::fileExists(clientPath) || 
                DownloadManager::getFileSize(clientPath) != expectedSize) {
                if (gProgressDialog) gProgressDialog->updateMessage("Downloading Minecraft client JAR...");
                if (!DownloadManager::downloadFile(clientUrl, clientPath, progressCallback)) {
                    return false;
                }
            }
        }
    }
    
    // Download libraries
    if (!downloadLibraries(version, versionJson)) {
        return false;
    }
    
    // Download assets
    if (!downloadAssets(version, versionJson)) {
        return false;
    }
    
    return true;
}

json MinecraftLauncher::getVersionInfo(const std::string& version) {
    std::string manifestPath = PathUtils::getMinecraftDir() + "/version_manifest.json";
    
    // Download manifest if needed
    if (!DownloadManager::fileExists(manifestPath)) {
        std::cout << "Downloading version manifest..." << std::endl;
        if (!DownloadManager::downloadFile(DownloadManager::VERSION_MANIFEST_URL, manifestPath)) {
            return json();
        }
    }
    
    // Parse manifest
    std::ifstream f(manifestPath);
    json manifest;
    f >> manifest;
    
    // Find version
    if (manifest.contains("versions")) {
        for (const auto& v : manifest["versions"]) {
            if (v["id"] == version) {
                return v;
            }
        }
    }
    
    return json();
}

bool MinecraftLauncher::downloadLibraries(const std::string& version, const json& versionJson) {
    if (!versionJson.contains("libraries")) {
        return true;
    }
    
    std::string librariesDir = PathUtils::getMinecraftDir() + "/libraries";
    fs::create_directories(librariesDir);
    
    for (const auto& lib : versionJson["libraries"]) {
        // Check rules (OS restrictions)
        if (lib.contains("rules")) {
            bool allowed = false;
            for (const auto& rule : lib["rules"]) {
                std::string action = rule["action"];
                
                if (rule.contains("os")) {
                    std::string osName = rule["os"]["name"];
                    #ifdef __linux__
                    if (osName == "linux") allowed = (action == "allow");
                    #elif __APPLE__
                    if (osName == "osx") allowed = (action == "allow");
                    #elif _WIN32
                    if (osName == "windows") allowed = (action == "allow");
                    #endif
                } else {
                    allowed = (action == "allow");
                }
            }
            if (!allowed) continue;
        }
        
        // Download artifact
        if (lib.contains("downloads") && lib["downloads"].contains("artifact")) {
            auto artifact = lib["downloads"]["artifact"];
            std::string url = artifact["url"];
            std::string path = artifact["path"];
            std::string sha1 = artifact.value("sha1", "");
            size_t size = artifact.value("size", 0);
            
            std::string fullPath = librariesDir + "/" + path;
            
            if (!DownloadManager::fileExists(fullPath) ||
                DownloadManager::getFileSize(fullPath) != size) {
                if (gProgressDialog) {
                    gProgressDialog->updateMessage("Downloading libraries...");
                    gProgressDialog->updateFileInfo(path, 0, size);
                }
                if (!DownloadManager::downloadFile(url, fullPath, progressCallback)) {
                    // Try fallback URL
                    std::string fallbackUrl = "https://libraries.minecraft.net/" + path;
                    if (!DownloadManager::downloadFile(fallbackUrl, fullPath, progressCallback)) {
                        std::cerr << "Failed to download library: " << path << std::endl;
                        // Continue anyway, some libraries might not be critical
                    }
                }
            }
        }
        
        // Download classifiers (natives)
        if (lib.contains("downloads") && lib["downloads"].contains("classifiers")) {
            auto classifiers = lib["downloads"]["classifiers"];
            std::string nativeKey;
            
            #ifdef __linux__
            nativeKey = "natives-linux";
            #elif __APPLE__
            nativeKey = "natives-osx";
            #elif _WIN32
            nativeKey = "natives-windows";
            #endif
            
            if (classifiers.contains(nativeKey)) {
                auto native = classifiers[nativeKey];
                std::string url = native["url"];
                std::string path = native["path"];
                
                std::string fullPath = librariesDir + "/" + path;
                if (!DownloadManager::fileExists(fullPath)) {
                    if (gProgressDialog) {
                        gProgressDialog->updateMessage("Downloading native libraries...");
                        gProgressDialog->updateFileInfo(path, 0, 0);
                    }
                    DownloadManager::downloadFile(url, fullPath, progressCallback);
                }
            }
        }
    }
    
    return true;
}

bool MinecraftLauncher::downloadAssets(const std::string& version, const json& versionJson) {
    if (!versionJson.contains("assetIndex")) {
        return true;
    }
    
    auto assetIndex = versionJson["assetIndex"];
    std::string indexUrl = assetIndex["url"];
    std::string indexId = assetIndex["id"];
    std::string indexSha1 = assetIndex.value("sha1", "");
    size_t indexSize = assetIndex.value("size", 0);
    
    std::string indexDir = PathUtils::getMinecraftDir() + "/assets/indexes";
    fs::create_directories(indexDir);
    std::string indexPath = indexDir + "/" + indexId + ".json";
    
    // Download asset index
    if (!DownloadManager::fileExists(indexPath) ||
        DownloadManager::getFileSize(indexPath) != indexSize) {
        std::cout << "Downloading asset index: " << indexId << std::endl;
        if (!DownloadManager::downloadFile(indexUrl, indexPath)) {
            return false;
        }
    }
    
    // Parse asset index
    std::ifstream f(indexPath);
    json indexJson;
    f >> indexJson;
    
    if (!indexJson.contains("objects")) {
        return true;
    }
    
    // Download assets
    std::string objectsDir = PathUtils::getMinecraftDir() + "/assets/objects";
    fs::create_directories(objectsDir);
    
    size_t totalAssets = indexJson["objects"].size();
    size_t currentAsset = 0;
    
    for (const auto& [name, obj] : indexJson["objects"].items()) {
        std::string hash = obj["hash"];
        size_t size = obj["size"];
        
        // First 2 chars of hash = folder name
        std::string prefix = hash.substr(0, 2);
        std::string objectDir = objectsDir + "/" + prefix;
        std::string objectPath = objectDir + "/" + hash;
        
        if (!DownloadManager::fileExists(objectPath) ||
            DownloadManager::getFileSize(objectPath) != size) {
            
            fs::create_directories(objectDir);
            
            std::string url = std::string(DownloadManager::ASSETS_BASE_URL) + "/" + prefix + "/" + hash;
            
            // Update progress every 50 assets
            if (gProgressDialog && (++currentAsset % 50 == 0 || totalAssets < 50)) {
                gProgressDialog->updateMessage(wxString::Format("Downloading assets (%zu/%zu)...", 
                    currentAsset, totalAssets));
            }
            
            if (!DownloadManager::downloadFile(url, objectPath, progressCallback)) {
                std::cerr << "Failed to download asset: " << name << std::endl;
                // Continue with other assets
            }
        }
    }
    
    return true;
}

std::string MinecraftLauncher::buildClasspath(const std::string& version) {
    std::stringstream cp;
    
    // Add version JAR
    std::string versionJar = PathUtils::getMinecraftDir() + "/versions/" + version + "/" + version + ".jar";
    if (DownloadManager::fileExists(versionJar)) {
        cp << versionJar;
    }
    
    // Add libraries
    std::string librariesDir = PathUtils::getMinecraftDir() + "/libraries";
    std::string versionJsonPath = PathUtils::getMinecraftDir() + "/versions/" + version + "/" + version + ".json";
    
    if (DownloadManager::fileExists(versionJsonPath)) {
        std::ifstream f(versionJsonPath);
        json versionJson;
        f >> versionJson;
        
        for (const auto& lib : versionJson["libraries"]) {
            // Skip disallowed libraries
            if (lib.contains("rules")) {
                bool allowed = false;
                for (const auto& rule : lib["rules"]) {
                    std::string action = rule["action"];
                    if (rule.contains("os")) {
                        std::string osName = rule["os"]["name"];
                        #ifdef __linux__
                        if (osName == "linux") allowed = (action == "allow");
                        #elif __APPLE__
                        if (osName == "osx") allowed = (action == "allow");
                        #elif _WIN32
                        if (osName == "windows") allowed = (action == "allow");
                        #endif
                    } else {
                        allowed = (action == "allow");
                    }
                }
                if (!allowed) continue;
            }
            
            // Add artifact to classpath
            if (lib.contains("downloads") && lib["downloads"].contains("artifact")) {
                std::string path = lib["downloads"]["artifact"]["path"];
                std::string fullPath = librariesDir + "/" + path;
                
                if (DownloadManager::fileExists(fullPath)) {
                    cp << ":" << fullPath;
                }
            }
        }
    }
    
    return cp.str();
}

bool MinecraftLauncher::extractNatives(const std::string& version, const std::string& destDir) {
    std::string versionJsonPath = PathUtils::getMinecraftDir() + "/versions/" + version + "/" + version + ".json";
    
    if (!DownloadManager::fileExists(versionJsonPath)) {
        return false;
    }
    
    std::ifstream f(versionJsonPath);
    json versionJson;
    f >> versionJson;
    
    std::string librariesDir = PathUtils::getMinecraftDir() + "/libraries";
    
    // Clear and recreate natives directory
    try {
        fs::remove_all(destDir);
        fs::create_directories(destDir);
    } catch (...) {}
    
    for (const auto& lib : versionJson["libraries"]) {
        // Check if library has natives
        if (!lib.contains("downloads") || !lib["downloads"].contains("classifiers")) {
            continue;
        }
        
        // Check rules
        if (lib.contains("rules")) {
            bool allowed = false;
            for (const auto& rule : lib["rules"]) {
                std::string action = rule["action"];
                if (rule.contains("os")) {
                    std::string osName = rule["os"]["name"];
                    #ifdef __linux__
                    if (osName == "linux") allowed = (action == "allow");
                    #elif __APPLE__
                    if (osName == "osx") allowed = (action == "allow");
                    #elif _WIN32
                    if (osName == "windows") allowed = (action == "allow");
                    #endif
                } else {
                    allowed = (action == "allow");
                }
            }
            if (!allowed) continue;
        }
        
        // Find native artifact
        auto classifiers = lib["downloads"]["classifiers"];
        std::string nativeKey;
        
        #ifdef __linux__
        nativeKey = "natives-linux";
        #elif __APPLE__
        nativeKey = "natives-osx";
        #elif _WIN32
        nativeKey = "natives-windows";
        #endif
        
        if (!classifiers.contains(nativeKey)) continue;
        
        std::string path = classifiers[nativeKey]["path"];
        std::string nativeJar = librariesDir + "/" + path;
        
        if (!DownloadManager::fileExists(nativeJar)) continue;
        
        // Extract JAR using unzip command
        std::string cmd = "unzip -o -q " + nativeJar + " -d " + destDir + " 2>/dev/null";
        std::system(cmd.c_str());
    }
    
    return true;
}

std::string MinecraftLauncher::getMainClass(const std::string& version) {
    std::string versionJsonPath = PathUtils::getMinecraftDir() + "/versions/" + version + "/" + version + ".json";
    
    if (!DownloadManager::fileExists(versionJsonPath)) {
        return "";
    }
    
    std::ifstream f(versionJsonPath);
    json versionJson;
    f >> versionJson;
    
    return versionJson.value("mainClass", "");
}

std::string MinecraftLauncher::getAssetIndex(const std::string& version) {
    std::string versionJsonPath = PathUtils::getMinecraftDir() + "/versions/" + version + "/" + version + ".json";
    
    if (!DownloadManager::fileExists(versionJsonPath)) {
        return "1.8";
    }
    
    std::ifstream f(versionJsonPath);
    json versionJson;
    f >> versionJson;
    
    if (versionJson.contains("assetIndex")) {
        return versionJson["assetIndex"].value("id", "1.8");
    }
    
    return "1.8";
}
