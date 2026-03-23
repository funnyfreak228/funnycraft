#include "MainFrame.h"
#include "AccountDialog.h"
#include "InstanceDialog.h"
#include "InstanceSettingsDialog.h"
#include "SettingsDialog.h"
#include "AccountManager.h"
#include "InstanceManager.h"
#include "MinecraftLauncher.h"
#include <wx/aboutdlg.h>

enum {
    ID_LAUNCH = 3001,
    ID_ACCOUNTS,
    ID_INSTANCES,
    ID_INSTANCE_SETTINGS,
    ID_ACCOUNT_CHOICE,
    ID_SETTINGS
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_LAUNCH, MainFrame::onLaunch)
    EVT_BUTTON(ID_ACCOUNTS, MainFrame::onAccounts)
    EVT_BUTTON(ID_INSTANCES, MainFrame::onInstances)
    EVT_BUTTON(ID_INSTANCE_SETTINGS, MainFrame::onInstanceSettings)
wxEND_EVENT_TABLE()

MainFrame::MainFrame() 
    : wxFrame(nullptr, wxID_ANY, "FunnyCraft - The Free & Open Source Minecraft Launcher", 
              wxDefaultPosition, wxSize(700, 500)) {
    
    // Load data
    AccountManager::getInstance().loadAccounts();
    InstanceManager::getInstance().loadInstances();
    
    setupUI();
    refreshUI();
    
    Centre();
}

void MainFrame::setupUI() {
    // Menu bar
    wxMenuBar* menuBar = new wxMenuBar;
    
    // File menu
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X", "Exit FunnyCraft");
    menuBar->Append(fileMenu, "&File");
    
    // Tools menu
    wxMenu* toolsMenu = new wxMenu;
    toolsMenu->Append(ID_SETTINGS, "&Settings\tCtrl+,", "Open settings");
    menuBar->Append(toolsMenu, "&Tools");
    
    // Help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
    menuBar->Append(helpMenu, "&Help");
    
    SetMenuBar(menuBar);
    
    Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::onSettings, this, ID_SETTINGS);
    Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
    
    // Main panel
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Top section - Account selection
    wxStaticBoxSizer* accountSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Account");
    accountChoice_ = new wxChoice(panel, ID_ACCOUNT_CHOICE);
    accountsBtn_ = new wxButton(panel, ID_ACCOUNTS, "Manage...");
    accountSizer->Add(accountChoice_, 1, wxEXPAND | wxALL, 5);
    accountSizer->Add(accountsBtn_, 0, wxALL, 5);
    mainSizer->Add(accountSizer, 0, wxEXPAND | wxALL, 10);
    
    // Middle section - Instance list
    wxStaticBoxSizer* instanceSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Instances");
    instanceList_ = new wxListView(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxLC_REPORT | wxLC_SINGLE_SEL);
    instanceList_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
    instanceList_->AppendColumn("Version", wxLIST_FORMAT_LEFT, 100);
    instanceList_->AppendColumn("Memory", wxLIST_FORMAT_LEFT, 80);
    instanceList_->AppendColumn("Directory", wxLIST_FORMAT_LEFT, 250);
    instanceSizer->Add(instanceList_, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(instanceSizer, 1, wxEXPAND | wxALL, 10);
    
    // Bottom section - Buttons and status
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    
    instancesBtn_ = new wxButton(panel, ID_INSTANCES, "Instances...");
    instanceSettingsBtn_ = new wxButton(panel, ID_INSTANCE_SETTINGS, "Settings...");
    launchBtn_ = new wxButton(panel, ID_LAUNCH, "Play!");
    launchBtn_->SetFont(launchBtn_->GetFont().Bold());
    launchBtn_->SetMinSize(wxSize(100, 40));
    
    bottomSizer->Add(instancesBtn_, 0, wxALL, 5);
    bottomSizer->Add(instanceSettingsBtn_, 0, wxALL, 5);
    bottomSizer->AddStretchSpacer(1);
    bottomSizer->Add(launchBtn_, 0, wxALL, 5);
    
    mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALL, 10);
    
    // Status bar
    statusBar_ = new wxStatusBar(this);
    SetStatusBar(statusBar_);
    
    panel->SetSizer(mainSizer);
}

void MainFrame::refreshUI() {
    // Update account choice
    accountChoice_->Clear();
    auto& accounts = AccountManager::getInstance().getAccounts();
    int activeIdx = 0;
    for (size_t i = 0; i < accounts.size(); ++i) {
        accountChoice_->Append(accounts[i].username);
        if (accounts[i].isActive) {
            activeIdx = i;
        }
    }
    if (!accounts.empty()) {
        accountChoice_->SetSelection(activeIdx);
    }
    
    // Update instance list
    instanceList_->DeleteAllItems();
    auto& instances = InstanceManager::getInstance().getInstances();
    for (size_t i = 0; i < instances.size(); ++i) {
        auto& inst = instances[i];
        long idx = instanceList_->InsertItem(i, inst.name);
        instanceList_->SetItem(idx, 1, inst.version);
        instanceList_->SetItem(idx, 2, std::to_string(inst.memoryMB) + " MB");
        instanceList_->SetItem(idx, 3, inst.gameDir);
    }
    
    // Select first instance if available
    if (instances.size() > 0) {
        instanceList_->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void MainFrame::onLaunch(wxCommandEvent& event) {
    // Get selected account
    int accIdx = accountChoice_->GetSelection();
    if (accIdx == wxNOT_FOUND) {
        wxMessageBox("Please select an account.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    auto& accounts = AccountManager::getInstance().getAccounts();
    if (accIdx >= static_cast<int>(accounts.size())) {
        wxMessageBox("Invalid account selected.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    // Get selected instance
    long instIdx = instanceList_->GetFirstSelected();
    if (instIdx == -1) {
        wxMessageBox("Please select an instance.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    auto& instances = InstanceManager::getInstance().getInstances();
    if (instIdx >= static_cast<long>(instances.size())) {
        wxMessageBox("Invalid instance selected.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    // Launch
    statusBar_->SetStatusText("Launching Minecraft...");
    bool success = MinecraftLauncher::launch(&instances[instIdx], &accounts[accIdx], this);
    
    if (success) {
        statusBar_->SetStatusText("Minecraft launched successfully!");
    } else {
        statusBar_->SetStatusText("Failed to launch Minecraft.");
        wxMessageBox("Failed to launch Minecraft. Make sure Java is installed and check the console for errors.", 
                    "Error", wxOK | wxICON_ERROR);
    }
}

void MainFrame::onAccounts(wxCommandEvent& event) {
    AccountDialog dlg(this);
    dlg.ShowModal();
    refreshUI();
}

void MainFrame::onInstances(wxCommandEvent& event) {
    InstanceDialog dlg(this);
    dlg.ShowModal();
    refreshUI();
}

void MainFrame::onInstanceSettings(wxCommandEvent& event) {
    // Get selected instance
    long instIdx = instanceList_->GetFirstSelected();
    if (instIdx == -1) {
        wxMessageBox("Please select an instance first.", "No Instance Selected", 
                     wxOK | wxICON_INFORMATION, this);
        return;
    }
    
    auto& instances = InstanceManager::getInstance().getInstances();
    if (instIdx >= static_cast<long>(instances.size())) {
        wxMessageBox("Invalid instance selected.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    Instance* instance = &instances[instIdx];
    InstanceSettingsDialog dialog(this, instance, static_cast<size_t>(instIdx));
    dialog.ShowModal();
    refreshUI();
}

void MainFrame::onSettings(wxCommandEvent& event) {
    SettingsDialog dlg(this);
    dlg.ShowModal();
}

void MainFrame::onExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::onAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName("FunnyCraft");
    info.SetVersion("1.0");
    info.SetDescription("A simple Minecraft launcher built with C++ and wxWidgets.\n"
                        "Supports Minecraft Java 1.8-1.21.11 with multiple instances.");
    info.SetCopyright("funnyfreak - (C) 2024");
    wxAboutBox(info);
}
