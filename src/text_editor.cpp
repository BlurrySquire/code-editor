#include "text_editor.hpp"

#include "settings.hpp"

TextEditor::TextEditor(wxWindow* parent, wxWindowID id)
    : wxStyledTextCtrl(parent, id)
{
    this->file_path = "";

    this->SetWrapMode(settings::get()["editor"]["word_wrap"].as_boolean() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
    this->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);

    wxFont code_font(11, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    code_font.SetFaceName(settings::get()["editor"]["font"]["family"].value_or(""));

    this->StyleSetFont(wxSTC_STYLE_DEFAULT, code_font);
    this->StyleClearAll();

    // Some weird thing with c++ and libraries needing classes over f***ing
    // integers, impl::wrap_node<long> means I can't use as_integer here.
    this->SetTabWidth(settings::get()["editor"]["indent"]["width"].value_or(0));
    this->SetIndent(settings::get()["editor"]["indent"]["width"].value_or(0));
    this->SetUseTabs(settings::get()["editor"]["indent"]["use_tabs"].as_boolean());
    this->SetBackSpaceUnIndents(true);
    this->SetIndentationGuides(settings::get()["editor"]["indent"]["show_guides"].as_boolean() ? wxSTC_IV_REAL : wxSTC_IV_NONE);

    this->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    this->SetMarginCursor(0, wxSTC_CURSORARROW);

    for (int i = 1; i < 2; i++) {
        this->SetMarginWidth(i, 0);
        this->SetMarginCursor(i, wxSTC_CURSORARROW);
    }

    this->UpdateLineNumberMargin(0);

    Bind(wxEVT_STC_UPDATEUI, &TextEditor::OnUpdateUI, this, this->GetId());
    Bind(wxEVT_STC_CHARADDED, &TextEditor::OnCharAdded, this, this->GetId());
    Bind(wxEVT_KEY_DOWN, &TextEditor::OnKeyDown, this, this->GetId());
}

TextEditor::~TextEditor() {

}

void TextEditor::OnUpdateUI(wxStyledTextEvent& event) {
    int line_count = this->GetLineCount();
    this->UpdateLineNumberMargin(line_count);
}

void TextEditor::OnCharAdded(wxStyledTextEvent& event) {
    char character = event.GetKey();
    int current_pos = this->GetCurrentPos();
    int current_line = this->LineFromPosition(current_pos);

    if (character == '\n') {
        this->HandleNewLines(current_line, current_pos);
    }

    static char opening[] = { '{', '[', '(', '"', '\'' };
    static char closing[] = { '}', ']', ')', '"', '\'' };

    bool just_opened = false;

    for (int i = 0; i < sizeof(opening); i++) {
        char current_char = (current_pos < this->GetTextLength()) ? this->GetCharAt(current_pos) : '\0';
        
        if (character == opening[i] && current_char != closing[i]) {
            this->InsertText(current_pos, closing[i]);
            just_opened = true;
            break;
        }
    }

    if (!just_opened) {
        for (int i = 0; i < sizeof(closing); i++) {
            char current_char = (current_pos < this->GetTextLength()) ? this->GetCharAt(current_pos) : '\0';

            if (character == closing[i] && this->GetCharAt(current_pos) == character) {
                this->DeleteRange(current_pos - 1, 1);
                this->CharRight();
                break;
            }
        }
    }
}

void TextEditor::OnKeyDown(wxKeyEvent& event) {
    if (event.GetKeyCode() == WXK_BACK) {
        static char opening[] = { '{', '[', '(', '"', '\'' };
        static char closing[] = { '}', ']', ')', '"', '\'' };

        int current_pos = this->GetCurrentPos();

        char left_char = this->GetCharAt(current_pos -1);
        char right_char = this->GetCharAt(current_pos);

        for (int i = 0; i < sizeof(opening); i++) {
            if (left_char == opening[i] && right_char == closing[i]) {
                this->DeleteRange(current_pos, 1);
                break;
            }
        }
    }

    event.Skip();
}

void TextEditor::UpdateLineNumberMargin(int line_count) {
    int digits = 1;
    while (line_count >= 10) {
        line_count /= 10;
        digits++;
    }

    digits = digits > 4 ? digits : 4;

    wxString margin_template = "_";
    for (int i = 0; i < digits; i++) {
        margin_template += "9";
    }

    int current_width = this->GetMarginWidth(0);
    int target_width = this->TextWidth(wxSTC_STYLE_LINENUMBER, margin_template);

    if (current_width != target_width) {
        this->SetMarginWidth(0, target_width);
    }
}

void TextEditor::HandleNewLines(int current_line, int current_pos) {
    char add_indent_chars[] = { '{', '[', '(' };
    
    if (current_line > 0) {
        int previous_line_indent = this->GetLineIndentation(current_line - 1);
        int indent_size = this->GetIndent();

        wxString previous_line = this->GetLine(current_line - 1);
        previous_line.Trim(true);

        int target_indent = previous_line_indent;

        bool open_bracket = false;

        if (!previous_line.empty()) {
            char last_char = previous_line.GetChar(previous_line.length() - 1);

            for (int i = 0; i < sizeof(add_indent_chars); i++) {
                if (last_char == add_indent_chars[i]) {
                    target_indent += indent_size;
                    open_bracket = true;
                    break;
                }
            }
        }

        this->SetLineIndentation(current_line, target_indent);

        if (open_bracket) {
            int eol_mode = this->GetEOLMode();
            wxString eol = (eol_mode == wxSTC_EOL_CRLF) ? "\r\n" : (eol_mode == wxSTC_EOL_CR) ? "\r" : "\n";

            int pos = this->GetLineIndentPosition(current_line);
            this->InsertText(pos, eol);

            this->SetLineIndentation(current_line + 1, previous_line_indent);
            this->SetLineIndentation(current_line, target_indent);
        }

        int pos = this->GetLineIndentPosition(current_line);
        this->GotoPos(pos);
    }
}

bool TextEditor::LoadFile(const wxString& filename) {
    this->file_path = filename;
    return wxStyledTextCtrl::LoadFile(filename);
}

void TextEditor::Save() {
    if (this->file_path.empty()) {
        this->SaveAs();
    }
    else {
        this->SaveFile(this->file_path);
    }
}

void TextEditor::SaveAs() {
    wxFileDialog save_dialog(this, "Save File As", "", "", "All Files, (*.*|*.*)", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (save_dialog.ShowModal() == wxID_CANCEL) return;

    this->SaveFile(save_dialog.GetPath());
}