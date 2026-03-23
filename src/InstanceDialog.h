#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>

class InstanceDialog : public wxDialog {
public:
    InstanceDialog(wxWindow* parent);
    
private:
    wxListView* instanceList_;
    wxButton* createBtn_;
    wxButton* deleteBtn_;
    wxButton* editBtn_;
    
    void setupUI();
    void refreshInstanceList();
    
    void onCreate(wxCommandEvent& event);
    void onDelete(wxCommandEvent& event);
    void onEdit(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};
