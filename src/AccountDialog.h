#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>

class AccountDialog : public wxDialog {
public:
    AccountDialog(wxWindow* parent);
    
private:
    wxListView* accountList_;
    wxButton* addOfflineBtn_;
    wxButton* addMicrosoftBtn_;
    wxButton* removeBtn_;
    wxButton* setActiveBtn_;
    
    void setupUI();
    void refreshAccountList();
    
    void onAddOffline(wxCommandEvent& event);
    void onAddMicrosoft(wxCommandEvent& event);
    void onRemove(wxCommandEvent& event);
    void onSetActive(wxCommandEvent& event);
    
    enum {
        ID_ADD_OFFLINE = 1001,
        ID_ADD_MICROSOFT,
        ID_REMOVE,
        ID_SET_ACTIVE
    };
    
    wxDECLARE_EVENT_TABLE();
};
