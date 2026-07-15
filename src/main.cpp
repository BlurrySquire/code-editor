#include <stdio.h>
#include <stdint.h>

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/cmdline.h>
#include <wx/splitter.h>

enum {
    ID_FILEMENU_NEW_WINDOW = 1,
    ID_FILEMENU_OPEN_FOLDER = 2
};

#include "settings.hpp"
#include "file_view.hpp"
#include "editor_tabs.hpp"

class MainFrame : public wxFrame {
public:
    MainFrame(wxString startup_path)
        : wxFrame(nullptr, wxID_ANY, "Hello, world!", wxDefaultPosition, wxSize(1280, 720))
    {
        wxMenu* menu_file = new wxMenu;
        menu_file->Append(wxID_NEW, "&New\tCtrl+N");
        menu_file->Append(ID_FILEMENU_NEW_WINDOW, "&New Window\tCtrl+Shift+N");
        menu_file->AppendSeparator();
        menu_file->Append(wxID_OPEN, "&Open File...\tCtrl+O");
        menu_file->Append(ID_FILEMENU_OPEN_FOLDER, "&Open Folder...\tCtrl+K");
        menu_file->AppendSeparator();
        menu_file->Append(wxID_SAVE, "&Save\tCtrl+S");
        menu_file->Append(wxID_SAVEAS, "Save As...\tCtrl+Shift+S");
        menu_file->AppendSeparator();
        menu_file->Append(wxID_EXIT);

        wxMenu* menu_help = new wxMenu;
        menu_help->Append(wxID_ABOUT);

        wxMenuBar* menu_bar = new wxMenuBar;
        menu_bar->Append(menu_file, "&File");
        menu_bar->Append(menu_help, "&Help");
        this->SetMenuBar(menu_bar);

        this->SetTitle("Code Editor");

        this->splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

        this->file_view = new FileView(this->splitter, wxID_ANY);
        if (!startup_path.empty()) {
            this->file_view->PopulateTree(startup_path);
        }

        this->editor_tabs = new EditorTabs(this->splitter, wxID_ANY);
        
        this->splitter->SplitVertically(this->file_view, this->editor_tabs, 200);
        this->splitter->SetMinimumPaneSize(50);

        Bind(wxEVT_MENU, &MainFrame::OnNewFile, this, wxID_NEW);
        Bind(wxEVT_MENU, &MainFrame::OnNewWindow, this, ID_FILEMENU_NEW_WINDOW);
        Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, wxID_OPEN);
        Bind(wxEVT_MENU, &MainFrame::OnOpenFolder, this, ID_FILEMENU_OPEN_FOLDER);
        Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
        Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, wxID_SAVEAS);
        Bind(wxEVT_MENU, &MainFrame::OnExitBtn, this, wxID_EXIT);

        Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);

        Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
    }

private:
    wxSplitterWindow* splitter;
    wxString workspace_path;
    FileView* file_view;
    EditorTabs* editor_tabs;

    void OnNewFile(wxCommandEvent& event) {
        this->editor_tabs->NewFile();
    }

    void OnNewWindow(wxCommandEvent& event) {
        wxString executable = wxStandardPaths::Get().GetExecutablePath();
        wxExecute(executable, wxEXEC_ASYNC);
    }

    void OnOpenFile(wxCommandEvent& event) {
        wxFileDialog open_dialog(this, "Open File", "", "", "All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (open_dialog.ShowModal() == wxID_CANCEL) return;
        
        this->editor_tabs->OpenFile(open_dialog.GetPath());
    }

    void OnOpenFolder(wxCommandEvent& event) {
        wxDirDialog dir_dialog(this, "Select Workspace Folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

        if (dir_dialog.ShowModal() == wxID_CANCEL) return;

        this->file_view->PopulateTree(dir_dialog.GetPath());
    }

    void OnSave(wxCommandEvent& event) {
        this->editor_tabs->SaveCurrentFile();
    }

    void OnSaveAs(wxCommandEvent& event) {
        this->editor_tabs->SaveCurrentFileAs();
    }

    void OnExitBtn(wxCommandEvent& event) {
        Close();
    }
    
    void OnAbout(wxCommandEvent& event) {
        wxMessageBox("Made with wxWidgets.", "About Code Editor", wxOK | wxICON_INFORMATION);
    }

    void OnClose(wxCloseEvent& event) {
        if (this->editor_tabs->CloseAllTabs()) {
            this->Destroy();
        }
        else {
            event.Veto();
        }
    }
};

class CodeEditor : public wxApp {
private:
    wxString startup_path;

public:

    void OnInitCmdLine(wxCmdLineParser& parser) override {
        parser.AddParam("File or Folder Path", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

        wxApp::OnInitCmdLine(parser);
    }

    bool OnCmdLineParsed(wxCmdLineParser& parser) override {
        if (parser.GetParamCount() > 0) {
            this->startup_path = parser.GetParam(0);
        }

        return wxApp::OnCmdLineParsed(parser);
    }

    bool OnInit() override {
        if (!wxApp::OnInit()) return false;

        settings::load();

        MainFrame* frame = new MainFrame(this->startup_path);
        frame->Show();
        
        return true;
    }
};

wxIMPLEMENT_APP(CodeEditor);