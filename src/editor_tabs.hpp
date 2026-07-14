#ifndef HPP_EDITOR_TABS
#define HPP_EDITOR_TABS

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>

class EditorTabs : public wxAuiNotebook {
private:
    void OnTabClose(wxAuiNotebookEvent& event);

public:
    EditorTabs(wxWindow* parent, wxWindowID id);
    virtual ~EditorTabs();

    void NewFile();
    void OpenFile(const wxFileName& path);

    void SaveCurrentFile();
    void SaveCurrentFileAs();
};

#endif