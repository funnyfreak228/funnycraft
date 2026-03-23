#include "SettingsDialog.h"
#include "PathUtils.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, SettingsDialog::onSave)
wxEND_EVENT_TABLE()

static std::string getSettingsPath() {
    return PathUtils::join(PathUtils::getDataDir(), "settings.json");
}

std::string SettingsDialog::getTheme() {
    std::string path = getSettingsPath();
    if (!fs::exists(path)) {
        return "System";
    }
    
    try {
        std::ifstream file(path);
        json j;
        file >> j;
        return j.value("theme", "System");
    } catch (...) {
        return "System";
    }
}

void SettingsDialog::setTheme(const std::string& theme) {
    std::string path = getSettingsPath();
    json j;
    
    if (fs::exists(path)) {
        try {
            std::ifstream inFile(path);
            inFile >> j;
        } catch (...) {}
    }
    
    j["theme"] = theme;
    
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream file(path);
    file << j.dump(4);
}

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(400, 300)) {
    setupUI();
}

void SettingsDialog::setupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Theme selection
    wxStaticBox* themeBox = new wxStaticBox(this, wxID_ANY, "Appearance");
    wxStaticBoxSizer* themeSizer = new wxStaticBoxSizer(themeBox, wxVERTICAL);
    
    wxArrayString themes;
    themes.Add("System");
    themes.Add("Light");
    themes.Add("Dark");
    
    themeChoice_ = new wxChoice(this, ID_THEME, wxDefaultPosition, wxDefaultSize, themes);
    themeChoice_->SetStringSelection(getTheme());
    
    themeSizer->Add(new wxStaticText(this, wxID_ANY, "Theme:"), 0, wxALL, 5);
    themeSizer->Add(themeChoice_, 0, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(themeSizer, 0, wxEXPAND | wxALL, 10);
    
    // Buttons
    mainSizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
}

void SettingsDialog::onSave(wxCommandEvent& event) {
    setTheme(themeChoice_->GetStringSelection().ToStdString());
    EndModal(wxID_OK);
}
