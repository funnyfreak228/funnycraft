#include "InstanceManager.h"
#include "PathUtils.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace fs = std::filesystem;
using json = nlohmann::json;

InstanceManager& InstanceManager::getInstance() {
    static InstanceManager instance;
    return instance;
}

void InstanceManager::loadInstances() {
    std::string path = getInstancesFilePath();
    if (!fs::exists(path)) {
        return;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }
    
    try {
        json j;
        file >> j;
        instances_ = j.get<std::vector<Instance>>();
    } catch (...) {
        instances_.clear();
    }
}

void InstanceManager::saveInstances() {
    std::string path = getInstancesFilePath();
    fs::create_directories(fs::path(path).parent_path());
    
    std::ofstream file(path);
    if (file.is_open()) {
        json j = instances_;
        file << j.dump(4);
    }
}

std::vector<Instance>& InstanceManager::getInstances() {
    return instances_;
}

Instance* InstanceManager::getInstance(const std::string& id) {
    for (auto& inst : instances_) {
        if (inst.id == id) {
            return &inst;
        }
    }
    return nullptr;
}

void InstanceManager::createInstance(const std::string& name, const std::string& version) {
    Instance inst;
    inst.id = generateInstanceId();
    inst.name = name;
    inst.version = version;
    inst.gameDir = PathUtils::join(PathUtils::getDataDir(), "instances", inst.id);
    inst.javaPath = "java";
    inst.memoryMB = 2048;
    inst.jvmArgs = "-XX:+UseG1GC -XX:+UnlockExperimentalVMOptions -XX:MaxGCPauseMillis=200";
    
    // Create instance directory
    fs::create_directories(inst.gameDir);
    fs::create_directories(PathUtils::join(inst.gameDir, ".minecraft"));
    fs::create_directories(PathUtils::join(inst.gameDir, ".minecraft", "mods"));
    fs::create_directories(PathUtils::join(inst.gameDir, ".minecraft", "saves"));
    fs::create_directories(PathUtils::join(inst.gameDir, ".minecraft", "resourcepacks"));
    
    instances_.push_back(inst);
    saveInstances();
}

void InstanceManager::deleteInstance(const std::string& id) {
    for (auto it = instances_.begin(); it != instances_.end(); ++it) {
        if (it->id == id) {
            // Remove instance directory
            try {
                fs::remove_all(it->gameDir);
            } catch (...) {}
            
            instances_.erase(it);
            saveInstances();
            return;
        }
    }
}

void InstanceManager::updateInstance(const Instance& instance) {
    for (auto& inst : instances_) {
        if (inst.id == instance.id) {
            inst = instance;
            saveInstances();
            return;
        }
    }
}

std::string InstanceManager::getInstancesDir() {
    return PathUtils::join(PathUtils::getDataDir(), "instances");
}

std::string InstanceManager::getInstancesFilePath() {
    return PathUtils::join(PathUtils::getDataDir(), "instances.json");
}

std::string InstanceManager::generateInstanceId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::stringstream ss;
    ss << "inst_" << std::hex << ms;
    return ss.str();
}
