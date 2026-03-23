#include "InstanceSettingsDialog.h"
#include "InstanceManager.h"
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <filesystem>

namespace fs = std::filesystem;

enum {
    ID_SAVE_NAME = 3001,
    ID_OPEN_MODS,
    ID_OPEN_RESOURCE_PACKS,
    ID_OPEN_DIR
};

wxBEGIN_EVENT_TABLE(InstanceSettingsDialog, wxDialog)
    EVT_BUTTON(ID_SAVE_NAME, InstanceSettingsDialog::onSaveName)
    EVT_BUTTON(ID_OPEN_MODS, InstanceSettingsDialog::onOpenMods)
    EVT_BUTTON(ID_OPEN_RESOURCE_PACKS, InstanceSettingsDialog::onOpenResourcePacks)
    EVT_BUTTON(ID_OPEN_DIR, InstanceSettingsDialog::onOpenInstanceDir)
wxEND_EVENT_TABLE()

InstanceSettingsDialog::InstanceSettingsDialog(wxWindow* parent, Instance* instance, size_t instanceIndex)
    : wxDialog(parent, wxID_ANY, "Instance Settings", wxDefaultPosition, wxSize(500, 400)),
      instance_(instance), instanceIndex_(instanceIndex) {
    setupUI();
    updatePaths();
}

void InstanceSettingsDialog::setupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Instance info header
    if (instance_) {
        wxString title = "Managing: " + wxString(instance_->name);
        wxStaticText* headerText = new wxStaticText(this, wxID_ANY, title);
        wxFont boldFont = headerText->GetFont();
        boldFont.SetWeight(wxFONTWEIGHT_BOLD);
        boldFont.SetPointSize(boldFont.GetPointSize() + 2);
        headerText->SetFont(boldFont);
        mainSizer->Add(headerText, 0, wxALL | wxALIGN_CENTER, 15);
    }
    
    // Name editing section
    wxStaticBox* nameBox = new wxStaticBox(this, wxID_ANY, "Instance Name");
    wxBoxSizer* nameSizer = new wxStaticBoxSizer(nameBox, wxVERTICAL);
    
    wxBoxSizer* nameEditSizer = new wxBoxSizer(wxHORIZONTAL);
    nameText_ = new wxTextCtrl(this, wxID_ANY, instance_ ? wxString(instance_->name) : "");
    nameEditSizer->Add(nameText_, 1, wxALL | wxEXPAND, 5);
    saveNameBtn_ = new wxButton(this, ID_SAVE_NAME, "Save");
    nameEditSizer->Add(saveNameBtn_, 0, wxALL, 5);
    nameSizer->Add(nameEditSizer, 0, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(nameSizer, 0, wxEXPAND | wxALL, 10);
    
    // Directories section
    wxStaticBox* dirBox = new wxStaticBox(this, wxID_ANY, "Manage Directories");
    wxBoxSizer* dirSizer = new wxStaticBoxSizer(dirBox, wxVERTICAL);
    
    // Path display
    pathText_ = new wxStaticText(this, wxID_ANY, "Instance path: " + 
                                 (instance_ ? wxString(instance_->gameDir) : ""));
    dirSizer->Add(pathText_, 0, wxALL, 5);
    
    // Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    
    modsBtn_ = new wxButton(this, ID_OPEN_MODS, "Open Mods Folder");
    resourcePacksBtn_ = new wxButton(this, ID_OPEN_RESOURCE_PACKS, "Open Resource Packs");
    openDirBtn_ = new wxButton(this, ID_OPEN_DIR, "Open Instance Folder");
    
    btnSizer->Add(modsBtn_, 0, wxALL, 5);
    btnSizer->Add(resourcePacksBtn_, 0, wxALL, 5);
    btnSizer->Add(openDirBtn_, 0, wxALL, 5);
    
    dirSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 5);
    
    mainSizer->Add(dirSizer, 0, wxEXPAND | wxALL, 10);
    
    // Info text
    wxStaticText* infoText = new wxStaticText(this, wxID_ANY, 
        "Note: Place .jar mod files in the mods folder.\n"
        "Place resource pack .zip files in the resourcepacks folder.");
    wxFont infoFont = infoText->GetFont();
    infoFont.SetPointSize(infoFont.GetPointSize() - 1);
    infoFont.SetStyle(wxFONTSTYLE_ITALIC);
    infoText->SetFont(infoFont);
    mainSizer->Add(infoText, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Close button
    mainSizer->Add(CreateStdDialogButtonSizer(wxOK), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
    Centre();
}

void InstanceSettingsDialog::updatePaths() {
    if (!instance_) return;
    pathText_->SetLabel("Instance path: " + wxString(instance_->gameDir));
}

void InstanceSettingsDialog::onSaveName(wxCommandEvent& event) {
    if (!instance_) return;
    
    wxString newName = nameText_->GetValue();
    if (newName.IsEmpty()) {
        wxMessageBox("Instance name cannot be empty.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    instance_->name = newName.ToStdString();
    InstanceManager::getInstance().updateInstance(*instance_);
    
    wxMessageBox("Instance name updated successfully!", "Success", wxOK | wxICON_INFORMATION, this);
}

void InstanceSettingsDialog::onOpenMods(wxCommandEvent& event) {
    if (!instance_) return;
    
    std::string modsPath = instance_->gameDir + "/.minecraft/mods";
    createDirectoryIfNeeded(modsPath);
    
    if (!openDirectory(modsPath)) {
        wxMessageBox("Failed to open mods directory.\nPath: " + modsPath,
                     "Error", wxOK | wxICON_ERROR, this);
    }
}

void InstanceSettingsDialog::onOpenResourcePacks(wxCommandEvent& event) {
    if (!instance_) return;
    
    std::string packsPath = instance_->gameDir + "/.minecraft/resourcepacks";
    createDirectoryIfNeeded(packsPath);
    
    if (!openDirectory(packsPath)) {
        wxMessageBox("Failed to open resource packs directory.\nPath: " + packsPath,
                     "Error", wxOK | wxICON_ERROR, this);
    }
}

void InstanceSettingsDialog::onOpenInstanceDir(wxCommandEvent& event) {
    if (!instance_) return;
    
    createDirectoryIfNeeded(instance_->gameDir);
    
    if (!openDirectory(instance_->gameDir)) {
        wxMessageBox("Failed to open instance directory.\nPath: " + instance_->gameDir,
                     "Error", wxOK | wxICON_ERROR, this);
    }
}

bool InstanceSettingsDialog::openDirectory(const std::string& path) {
#ifdef _WIN32
    // Windows: use explorer
    std::string cmd = "explorer \"" + path + "\"";
    return std::system(cmd.c_str()) == 0;
#elif __APPLE__
    // macOS: use open
    std::string cmd = "open \"" + path + "\"";
    return std::system(cmd.c_str()) == 0;
#else
    // Linux: try common file managers
    std::vector<std::string> commands = {
        "xdg-open \"" + path + "\"",  // Most common
        "nautilus \"" + path + "\" &",  // GNOME
        "dolphin \"" + path + "\" &",  // KDE
        "thunar \"" + path + "\" &",  // XFCE
        "pcmanfm \"" + path + "\" &"  // LXDE
    };
    
    for (const auto& cmd : commands) {
        if (std::system(cmd.c_str()) == 0) {
            return true;
        }
    }
    return false;
#endif
}

void InstanceSettingsDialog::createDirectoryIfNeeded(const std::string& path) {
    if (!fs::exists(path)) {
        try {
            fs::create_directories(path);
        } catch (...) {
            // Ignore errors, we'll show error in openDirectory
        }
    }
}
