#include "PathUtils.h"
#include <cstdlib>
#include <unistd.h>
#include <climits>
#include <filesystem>

namespace fs = std::filesystem;

std::string PathUtils::getExecutableDir() {
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return fs::path(path).parent_path().string();
    }
    // Fallback to current working directory
    return fs::current_path().string();
}

std::string PathUtils::getHomeDir() {
    const char* home = getenv("HOME");
    if (home) {
        return home;
    }
    return ".";
}

std::string PathUtils::getDataDir() {
    // Check for environment variable override
    const char* dataDir = getenv("FUNNYCRAFT_DATA_DIR");
    if (dataDir) {
        return dataDir;
    }
    // Default to ~/.funnycraft
    return join(getHomeDir(), ".funnycraft");
}

std::string PathUtils::getMinecraftDir() {
    // Check for environment variable override
    const char* mcDir = getenv("MINECRAFT_DIR");
    if (mcDir) {
        return mcDir;
    }
    // Default to executable directory/.minecraft for portability
    // or ~/.minecraft if user wants standard location
    const char* portable = getenv("FUNNYCRAFT_PORTABLE");
    if (portable && std::string(portable) == "1") {
        return join(getExecutableDir(), ".minecraft");
    }
    // Default to standard Minecraft location
    return join(getHomeDir(), ".minecraft");
}

std::string PathUtils::join(const std::string& a, const std::string& b) {
    return (fs::path(a) / fs::path(b)).string();
}

std::string PathUtils::join(const std::string& a, const std::string& b, const std::string& c) {
    return (fs::path(a) / fs::path(b) / fs::path(c)).string();
}
