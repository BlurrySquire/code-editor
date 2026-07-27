#include "file_view.hpp"
#include "config.hpp"
#include "settings.hpp"

#include <wx/artprov.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/textdlg.h>
#include <wx/file.h>
#include <wx/fontenum.h>

enum {
    ID_FILEVIEW_NEW_FILE = wxID_HIGHEST + 100,
    ID_FILEVIEW_NEW_FOLDER,
    ID_FILEVIEW_DELETE
};

FileView::FileView(wxWindow* parent, wxWindowID id)
    : wxTreeCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT) , refresh_timer(this)
{
    int icon_size = settings::get()["files"]["icons"]["size"].value_or(16);

    this->icon_list = new wxImageList(icon_size, icon_size, true);
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(icon_size, icon_size)));
    this->icon_list->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(icon_size, icon_size)));

    this->AssignImageList(this->icon_list);

    wxFont tree_font(
        settings::get()["files"]["font"]["size"].value_or(12),
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    );

    wxString configured_face = settings::get()["files"]["font"]["family"].value_or("");
    if (!configured_face.empty() && wxFontEnumerator::IsValidFacename(configured_face)) {
        tree_font.SetFaceName(configured_face);
    }

    this->SetFont(tree_font);

    Bind(wxEVT_TREE_ITEM_ACTIVATED, &FileView::OnItemActivated, this, this->GetId());
    Bind(wxEVT_CONTEXT_MENU, &FileView::OnContextMenu, this);
    Bind(wxEVT_TIMER, &FileView::OnRefreshTimer, this, this->refresh_timer.GetId());
    Bind(wxEVT_MENU, &FileView::OnNewFile, this, ID_FILEVIEW_NEW_FILE);
    Bind(wxEVT_MENU, &FileView::OnNewFolder, this, ID_FILEVIEW_NEW_FOLDER);
    Bind(wxEVT_MENU, &FileView::OnDeleteItem, this, ID_FILEVIEW_DELETE);
    Bind(wxEVT_TREE_BEGIN_DRAG, &FileView::OnBeginDrag, this, this->GetId());
    Bind(wxEVT_TREE_END_DRAG, &FileView::OnEndDrag, this, this->GetId());

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
wxDEFINE_EVENT(FILEVIEW_PATH_MOVED, wxCommandEvent);

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
    wxTreeItemId item = this->GetSelection();

    FileViewItemData* data = static_cast<FileViewItemData*>(this->GetItemData(item));
    if (data == nullptr) {
        wxMessageBox("Invalid item data.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString path = data->full_path;
    bool is_directory = data->is_directory;

    if (is_directory) {
        if (!wxDirExists(path)) {
            wxMessageBox(FAILEDTO_NOTEXIST_FOLDER "\n\n" + path, "Error", wxOK | wxICON_ERROR);
            this->RefreshTree();
            return;
        }
    } else {
        if (!wxFileExists(path)) {
            wxMessageBox(FAILEDTO_NOTEXIST_FILE "\n\n" + path, "Error", wxOK | wxICON_ERROR);
            this->RefreshTree();
            return;
        }
    }

    wxString item_type = is_directory ? LABEL_FOLDER : LABEL_FILE;


    wxMessageDialog confirm_dialog(
        this,
        DIALOG_CONFIRM_DELETION " " + item_type + "?\n\n" + data->full_path,
        DIALOG_DELETE_TEXT " " + item_type,
        wxYES_NO | wxICON_WARNING
    );
    confirm_dialog.SetYesNoLabels(YN_LABEL_DELETE, YN_LABEL_CANCEL);

    if (confirm_dialog.ShowModal() != wxID_YES) return;
    if (is_directory && !wxDirExists(path)) {
        wxMessageBox(FAILEDTO_ALREADYDELETED_FOLDER, "Error", wxOK | wxICON_ERROR);
        this->RefreshTree();
        return;
    }
    if (!is_directory && !wxFileExists(path)) {
        wxMessageBox(FAILEDTO_ALREADYDELETED_FILE, "Error", wxOK | wxICON_ERROR);
        this->RefreshTree();
        return;
    }

    bool success = is_directory
        ? wxFileName::Rmdir(path, wxPATH_RMDIR_RECURSIVE)
        : wxRemoveFile(path);

    if (!success) {
        wxMessageBox("Failed to delete the " + item_type + ".", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxCommandEvent deleted_event(FILEVIEW_PATH_DELETED, this->GetId());
    deleted_event.SetEventObject(this);
    deleted_event.SetString(path);
    deleted_event.SetInt(is_directory ? 1 : 0);
    this->ProcessWindowEvent(deleted_event);

    this->context_item = wxTreeItemId();
    this->RefreshTree();
}

bool FileView::IsDescendantPath(const wxString& parent_path, const wxString& candidate_path) {
    wxString prefix = parent_path + wxFileName::GetPathSeparator();
    return candidate_path == parent_path || candidate_path.StartsWith(prefix);
}

void FileView::OnBeginDrag(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();
    FileViewItemData* data = static_cast<FileViewItemData*> (this->GetItemData(item));

    if (data == nullptr) {
        event.Veto();
        return;
    }

    this->drag_item = item;
    event.Allow();
}

void FileView::OnEndDrag(wxTreeEvent& event) {
    wxTreeItemId target_item = event.GetItem();

    if (!this->drag_item.IsOk()) {
        event.Skip();
        return;
    }

    FileViewItemData* source_data = static_cast<FileViewItemData*> (this->GetItemData(this->drag_item));

    if (source_data == nullptr) {
        this->drag_item = wxTreeItemId();

        return;
    }

    wxString target_dir;
    wxString source_path = source_data->full_path;

    bool source_is_dir = source_data->is_directory;

    if (target_item.IsOk()) {
        FileViewItemData* target_data = static_cast<FileViewItemData*> (this->GetItemData(target_item));

        if (target_data != nullptr){
            target_dir =
                target_data->is_directory ?
                target_data->full_path : wxFileName(target_data->full_path).GetPath()
            ;
        } else
        {
            target_dir = this->root_path;
        }
    } else {
        target_dir = this->root_path;
    }

    this->drag_item = wxTreeItemId();

    if (target_dir.empty()) return;

    if (source_is_dir && this->IsDescendantPath(source_path, target_dir)) {
        wxMessageBox(FAILEDTO_MOVING_INTO_ITSELF_FOLDER, "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString filename = wxFileName(source_path).GetFullName();
    wxString new_path = target_dir + wxFileName::GetPathSeparator() + filename;

    if (wxFileExists(new_path) || wxDirExists(new_path)) {
        wxMessageBox(FAILEDTO_ALREADYEXISTS, "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (!wxRenameFile(source_path, new_path, false)){
        wxMessageBox("didnt work", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (new_path == source_path) return;

    this->context_item = wxTreeItemId();
    this->CallAfter([this, source_path, new_path]() {
        wxCommandEvent moved_event(FILEVIEW_PATH_MOVED, this->GetId());
        moved_event.SetEventObject(this);
        moved_event.SetString(source_path);
        moved_event.SetClientData(new wxString(new_path));
        this->ProcessWindowEvent(moved_event);
        this->RefreshTree();
    });
}

void FileView::OnRefreshTimer(wxTimerEvent& event) {
    if (this->drag_item.IsOk()) return;
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
