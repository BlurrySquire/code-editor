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
    Bind(wxEVT_TREE_ITEM_ACTIVATED, &FileView::OnItemActivated, this, this->GetId());
}

FileView::~FileView() {

}

void FileView::PopulateTree(const wxString& root_path) {
    this->DeleteAllItems();

    wxTreeItemId root = this->AddRoot("Root");

    this->AddFolderItems(root, root_path);
}

wxDEFINE_EVENT(FILEVIEW_FILE_ACTIVATED, wxCommandEvent);

void FileView::AddFolderItems(const wxTreeItemId& parent_id, const wxString& path) {
    wxDir dir(path);
    if (!dir.IsOpened()) {
        wxMessageBox("Failed to open folder!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString filename;

    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
    while (cont) {
        wxString full_path = path + wxFileName::GetPathSeparator() + filename;
        wxTreeItemId item = AppendItem(parent_id, filename, 0);
        this->SetItemData(item, new FileViewItemData(full_path, true));
        this->AddFolderItems(item, full_path);

        cont = dir.GetNext(&filename);
    }

    cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont) {
        wxString full_path = path + wxFileName::GetPathSeparator() + filename;
        wxTreeItemId item = this->AppendItem(parent_id, filename, 1);
        this->SetItemData(item, new FileViewItemData(full_path, false));

        cont = dir.GetNext(&filename);
    }
}

void FileView::OnItemActivated(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();
    FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(item));

    if (data == nullptr) {
        event.Skip();
        return;
    }

    if (data->is_directory) {
        this->Toggle(item);
        return;
    }

    wxCommandEvent activated_event(FILEVIEW_FILE_ACTIVATED, this->GetId());
    activated_event.SetEventObject(this);
    activated_event.SetString(data->full_path);

    this->ProcessWindowEvent(activated_event);
}
