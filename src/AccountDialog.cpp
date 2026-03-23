#include "AccountDialog.h"
#include "AccountManager.h"
#include <wx/textdlg.h>

wxBEGIN_EVENT_TABLE(AccountDialog, wxDialog)
    EVT_BUTTON(ID_ADD_OFFLINE, AccountDialog::onAddOffline)
    EVT_BUTTON(ID_ADD_MICROSOFT, AccountDialog::onAddMicrosoft)
    EVT_BUTTON(ID_REMOVE, AccountDialog::onRemove)
    EVT_BUTTON(ID_SET_ACTIVE, AccountDialog::onSetActive)
wxEND_EVENT_TABLE()

AccountDialog::AccountDialog(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Account Management", wxDefaultPosition, wxSize(500, 400)) {
    setupUI();
    refreshAccountList();
}

void AccountDialog::setupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Account list
    accountList_ = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
    accountList_->AppendColumn("Username", wxLIST_FORMAT_LEFT, 150);
    accountList_->AppendColumn("Type", wxLIST_FORMAT_LEFT, 100);
    accountList_->AppendColumn("Active", wxLIST_FORMAT_LEFT, 80);
    mainSizer->Add(accountList_, 1, wxEXPAND | wxALL, 10);
    
    // Button sizer
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    
    addOfflineBtn_ = new wxButton(this, ID_ADD_OFFLINE, "Add Offline");
    addMicrosoftBtn_ = new wxButton(this, ID_ADD_MICROSOFT, "Add Microsoft");
    removeBtn_ = new wxButton(this, ID_REMOVE, "Remove");
    setActiveBtn_ = new wxButton(this, ID_SET_ACTIVE, "Set Active");
    
    btnSizer->Add(addOfflineBtn_, 0, wxALL, 5);
    btnSizer->Add(addMicrosoftBtn_, 0, wxALL, 5);
    btnSizer->Add(removeBtn_, 0, wxALL, 5);
    btnSizer->Add(setActiveBtn_, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 5);
    
    // Close button
    mainSizer->Add(CreateStdDialogButtonSizer(wxOK), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
}

void AccountDialog::refreshAccountList() {
    accountList_->DeleteAllItems();
    
    auto& accounts = AccountManager::getInstance().getAccounts();
    for (size_t i = 0; i < accounts.size(); ++i) {
        auto& acc = accounts[i];
        long idx = accountList_->InsertItem(i, acc.username);
        std::string typeStr;
        switch (acc.type) {
            case AccountType::OFFLINE: typeStr = "Offline"; break;
            case AccountType::MOJANG: typeStr = "Mojang"; break;
            case AccountType::MICROSOFT: typeStr = "Microsoft"; break;
        }
        accountList_->SetItem(idx, 1, typeStr);
        accountList_->SetItem(idx, 2, acc.isActive ? "Yes" : "No");
    }
}

void AccountDialog::onAddOffline(wxCommandEvent& event) {
    wxString username = wxGetTextFromUser("Enter username:", "Add Offline Account", "", this);
    if (!username.IsEmpty()) {
        AccountManager::getInstance().addOfflineAccount(username.ToStdString());
        refreshAccountList();
    }
}

void AccountDialog::onAddMicrosoft(wxCommandEvent& event) {
    wxMessageBox("Microsoft authentication will be implemented in the next update.\n"
                 "For now, please use offline accounts.", 
                 "Coming Soon", wxOK | wxICON_INFORMATION, this);
}

void AccountDialog::onRemove(wxCommandEvent& event) {
    long idx = accountList_->GetFirstSelected();
    if (idx != -1) {
        AccountManager::getInstance().removeAccount(idx);
        refreshAccountList();
    }
}

void AccountDialog::onSetActive(wxCommandEvent& event) {
    long idx = accountList_->GetFirstSelected();
    if (idx != -1) {
        AccountManager::getInstance().setActiveAccount(idx);
        refreshAccountList();
    }
}
