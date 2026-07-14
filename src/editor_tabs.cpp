#include "editor_tabs.hpp"

#include "text_editor.hpp"

#define AUI_NB_STYLE \
    wxAUI_NB_DEFAULT_STYLE | \
    wxAUI_NB_CLOSE_ON_ALL_TABS

EditorTabs::EditorTabs(wxWindow* parent, wxWindowID id)
    : wxAuiNotebook(parent, id, wxDefaultPosition, wxDefaultSize, AUI_NB_STYLE)
{
    Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &EditorTabs::OnTabClose, this);
}

EditorTabs::~EditorTabs() {

}

void EditorTabs::OnTabClose(wxAuiNotebookEvent& event) {
    int index = event.GetSelection();
    
    wxWindow* window = this->GetPage(index);
    TextEditor* editor = static_cast<TextEditor*>(window);

    if (editor->GetModify()) {
        wxMessageDialog dialog(this, "Would you like to save your changes?", "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_QUESTION);

        dialog.SetYesNoCancelLabels("Save", "Don't Save", "Cancel");
        
        int response = dialog.ShowModal();
        
        if (response == wxID_CANCEL) {
            event.Veto();
            return;
        }
        else if (response == wxID_YES) {
            editor->Save();
        }
    }

    event.Skip();
}

void EditorTabs::NewFile() {
    TextEditor* text_editor = new TextEditor(this, wxID_ANY);

    this->AddPage(text_editor, "Untitled");
}

void EditorTabs::OpenFile(const wxFileName& filename) {
    TextEditor* text_editor = new TextEditor(this, wxID_ANY);
    text_editor->LoadFile(filename.GetAbsolutePath());

    this->AddPage(text_editor, filename.GetFullName());
}

void EditorTabs::SaveCurrentFile() {
    int index = this->GetSelection();

    wxWindow* window = this->GetPage(index);
    TextEditor* editor = static_cast<TextEditor*>(window);

    editor->Save();
}

void EditorTabs::SaveCurrentFileAs() {
    int index = this->GetSelection();

    wxWindow* window = this->GetPage(index);
    TextEditor* editor = static_cast<TextEditor*>(window);

    editor->SaveAs();
}
