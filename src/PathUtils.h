#pragma once

#include <string>

class PathUtils {
public:
    // Get directory where executable is located
    static std::string getExecutableDir();
    
    // Get user's home directory
    static std::string getHomeDir();
    
    // Get data directory (where accounts/instances are stored)
    // Uses FUNNYCRAFT_DATA_DIR env var, or falls back to ~/.funnycraft
    static std::string getDataDir();
    
    // Get Minecraft directory (where Mojang files are downloaded)
    // Uses executable directory for portability
    static std::string getMinecraftDir();
    
    // Join paths
    static std::string join(const std::string& a, const std::string& b);
    static std::string join(const std::string& a, const std::string& b, const std::string& c);
};
