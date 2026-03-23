#include "ProgressDialog.h"
#include <sstream>
#include <iomanip>

wxDEFINE_EVENT(wxEVT_THREAD_UPDATE, wxCommandEvent);

wxBEGIN_EVENT_TABLE(ProgressDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, ProgressDialog::onCancel)
    EVT_CLOSE(ProgressDialog::onClose)
    EVT_COMMAND(wxID_ANY, wxEVT_THREAD_UPDATE, ProgressDialog::onThreadUpdate)
wxEND_EVENT_TABLE()

ProgressDialog::ProgressDialog(wxWindow* parent, const wxString& title, int maxValue)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(500, 200),
               wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP),
      cancelled_(false), completed_(false) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Message text
    messageText_ = new wxStaticText(this, wxID_ANY, "Preparing download...", 
                                     wxDefaultPosition, wxDefaultSize,
                                     wxALIGN_LEFT);
    messageText_->SetFont(messageText_->GetFont().Bold());
    mainSizer->Add(messageText_, 0, wxEXPAND | wxALL, 10);
    
    // Progress gauge
    gauge_ = new wxGauge(this, wxID_ANY, 100, 
                          wxDefaultPosition, wxSize(-1, 25),
                          wxGA_HORIZONTAL | wxGA_SMOOTH);
    mainSizer->Add(gauge_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // File info
    fileText_ = new wxStaticText(this, wxID_ANY, "Waiting...", 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_LEFT);
    mainSizer->Add(fileText_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    // Size info
    sizeText_ = new wxStaticText(this, wxID_ANY, "", 
                                 wxDefaultPosition, wxDefaultSize,
                                 wxALIGN_LEFT);
    mainSizer->Add(sizeText_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // Cancel button
    cancelBtn_ = new wxButton(this, wxID_CANCEL, "Cancel");
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer(1);
    btnSizer->Add(cancelBtn_, 0, wxALL, 10);
    mainSizer->Add(btnSizer, 0, wxEXPAND);
    
    SetSizer(mainSizer);
    Centre();
    
    // Show immediately
    Show();
    wxYield();
}

ProgressDialog::~ProgressDialog() {
    completed_ = true;
}

void ProgressDialog::updateProgress(int value) {
    if (completed_ || cancelled_) return;
    
    wxCommandEvent* evt = new wxCommandEvent(wxEVT_THREAD_UPDATE, ID_PROGRESS_UPDATE);
    evt->SetInt(value);
    wxQueueEvent(this, evt);
}

void ProgressDialog::updateMessage(const wxString& message) {
    if (completed_ || cancelled_) return;
    
    wxCommandEvent* evt = new wxCommandEvent(wxEVT_THREAD_UPDATE, ID_MESSAGE_UPDATE);
    evt->SetString(message);
    wxQueueEvent(this, evt);
}

void ProgressDialog::updateFileInfo(const wxString& filename, int64_t downloadedBytes, int64_t totalBytes) {
    if (completed_ || cancelled_) return;
    
    wxString info = filename + "|" + 
                    formatBytes(downloadedBytes) + "|" + 
                    formatBytes(totalBytes) + "|" +
                    wxString::Format("%lld", downloadedBytes) + "|" +
                    wxString::Format("%lld", totalBytes);
    
    wxCommandEvent* evt = new wxCommandEvent(wxEVT_THREAD_UPDATE, ID_FILEINFO_UPDATE);
    evt->SetString(info);
    wxQueueEvent(this, evt);
}

void ProgressDialog::finish() {
    completed_ = true;
    wxCommandEvent* evt = new wxCommandEvent(wxEVT_THREAD_UPDATE, ID_FINISH);
    wxQueueEvent(this, evt);
}

void ProgressDialog::onThreadUpdate(wxCommandEvent& event) {
    switch (event.GetId()) {
        case ID_PROGRESS_UPDATE:
            gauge_->SetValue(event.GetInt());
            break;
        case ID_MESSAGE_UPDATE:
            messageText_->SetLabel(event.GetString());
            break;
        case ID_FILEINFO_UPDATE:
            onUpdateFileInfoInternal(event.GetString());
            break;
        case ID_FINISH:
            if (onComplete_) {
                onComplete_();
            }
            EndModal(wxID_OK);
            break;
    }
}

void ProgressDialog::onUpdateFileInfoInternal(const wxString& info) {
    wxArrayString parts = wxSplit(info, '|');
    
    if (parts.size() >= 5) {
        wxString filename = parts[0];
        wxString downloaded = parts[1];
        wxString total = parts[2];
        
        // Truncate filename if too long
        if (filename.length() > 50) {
            filename = "..." + filename.substr(filename.length() - 47);
        }
        
        fileText_->SetLabel("File: " + filename);
        
        // Calculate percentage and remaining
        long long dl, tl;
        parts[3].ToLongLong(&dl);
        parts[4].ToLongLong(&tl);
        
        if (tl > 0) {
            int percent = static_cast<int>((dl * 100) / tl);
            if (percent > 100) percent = 100;
            int64_t remaining = tl - dl;
            if (remaining < 0) remaining = 0;
            
            wxString sizeStr = wxString::Format("%s / %s (%d%%) - %s remaining",
                                                  downloaded, total, percent,
                                                  formatBytes(remaining));
            sizeText_->SetLabel(sizeStr);
            
            // Update gauge to match file progress
            gauge_->SetValue(percent);
        } else {
            sizeText_->SetLabel(downloaded + " / " + total);
            gauge_->Pulse();
        }
    }
}

void ProgressDialog::onClose(wxCloseEvent& event) {
    cancelled_ = true;
    completed_ = true;
    EndModal(wxID_CANCEL);
}

void ProgressDialog::onCancel(wxCommandEvent& event) {
    cancelled_ = true;
    messageText_->SetLabel("Cancelling...");
    cancelBtn_->Disable();
    // Don't close yet - wait for download thread to finish
    // The caller will check isCancelled() and handle cleanup
}

wxString ProgressDialog::formatBytes(int64_t bytes) {
    if (bytes < 0) bytes = 0;
    
    if (bytes < 1024) {
        return wxString::Format("%lld B", bytes);
    } else if (bytes < 1024 * 1024) {
        return wxString::Format("%.1f KiB", bytes / 1024.0);
    } else if (bytes < 1024LL * 1024LL * 1024LL) {
        return wxString::Format("%.1f MiB", bytes / (1024.0 * 1024.0));
    } else {
        return wxString::Format("%.2f GiB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}
