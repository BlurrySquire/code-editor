#ifndef HPP_CODEEDITOR_FILE_VIEW
#define HPP_CODEEDITOR_FILE_VIEW

#include <wx/wx.h>
#include <wx/treectrl.h>

class FileViewItemData : public wxTreeItemData {
public:
    wxString full_path;
    bool is_directory;

    FileViewItemData(const wxString& full_path, bool is_directory) : full_path(full_path), is_directory(is_directory){}
};

wxDECLARE_EVENT(FILEVIEW_FILE_ACTIVATED, wxCommandEvent);
class FileView : public wxTreeCtrl {
private:
    wxImageList* icon_list;
    bool show_hidden_files;

    void AddFolderItems(const wxTreeItemId& parent_id, const wxString& path);
    void OnItemActivated(wxTreeEvent& event);

public:
    FileView(wxWindow* parent, wxWindowID id);
    virtual ~FileView();

    void PopulateTree(const wxString& root_path);

    void ShowHiddenFiles(bool toggle);
};

#endif
