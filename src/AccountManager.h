#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class AccountType {
    OFFLINE,
    MOJANG,
    MICROSOFT
};

struct Account {
    std::string username;
    std::string uuid;
    std::string accessToken;
    std::string refreshToken;
    AccountType type;
    bool isActive;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Account, username, uuid, accessToken, refreshToken, type, isActive)
};

class AccountManager {
public:
    static AccountManager& getInstance();
    
    void loadAccounts();
    void saveAccounts();
    
    std::vector<Account>& getAccounts();
    Account* getActiveAccount();
    
    void addOfflineAccount(const std::string& username);
    bool addMicrosoftAccount(const std::string& authCode);
    void removeAccount(size_t index);
    void setActiveAccount(size_t index);
    
private:
    AccountManager() = default;
    std::vector<Account> accounts_;
    std::string getAccountsFilePath();
    std::string generateOfflineUUID(const std::string& username);
};
