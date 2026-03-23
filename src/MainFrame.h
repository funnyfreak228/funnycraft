#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/choice.h>

class MainFrame : public wxFrame {
public:
    MainFrame();
    
private:
    wxChoice* accountChoice_;
    wxListView* instanceList_;
    wxButton* launchBtn_;
    wxButton* accountsBtn_;
    wxButton* instancesBtn_;
    wxButton* instanceSettingsBtn_;
    wxStatusBar* statusBar_;
    
    void setupUI();
    void refreshUI();
    
    void onLaunch(wxCommandEvent& event);
    void onAccounts(wxCommandEvent& event);
    void onInstances(wxCommandEvent& event);
    void onInstanceSettings(wxCommandEvent& event);
    void onSettings(wxCommandEvent& event);
    void onExit(wxCommandEvent& event);
    void onAbout(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};
