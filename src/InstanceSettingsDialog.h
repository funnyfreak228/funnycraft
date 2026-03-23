#pragma once

#include <wx/wx.h>
#include "InstanceManager.h"

class InstanceSettingsDialog : public wxDialog {
public:
    InstanceSettingsDialog(wxWindow* parent, Instance* instance, size_t instanceIndex);
    
private:
    Instance* instance_;
    size_t instanceIndex_;
    
    // UI Elements
    wxTextCtrl* nameText_;
    wxStaticText* pathText_;
    wxButton* saveNameBtn_;
    wxButton* modsBtn_;
    wxButton* resourcePacksBtn_;
    wxButton* openDirBtn_;
    
    void setupUI();
    void updatePaths();
    
    void onSaveName(wxCommandEvent& event);
    void onOpenMods(wxCommandEvent& event);
    void onOpenResourcePacks(wxCommandEvent& event);
    void onOpenInstanceDir(wxCommandEvent& event);
    
    bool openDirectory(const std::string& path);
    void createDirectoryIfNeeded(const std::string& path);
    
    wxDECLARE_EVENT_TABLE();
};
