#include "AccountManager.h"
#include "PathUtils.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <random>

namespace fs = std::filesystem;
using json = nlohmann::json;

AccountManager& AccountManager::getInstance() {
    static AccountManager instance;
    return instance;
}

void AccountManager::loadAccounts() {
    std::string path = getAccountsFilePath();
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
        accounts_ = j.get<std::vector<Account>>();
    } catch (...) {
        accounts_.clear();
    }
}

void AccountManager::saveAccounts() {
    std::string path = getAccountsFilePath();
    fs::create_directories(fs::path(path).parent_path());
    
    std::ofstream file(path);
    if (file.is_open()) {
        json j = accounts_;
        file << j.dump(4);
    }
}

std::vector<Account>& AccountManager::getAccounts() {
    return accounts_;
}

Account* AccountManager::getActiveAccount() {
    for (auto& acc : accounts_) {
        if (acc.isActive) {
            return &acc;
        }
    }
    return accounts_.empty() ? nullptr : &accounts_[0];
}

void AccountManager::addOfflineAccount(const std::string& username) {
    Account acc;
    acc.username = username;
    acc.uuid = generateOfflineUUID(username);
    acc.accessToken = "null";
    acc.refreshToken = "";
    acc.type = AccountType::OFFLINE;
    acc.isActive = accounts_.empty();
    
    accounts_.push_back(acc);
    saveAccounts();
}

bool AccountManager::addMicrosoftAccount(const std::string& authCode) {
    // TODO: Implement Microsoft OAuth flow
    // For now, create a placeholder account
    Account acc;
    acc.username = "MicrosoftUser";
    acc.uuid = generateOfflineUUID(acc.username);
    acc.accessToken = "ms_token_placeholder";
    acc.refreshToken = "";
    acc.type = AccountType::MICROSOFT;
    acc.isActive = accounts_.empty();
    
    accounts_.push_back(acc);
    saveAccounts();
    return true;
}

void AccountManager::removeAccount(size_t index) {
    if (index < accounts_.size()) {
        accounts_.erase(accounts_.begin() + index);
        if (!accounts_.empty() && !getActiveAccount()) {
            accounts_[0].isActive = true;
        }
        saveAccounts();
    }
}

void AccountManager::setActiveAccount(size_t index) {
    if (index < accounts_.size()) {
        for (auto& acc : accounts_) {
            acc.isActive = false;
        }
        accounts_[index].isActive = true;
        saveAccounts();
    }
}

std::string AccountManager::getAccountsFilePath() {
    return PathUtils::join(PathUtils::getDataDir(), "accounts.json");
}

std::string AccountManager::generateOfflineUUID(const std::string& username) {
    // Generate offline UUID (format: 00000000-0000-0000-0000-000000000000 based on username hash)
    std::hash<std::string> hasher;
    size_t hash = hasher(username);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << (hash & 0xFFFFFFFF) << "-";
    ss << std::setw(4) << ((hash >> 32) & 0xFFFF) << "-";
    ss << std::setw(4) << ((hash >> 48) & 0x0FFF) << "-";
    ss << std::setw(4) << ((hash >> 60) & 0x3FFF) << "-";
    ss << std::setw(12) << (hash & 0xFFFFFFFFFFFF);
    
    return ss.str();
}
