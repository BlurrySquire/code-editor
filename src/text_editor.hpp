#ifndef HPP_TEXT_EDITOR
#define HPP_TEXT_EDITOR

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stc/stc.h>

class TextEditor : public wxStyledTextCtrl {
private:
    wxString file_path;

    void OnUpdateUI(wxStyledTextEvent& event);
    void OnCharAdded(wxStyledTextEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    void UpdateLineNumberMargin(int line_count);
    void HandleNewLines(int current_line, int current_pos);

public:
    TextEditor(wxWindow* parent, wxWindowID id);
    virtual ~TextEditor();

    bool LoadFile(const wxString& filename);
    void Save();
    void SaveAs();
};

#endif