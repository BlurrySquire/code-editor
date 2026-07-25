#include "file_view.hpp"
#include "config.hpp"

#include <wx/artprov.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/textdlg.h>
#include <wx/file.h>

enum {
    ID_FILEVIEW_NEW_FILE = wxID_HIGHEST + 100,
    ID_FILEVIEW_NEW_FOLDER,
    ID_FILEVIEW_DELETE
};

FileView::FileView(wxWindow* parent, wxWindowID id)
    : wxTreeCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT) , refresh_timer(this)
{
    this->icon_list = new wxImageList(16, 16, true);
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)));
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));

    this->AssignImageList(this->icon_list);
    Bind(wxEVT_TREE_ITEM_ACTIVATED, &FileView::OnItemActivated, this, this->GetId());
    Bind(wxEVT_CONTEXT_MENU, &FileView::OnContextMenu, this);
    Bind(wxEVT_TIMER, &FileView::OnRefreshTimer, this, this->refresh_timer.GetId());
    Bind(wxEVT_MENU, &FileView::OnNewFile, this, ID_FILEVIEW_NEW_FILE);
    Bind(wxEVT_MENU, &FileView::OnNewFolder, this, ID_FILEVIEW_NEW_FOLDER);
    Bind(wxEVT_MENU, &FileView::OnDeleteItem, this, ID_FILEVIEW_DELETE);

    this->refresh_timer.Start(1500);
}

FileView::~FileView() {
    this->refresh_timer.Stop();
}

void FileView::PopulateTree(const wxString& root_path) {
    this->root_path = root_path;
    this->context_item = wxTreeItemId();
    this->DeleteAllItems();

    wxTreeItemId root = this->AddRoot("Root");

    this->AddFolderItems(root, root_path);
}

wxDEFINE_EVENT(FILEVIEW_FILE_ACTIVATED, wxCommandEvent);
wxDEFINE_EVENT(FILEVIEW_PATH_DELETED, wxCommandEvent);

void FileView::AddFolderItems(const wxTreeItemId& parent_id, const wxString& path) {
    wxDir dir(path);
    if (!dir.IsOpened()) {
        wxMessageBox(FAILEDTO_OPENFOLDER, "Error", wxOK | wxICON_ERROR);
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

void FileView::OnContextMenu(wxContextMenuEvent& event) {
    wxPoint screen_pos = event.GetPosition();
    wxPoint client_pos = (screen_pos == wxDefaultPosition) ? wxPoint(0, 0) : this->ScreenToClient(screen_pos);

    int flags;
    wxTreeItemId item = this->HitTest(client_pos, flags);

    this->context_item = item;

    if (item.IsOk()) {
        this->SelectItem(item);
    }

    wxMenu menu;
    menu.Append(ID_FILEVIEW_NEW_FILE, NEWFILE);
    menu.Append(ID_FILEVIEW_NEW_FOLDER, NEWFOLDER);

    if (item.IsOk()) {
        FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(item));

        if (data != nullptr) {
            menu.AppendSeparator();
            menu.Append(ID_FILEVIEW_DELETE, data->is_directory ? DELETEFOLDER : DELETEFILE);

        }
    }

    this->PopupMenu(&menu, client_pos);
}

wxString FileView::GetTargetDirectory() {
    if (this->context_item.IsOk() ) {
        FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData (this->context_item));

        if (data != nullptr) {
            return data->is_directory ? data->full_path : wxFileName(data->full_path).GetPath();
        }
    }

    return this->root_path;
}

void FileView::OnNewFile(wxCommandEvent& event) {
    wxString dir = this->GetTargetDirectory();
    if (dir.empty()) return;

    //TODO:
    // shouldnt create a window/popup, instead a new tab in
    // sidebar(in right position) as input box for the name
    wxTextEntryDialog dialog(this, DIALOG_INPUT_NEWFILE, NEWFILE);

    if (dialog.ShowModal() == wxID_CANCEL) return;

    wxString name = dialog.GetValue();
    name.Trim(true).Trim(false);
    if (name.empty()) return;

    wxString full_path = dir + wxFileName::GetPathSeparator() + name;
    wxFile file;

    if (wxFileExists(full_path) || wxDirExists(full_path)) {
        wxMessageBox(FAILEDTO_ALREADYEXISTS, "Error", wxOK | wxICON_ERROR);
        return;
    }
    if (!file.Create(full_path)) {
        wxMessageBox(FAILEDTO_CREATEFILE, "Error", wxOK | wxICON_ERROR);
        return;
    }

    this->RefreshTree();

    wxCommandEvent activated_event(FILEVIEW_FILE_ACTIVATED, this->GetId());
    activated_event.SetEventObject(this);
    activated_event.SetString(full_path);
    this->ProcessWindowEvent(activated_event);
}

void FileView::OnNewFolder(wxCommandEvent& event) {
    wxString dir = this->GetTargetDirectory();
    if (dir.empty()) return;

    wxTextEntryDialog dialog(this, DIALOG_INPUT_NEWFOLDER, NEWFOLDER);

    if (dialog.ShowModal() == wxID_CANCEL) return;

    wxString name = dialog.GetValue();
    name.Trim(true).Trim(false);
    if (name.empty()) return;

    wxString full_path = dir + wxFileName::GetPathSeparator() + name;

    if (wxFileExists(full_path) || wxDirExists(full_path)) {
        wxMessageBox(FAILEDTO_ALREADYEXISTS, "Error", wxOK | wxICON_ERROR);
        return;
    }
    if (!wxFileName::Mkdir(full_path)) {
        wxMessageBox(FAILEDTO_CREATEFOLDER, "Error", wxOK | wxICON_ERROR);
        return;
    }

    this->RefreshTree();
}

void FileView::OnDeleteItem(wxCommandEvent& event) {
    if (!this->context_item.IsOk()) return;

    FileViewItemData* data = static_cast<FileViewItemData*>(this->GetItemData(this->context_item));
    if (data == nullptr) return;

    wxString item_type = data->is_directory ? FOLDER : FILE;

    wxMessageDialog confirm_dialog(
        this,
        DIALOG_CONFIRM_DELETION " " + item_type + "?\n\n" + data->full_path,
        DIALOG_DELETE_TEXT " " + item_type,
        wxYES_NO | wxICON_WARNING
    );
    confirm_dialog.SetYesNoLabels(YN_LABEL_DELETE, YN_LABEL_CANCEL);

    if (confirm_dialog.ShowModal() != wxID_YES) return;

    bool success = data->is_directory
        ? wxFileName::Rmdir(data->full_path, wxPATH_RMDIR_RECURSIVE)
        : wxRemoveFile(data->full_path);

    if (!success) {
        wxMessageBox("Failed to delete the " + item_type + ".", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxCommandEvent deleted_event(FILEVIEW_PATH_DELETED, this->GetId());
    deleted_event.SetEventObject(this);
    deleted_event.SetString(data->full_path);
    deleted_event.SetInt(data->is_directory ? 1 : 0);
    this->ProcessWindowEvent(deleted_event);

    this->context_item = wxTreeItemId();
    this->RefreshTree();
}

void FileView::OnRefreshTimer(wxTimerEvent& event) {
    this->RefreshTree();
}

void FileView::CollectExpandedPaths(const wxTreeItemId& item, wxArrayString& out) {
    wxTreeItemIdValue cookie;
    wxTreeItemId child = this->GetFirstChild(item, cookie);

    while (child.IsOk()) {
        FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(child));

        if (data != nullptr && data->is_directory && this->IsExpanded(child)) {
            out.Add(data->full_path);
        }
        this->CollectExpandedPaths(child, out);

        child = this->GetNextChild(item, cookie);
    }
}

void FileView::RestoreTreeState(
    const wxTreeItemId& item,
    const wxArrayString& expanded_paths,
    const wxString& selected_path
) {
    wxTreeItemIdValue cookie;
    wxTreeItemId child = this->GetFirstChild(item, cookie);

    while (child.IsOk()) {
        FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(child));

        if (data != nullptr) {
            if (data->is_directory && expanded_paths.Index (data->full_path) != wxNOT_FOUND) {
                this->Expand(child);
            }
            if (!selected_path.empty() && data->full_path == selected_path) {
                this->SelectItem(child);
            }

        }

        this->RestoreTreeState(child, expanded_paths, selected_path);

        child = this->GetNextChild(item, cookie);
    }
}

void FileView::RefreshTree() {
    if (this->root_path.empty()) return;
    if (this->context_item.IsOk() && this->GetEditControl() != nullptr) return;

    wxArrayString expanded_paths;
    wxString selected_path;

    wxTreeItemId root = this->GetRootItem();

    if (root.IsOk()) {
        this->CollectExpandedPaths(root, expanded_paths);
    }

    wxTreeItemId selected = this->GetSelection();

    if (selected.IsOk()) {
        FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(selected));

        if (data != nullptr) {
            selected_path = data->full_path;
        }
    }

    wxString path_to_load = this->root_path;

    this->DeleteAllItems();

    wxTreeItemId new_root = this->AddRoot("Root");

    this->AddFolderItems(new_root, path_to_load);
    this->RestoreTreeState(new_root, expanded_paths, selected_path);
}
