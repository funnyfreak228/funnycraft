#include "InstanceDialog.h"
#include "InstanceManager.h"
#include <wx/textdlg.h>

enum {
    ID_CREATE = 2001,
    ID_DELETE,
    ID_EDIT
};

wxBEGIN_EVENT_TABLE(InstanceDialog, wxDialog)
    EVT_BUTTON(ID_CREATE, InstanceDialog::onCreate)
    EVT_BUTTON(ID_DELETE, InstanceDialog::onDelete)
    EVT_BUTTON(ID_EDIT, InstanceDialog::onEdit)
wxEND_EVENT_TABLE()

InstanceDialog::InstanceDialog(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Instance Management", wxDefaultPosition, wxSize(600, 400)) {
    setupUI();
    refreshInstanceList();
}

void InstanceDialog::setupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Instance list
    instanceList_ = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                    wxLC_REPORT | wxLC_SINGLE_SEL);
    instanceList_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
    instanceList_->AppendColumn("Version", wxLIST_FORMAT_LEFT, 100);
    instanceList_->AppendColumn("Memory", wxLIST_FORMAT_LEFT, 80);
    instanceList_->AppendColumn("Directory", wxLIST_FORMAT_LEFT, 200);
    mainSizer->Add(instanceList_, 1, wxEXPAND | wxALL, 10);
    
    // Button sizer
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    
    createBtn_ = new wxButton(this, ID_CREATE, "Create Instance");
    deleteBtn_ = new wxButton(this, ID_DELETE, "Delete Instance");
    editBtn_ = new wxButton(this, ID_EDIT, "Edit Settings");
    
    btnSizer->Add(createBtn_, 0, wxALL, 5);
    btnSizer->Add(deleteBtn_, 0, wxALL, 5);
    btnSizer->Add(editBtn_, 0, wxALL, 5);
    
    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 5);
    
    // Close button
    mainSizer->Add(CreateStdDialogButtonSizer(wxOK), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
}

void InstanceDialog::refreshInstanceList() {
    instanceList_->DeleteAllItems();
    
    auto& instances = InstanceManager::getInstance().getInstances();
    for (size_t i = 0; i < instances.size(); ++i) {
        auto& inst = instances[i];
        long idx = instanceList_->InsertItem(i, inst.name);
        instanceList_->SetItem(idx, 1, inst.version);
        instanceList_->SetItem(idx, 2, std::to_string(inst.memoryMB) + " MB");
        instanceList_->SetItem(idx, 3, inst.gameDir);
    }
}

void InstanceDialog::onCreate(wxCommandEvent& event) {
    wxString name = wxGetTextFromUser("Enter instance name:", "Create Instance", "New Instance", this);
    if (!name.IsEmpty()) {
        wxArrayString versions;
        // Latest versions (1.21.x)
        versions.Add("1.21.11");
        versions.Add("1.21.10");
        versions.Add("1.21.9");
        versions.Add("1.21.8");
        versions.Add("1.21.7");
        versions.Add("1.21.6");
        versions.Add("1.21.5");
        versions.Add("1.21.4");
        versions.Add("1.21.3");
        versions.Add("1.21.2");
        versions.Add("1.21.1");
        versions.Add("1.21");
        // 1.20.x versions
        versions.Add("1.20.6");
        versions.Add("1.20.5");
        versions.Add("1.20.4");
        versions.Add("1.20.3");
        versions.Add("1.20.2");
        versions.Add("1.20.1");
        versions.Add("1.20");
        // 1.19.x versions
        versions.Add("1.19.4");
        versions.Add("1.19.3");
        versions.Add("1.19.2");
        versions.Add("1.19.1");
        versions.Add("1.19");
        // 1.18.x versions
        versions.Add("1.18.2");
        versions.Add("1.18.1");
        versions.Add("1.18");
        // 1.17.x versions
        versions.Add("1.17.1");
        versions.Add("1.17");
        // 1.16.x versions
        versions.Add("1.16.5");
        versions.Add("1.16.4");
        versions.Add("1.16.3");
        versions.Add("1.16.2");
        versions.Add("1.16.1");
        versions.Add("1.16");
        // 1.15.x versions
        versions.Add("1.15.2");
        versions.Add("1.15.1");
        versions.Add("1.15");
        // 1.14.x versions
        versions.Add("1.14.4");
        versions.Add("1.14.3");
        versions.Add("1.14.2");
        versions.Add("1.14.1");
        versions.Add("1.14");
        // 1.13.x versions
        versions.Add("1.13.2");
        versions.Add("1.13.1");
        versions.Add("1.13");
        // 1.12.x versions
        versions.Add("1.12.2");
        versions.Add("1.12.1");
        versions.Add("1.12");
        // 1.11.x versions
        versions.Add("1.11.2");
        versions.Add("1.11.1");
        versions.Add("1.11");
        // 1.10.x versions
        versions.Add("1.10.2");
        versions.Add("1.10.1");
        versions.Add("1.10");
        // 1.9.x versions
        versions.Add("1.9.4");
        versions.Add("1.9.3");
        versions.Add("1.9.2");
        versions.Add("1.9.1");
        versions.Add("1.9");
        // 1.8.x versions (minimum supported)
        versions.Add("1.8.9");
        versions.Add("1.8.8");
        versions.Add("1.8.7");
        versions.Add("1.8.6");
        versions.Add("1.8.5");
        versions.Add("1.8.4");
        versions.Add("1.8.3");
        versions.Add("1.8.2");
        versions.Add("1.8.1");
        versions.Add("1.8");
        
        wxSingleChoiceDialog versionDlg(this, "Select Minecraft version:", "Version", versions);
        if (versionDlg.ShowModal() == wxID_OK) {
            std::string version = versionDlg.GetStringSelection().ToStdString();
            InstanceManager::getInstance().createInstance(name.ToStdString(), version);
            refreshInstanceList();
        }
    }
}

void InstanceDialog::onDelete(wxCommandEvent& event) {
    long idx = instanceList_->GetFirstSelected();
    if (idx != -1) {
        auto& instances = InstanceManager::getInstance().getInstances();
        if (idx < static_cast<long>(instances.size())) {
            wxString msg = "Delete instance '" + instances[idx].name + "'?";
            if (wxMessageBox(msg, "Confirm", wxYES_NO | wxICON_QUESTION, this) == wxYES) {
                InstanceManager::getInstance().deleteInstance(instances[idx].id);
                refreshInstanceList();
            }
        }
    }
}

void InstanceDialog::onEdit(wxCommandEvent& event) {
    long idx = instanceList_->GetFirstSelected();
    if (idx != -1) {
        auto& instances = InstanceManager::getInstance().getInstances();
        if (idx < static_cast<long>(instances.size())) {
            Instance& inst = instances[idx];
            
            wxString memStr = wxGetTextFromUser("Memory (MB):", "Edit Instance", 
                                                 std::to_string(inst.memoryMB), this);
            if (!memStr.IsEmpty()) {
                long mem;
                if (memStr.ToLong(&mem)) {
                    inst.memoryMB = static_cast<int>(mem);
                    InstanceManager::getInstance().updateInstance(inst);
                    refreshInstanceList();
                }
            }
        }
    }
}
