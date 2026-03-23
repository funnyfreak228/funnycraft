#pragma once

#include <wx/wx.h>

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent);
    
    static std::string getTheme();
    static void setTheme(const std::string& theme);
    
private:
    wxChoice* themeChoice_;
    
    void setupUI();
    void onSave(wxCommandEvent& event);
    
    enum {
        ID_THEME = 2001
    };
    
    wxDECLARE_EVENT_TABLE();
};
