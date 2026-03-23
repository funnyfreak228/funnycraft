#pragma once

#include <wx/wx.h>
#include <wx/gauge.h>
#include <atomic>
#include <thread>
#include <functional>

// Custom event for thread updates
wxDECLARE_EVENT(wxEVT_THREAD_UPDATE, wxCommandEvent);

class ProgressDialog : public wxDialog {
public:
    ProgressDialog(wxWindow* parent, const wxString& title, int maxValue = 100);
    ~ProgressDialog();
    
    // Update progress (thread-safe)
    void updateProgress(int value);
    void updateMessage(const wxString& message);
    void updateFileInfo(const wxString& filename, int64_t downloadedBytes, int64_t totalBytes);
    
    // Check if cancelled
    bool isCancelled() const { return cancelled_; }
    
    // Set completion callback
    void setCompleteCallback(std::function<void()> callback) { onComplete_ = callback; }
    
    // Finish dialog
    void finish();
    
private:
    wxGauge* gauge_;
    wxStaticText* messageText_;
    wxStaticText* fileText_;
    wxStaticText* sizeText_;
    wxButton* cancelBtn_;
    
    std::atomic<bool> cancelled_;
    std::atomic<bool> completed_;
    std::function<void()> onComplete_;
    
    // Thread-safe updates via events
    void onThreadUpdate(wxCommandEvent& event);
    void onUpdateFileInfoInternal(const wxString& info);
    void onClose(wxCloseEvent& event);
    void onCancel(wxCommandEvent& event);
    
    // Format bytes to human readable
    static wxString formatBytes(int64_t bytes);
    
    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum {
    ID_PROGRESS_UPDATE = 4001,
    ID_MESSAGE_UPDATE,
    ID_FILEINFO_UPDATE,
    ID_FINISH
};
