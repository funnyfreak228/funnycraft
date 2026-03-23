#include <wx/wx.h>
#include "MainFrame.h"

class FunnyCraftApp : public wxApp {
public:
    virtual bool OnInit() override;
};

wxIMPLEMENT_APP(FunnyCraftApp);

bool FunnyCraftApp::OnInit() {
    if (!wxApp::OnInit())
        return false;
    
    // Initialize wxWidgets
    wxInitAllImageHandlers();
    
    // Create main frame
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    
    return true;
}
