#ifndef HPP_CODEEDITOR_FILE_VIEW
#define HPP_CODEEDITOR_FILE_VIEW

#include <wx/wx.h>
#include <wx/treectrl.h>

class FileView : public wxTreeCtrl {
private:
    wxImageList* icon_list;
    bool show_hidden_files;

    void AddFolderItems(const wxTreeItemId& parent_id, const wxString& path);

public:
    FileView(wxWindow* parent, wxWindowID id);
    virtual ~FileView();

    void PopulateTree(const wxString& root_path);

    void ShowHiddenFiles(bool toggle);
};

#endif