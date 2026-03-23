#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Instance {
    std::string id;
    std::string name;
    std::string version;
    std::string gameDir;
    std::string javaPath;
    int memoryMB;
    std::string jvmArgs;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Instance, id, name, version, gameDir, javaPath, memoryMB, jvmArgs)
};

class InstanceManager {
public:
    static InstanceManager& getInstance();
    
    void loadInstances();
    void saveInstances();
    
    std::vector<Instance>& getInstances();
    Instance* getInstance(const std::string& id);
    
    void createInstance(const std::string& name, const std::string& version);
    void deleteInstance(const std::string& id);
    void updateInstance(const Instance& instance);
    
    std::string getInstancesDir();
    
private:
    InstanceManager() = default;
    std::vector<Instance> instances_;
    std::string getInstancesFilePath();
    std::string generateInstanceId();
};
