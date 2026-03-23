#include "DownloadManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <array>
#include <memory>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

// Execute command and capture output
static std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Get file size from HTTP header using curl
static int64_t getRemoteFileSize(const std::string& url) {
    std::string cmd = "curl -sI \"" + url + "\" | grep -i content-length | awk '{print $2}' | tr -d '\\r'";
    std::string result = exec(cmd.c_str());
    
    // Trim whitespace
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    
    try {
        return std::stoll(result);
    } catch (...) {
        return -1; // Unknown size
    }
}

bool DownloadManager::downloadFile(const std::string& url, const std::string& destPath,
                                   ProgressCallback callback) {
    // Create directory if needed
    fs::create_directories(fs::path(destPath).parent_path());
    
    std::string filename = fs::path(destPath).filename().string();
    
    // Get remote file size for progress calculation
    int64_t totalSize = -1;
    if (callback) {
        totalSize = getRemoteFileSize(url);
        callback(filename, 0, totalSize > 0 ? totalSize : 0);
    }
    
    // Use curl with progress bar for better feedback
    // -# shows progress bar, --progress-bar for cleaner output
    std::string cmd = "curl -fSL --connect-timeout 30 --max-time 300 ";
    cmd += "-A \"FunnyCraft/1.0\" ";
    cmd += "\"" + url + "\" ";
    cmd += "-o \"" + destPath + "\"";
    
    // If we have a callback, run download in a thread and monitor file size
    if (callback && totalSize > 0) {
        // Start download in background
        std::thread downloadThread([cmd]() {
            std::system(cmd.c_str());
        });
        
        // Monitor progress
        int64_t lastSize = 0;
        int stallCount = 0;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            int64_t currentSize = 0;
            if (fs::exists(destPath)) {
                try {
                    currentSize = fs::file_size(destPath);
                } catch (...) {}
            }
            
            callback(filename, currentSize, totalSize);
            
            // Check if download is complete or stalled
            if (currentSize >= totalSize) {
                break;
            }
            if (currentSize == lastSize) {
                stallCount++;
                // If stalled for too long, might be done or failed
                if (stallCount > 50) { // 10 seconds
                    break;
                }
            } else {
                stallCount = 0;
            }
            lastSize = currentSize;
        }
        
        downloadThread.join();
        
        // Final callback
        int64_t finalSize = 0;
        if (fs::exists(destPath)) {
            try {
                finalSize = fs::file_size(destPath);
            } catch (...) {}
        }
        callback(filename, finalSize, totalSize);
        
        // Check if download succeeded
        if (finalSize < totalSize * 0.9) { // Allow some tolerance
            fs::remove(destPath);
            return false;
        }
    } else {
        // Simple download without progress
        int result = std::system(cmd.c_str());
        if (result != 0) {
            fs::remove(destPath);
            return false;
        }
    }
    
    return true;
}

std::string DownloadManager::downloadString(const std::string& url) {
    // Use curl command line
    std::string cmd = "curl -fsSL --connect-timeout 30 --max-time 60 ";
    cmd += "-A \"FunnyCraft/1.0\" ";
    cmd += "\"" + url + "\"";
    
    return exec(cmd.c_str());
}

bool DownloadManager::fileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

size_t DownloadManager::getFileSize(const std::string& path) {
    if (fileExists(path)) {
        return fs::file_size(path);
    }
    return 0;
}
