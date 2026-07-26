#ifndef HPP_CODEEDITOR_FILE_VIEW
#define HPP_CODEEDITOR_FILE_VIEW

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/timer.h>

class FileViewItemData : public wxTreeItemData {
public:
    wxString full_path;
    bool is_directory;

    FileViewItemData(const wxString& full_path, bool is_directory) : full_path(full_path), is_directory(is_directory){}
};

wxDECLARE_EVENT(FILEVIEW_FILE_ACTIVATED, wxCommandEvent);
wxDECLARE_EVENT(FILEVIEW_PATH_DELETED, wxCommandEvent);
wxDECLARE_EVENT(FILEVIEW_PATH_MOVED, wxCommandEvent);

class FileView : public wxTreeCtrl {
private:
    wxImageList* icon_list;
    bool show_hidden_files;
    wxString root_path;
    wxTreeItemId context_item;
    wxTimer refresh_timer;
    wxTreeItemId drag_item;

    void AddFolderItems(const wxTreeItemId& parent_id, const wxString& path);
    void OnItemActivated(wxTreeEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnRefreshTimer(wxTimerEvent& event);
    void OnNewFile(wxCommandEvent& event);
    void OnNewFolder(wxCommandEvent& event);
    void OnDeleteItem(wxCommandEvent& event);
    void OnBeginDrag(wxTreeEvent& event);
    void OnEndDrag(wxTreeEvent& event);
    wxString GetTargetDirectory();
    bool IsDescendantPath(const wxString& parent_path, const wxString& candidate_path);

    void CollectExpandedPaths(const wxTreeItemId& item, wxArrayString& out);
    void RestoreTreeState(const wxTreeItemId& item, const wxArrayString& expanded_paths, const wxString& selected_path);

public:
    FileView(wxWindow* parent, wxWindowID id);
    virtual ~FileView();

    void PopulateTree(const wxString& root_path);
    void RefreshTree();

    void ShowHiddenFiles(bool toggle);
};

#endif
