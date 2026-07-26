#include "editor_tabs.hpp"
#include "config.hpp"
#include "text_editor.hpp"

#include <wx/artprov.h>
#include <wx/image.h>

#define AUI_NB_STYLE \
    wxAUI_NB_DEFAULT_STYLE | \
    wxAUI_NB_CLOSE_ON_ALL_TABS

EditorTabs::EditorTabs(wxWindow* parent, wxWindowID id)
    : wxAuiNotebook(parent, id, wxDefaultPosition, wxDefaultSize, AUI_NB_STYLE)
{
    #ifdef __WXMSW__
        this->modified_icon = wxBITMAP_PNG(modified-icon);
    #else
        wxImage modified_icon_image("resources/modified-icon.png", wxBITMAP_TYPE_PNG );
        this->modified_icon = wxBitmap(modified_icon_image);
    #endif

    Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &EditorTabs::OnTabClose, this);

    Bind(wxEVT_STC_SAVEPOINTLEFT, &EditorTabs::OnTabModified, this);
    Bind(wxEVT_STC_SAVEPOINTREACHED, &EditorTabs::OnTabSaved, this);
}

EditorTabs::~EditorTabs() {

}

void EditorTabs::OnTabClose(wxAuiNotebookEvent& event) {
    int index = event.GetSelection();

    if (index != wxNOT_FOUND) {
        wxWindow* window = this->GetPage(index);
        TextEditor* editor = static_cast<TextEditor*>(window);

        if (editor->GetModify()) {
            wxMessageDialog dialog(this, "Would you like to save your changes to " + this->GetPageText(index), "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_QUESTION);

            dialog.SetYesNoCancelLabels(YN_LABEL_SAVE, YN_LABEL_NOTSAVE, YN_LABEL_CANCEL);

            int response = dialog.ShowModal();

            if (response == wxID_CANCEL) {
                event.Veto();
                return;
            }
            else if (response == wxID_YES) {
                editor->Save();
            }
        }
    }

    event.Skip();
}

void EditorTabs::OnTabModified(wxStyledTextEvent& event) {
    wxWindow* page = wxDynamicCast(event.GetEventObject(), wxWindow);
    int index = this->GetPageIndex(page);

    if (index != wxNOT_FOUND) {
        this->SetPageBitmap(index, this->modified_icon);
    }
}

void EditorTabs::OnTabSaved(wxStyledTextEvent& event) {
    wxWindow* page = wxDynamicCast(event.GetEventObject(), wxWindow);
    int index = this->GetPageIndex(page);

    if (index != wxNOT_FOUND) {
        this->SetPageBitmap(index, wxNullBitmap);
    }
}


void EditorTabs::NewFile() {
    TextEditor* text_editor = new TextEditor(this, wxID_ANY);

    this->AddPage(text_editor, "Untitled");
}

void EditorTabs::OpenFile(const wxFileName& filename) {
    wxString target_path = filename.GetAbsolutePath();

    for (size_t i = 0; i < this->GetPageCount(); i++) {
        TextEditor* editor = static_cast<TextEditor*> (this->GetPage(i));

        if (editor->GetFilePath() == target_path) {
            this->SetSelection(i);
            return;
        }
    }

    TextEditor* text_editor = new TextEditor(this, wxID_ANY);

    text_editor->LoadFile(target_path);

    size_t new_index = this->GetPageCount();
    this->AddPage(text_editor, filename.GetFullName());
    this->SetSelection(new_index);
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

void EditorTabs::CloseCurrentTab() {
    int index = this->GetSelection();

    if (index != wxNOT_FOUND) {
        wxWindow* window = this->GetPage(index);
        TextEditor* editor = static_cast<TextEditor*>(window);

        if (editor->GetModify()) {
            wxMessageDialog dialog(this, "Would you like to save your changes to " + this->GetPageText(index), "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_QUESTION);

            dialog.SetYesNoCancelLabels(YN_LABEL_SAVE, YN_LABEL_NOTSAVE, YN_LABEL_CANCEL);

            int response = dialog.ShowModal();

            if (response == wxID_CANCEL) {
                return;
            }
            else if (response == wxID_YES) {
                editor->Save();
            }
        }

        this->DeletePage(index);
    }
}

bool EditorTabs::CloseAllTabs() {
    for (int i = this->GetPageCount() - 1; i >= 0; i--) {
        wxWindow* window = this->GetPage(i);
        TextEditor* editor = static_cast<TextEditor*>(window);

        if (editor->GetModify()) {
            wxMessageDialog dialog(this, "Would you like to save your changes to " + this->GetPageText(i), "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_QUESTION);

            dialog.SetYesNoCancelLabels(YN_LABEL_SAVE, YN_LABEL_NOTSAVE, YN_LABEL_CANCEL);

            int response = dialog.ShowModal();

            if (response == wxID_CANCEL) {
                return false;
            }
            else if (response == wxID_YES) {
                editor->Save();
            }
        }

        this->DeletePage(i);
    }

    return true;
}

void EditorTabs::CloseTabByPath(const wxString& path) {
    for (size_t i = 0; i < this->GetPageCount(); i++) {
        TextEditor* editor = static_cast<TextEditor*> (this->GetPage(i));

        if (editor->GetFilePath() == path) {
            this->DeletePage(i);
            break;
        }
    }
}

void EditorTabs::CloseTabsInFolder(const wxString& folder_path) {
    wxString prefix = folder_path + wxFileName::GetPathSeparator();

    for (int i = (int)this->GetPageCount() - 1; i >= 0; i--) {
        TextEditor* editor = static_cast<TextEditor*> (this->GetPage(i));

        if (editor->GetFilePath().StartsWith(prefix)) {
            this->DeletePage(i);
        }
    }
}
