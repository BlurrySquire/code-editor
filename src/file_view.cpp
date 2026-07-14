#include "file_view.hpp"

#include <wx/artprov.h>
#include <wx/dir.h>
#include <wx/filename.h>

FileView::FileView(wxWindow* parent, wxWindowID id)
    : wxTreeCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT)
{
    this->icon_list = new wxImageList(16, 16, true);
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)));
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));

    this->AssignImageList(this->icon_list);
}

FileView::~FileView() {

}

void FileView::PopulateTree(const wxString& root_path) {
    this->DeleteAllItems();

    wxTreeItemId root = this->AddRoot("Root");

    this->AddFolderItems(root, root_path);
}

void FileView::AddFolderItems(const wxTreeItemId& parent_id, const wxString& path) {
    wxDir dir(path);
    if (!dir.IsOpened()) {
        wxMessageBox("Failed to open folder!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString filename;
    
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
    while (cont) {
        wxTreeItemId item = AppendItem(parent_id, filename, 0);
        this->AddFolderItems(item, path + wxFileName::GetPathSeparator() + filename);

        cont = dir.GetNext(&filename);
    }

    cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont) {
        this->AppendItem(parent_id, filename, 1);

        cont = dir.GetNext(&filename);
    }
}