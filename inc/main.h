#ifndef __MAIN_H__
#define __MAIN_H__

const char*     MNP_APPNAME_C             = "MyNotePad";
const wchar_t*  MNP_APPNAME               = L"MyNotePad";
const char*     MNP_DOC_TITLE             = " - MyNotePad";
const char*     MNP_DOC_NOTITLE           = "Untitled";
const wchar_t*  MNP_COPYRIGHT             = L"\nCopyright(c) moooc.cc 2020";

int             MNP_PADDING_LEFT          = 16;    // space for line number
const int       MNP_LINENUM_MARGIN_LEFT   = 4;
const int       MNP_LINENUM_MARGIN_TOP    = 4;
const wchar_t*  MNP_LINENUM_FONTFACE      = L"Arial";
const int       MNP_SCROLLBAR_WIDTH       = 14;

const char*     MNP_CONFIG_FILE           = "MyNotePad.conf";
const char*     MNP_CONFIG_THEME          = "theme";
const char*     MNP_CONFIG_WORDWRAP       = "word-wrap";
const char*     MNP_CONFIG_LINENUMBER     = "line-number";
const char*     MNP_CONFIG_FONTNAME       = "font-name";

const wchar_t*  INFO_SAVE                 = L"Save changes?";
const wchar_t*  ERR_OPEN_FAIL             = L"Failed to save HTML file, \n"\
                                             "make sure you have permissions to write that file.";
const wchar_t*  ERR_FAULT_ENCODING        = L"Can only open UTF-8 files!";
const wchar_t*  ERR_UNKNOWN               = L"Unknown error";

#ifdef _WIN32
const wchar_t*  FONT_MSYAHEI              = L"Microsoft Yahei";
const wchar_t*  FONT_LUCIDA               = L"Lucida Console";
const wchar_t*  FONT_COURIER              = L"Courier New";
const wchar_t*  FONT_CONSOLAS             = L"Consolas";
#else
const wchar_t*  FONT_MSYAHEI              = L"Noto Sans Mono";
const wchar_t*  FONT_LUCIDA               = L"Liberation Mono";
const wchar_t*  FONT_COURIER              = L"Courier 10 Pitch";
const wchar_t*  FONT_CONSOLAS             = L"DejaVu Sans Mono";
#endif

struct CharStyle
{
    uint8_t bold : 1;
    uint8_t itatic : 1;
    uint8_t underline : 1;
    uint8_t reserved : 5;
};

const CharStyle default_style = { 0 };

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    
    void themeLight();
    void themeDark();
    void loadSettings();
    void saveSettings();
    void updateSaveState(bool saved);
    void loadFile(const std::string path);
    bool sureToQuit(wxCommandEvent& event);
    void saveHTML(const std::string pathname);
    std::wstring all_to_string();
    void ApplyTheme();
    void md2html(std::wstring& str);
    int widthOfLineNumber() const;
#ifndef _WIN32
    // this function is not exists on Linux
    int FromDIP(int n) {
        return n;
    }
#endif
protected:
    void OnLeftButtonDown(wxMouseEvent& event);
    void OnLeftButtonUp(wxMouseEvent& event);
    void OnLeftButtonDBClick(wxMouseEvent& event);
    void OnRightButtonDown(wxMouseEvent& event);
    void OnRightButtonUp(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnCopyHtml(wxCommandEvent& event);
    void OnExportHTML(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnSelAll(wxCommandEvent& event);
    void OnOptions(wxCommandEvent& event);
    void OnMenuOpen(wxMenuEvent& event);
    void OnThemeLight(wxCommandEvent& event);
    void OnThemeDark(wxCommandEvent& event);
    void OnLineNumber(wxCommandEvent& event);
    void OnFont(wxCommandEvent& event);
    void OnFontSelect(wxCommandEvent& event);
    void OnBrowser(wxCommandEvent& event);
    void OnWordWrap(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
private:
    class notepadCtrl : public wxStyledTextCtrl {
    public:
        notepadCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxString& name = wxSTCNameStr)
            : wxStyledTextCtrl(parent, id, pos, size, style, name) {};
#ifdef _WIN32
        WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
    };
    notepadCtrl *article;
    bool bShowLineNumber;       // display line number (initial true)
    bool bResized;              // flag indicates the app window is resized
    bool bSaved;                // set false after file is modified
    bool bWordWrap;             // break word (initial false)
    
    enum THEME {
        THEME_LIGHT,
        THEME_DARK
    };
    THEME theme;
    wxColour bgColorEdit;
    wxColour bgColorLight;
    wxColour bgColorSel;
    wxColour fontColor;
    wxColour linkColor;
    wxColour strongColor;
    wxColour quoteColor;
    wxColour lineNumFontColor;
    wxColour scrollBarBgColor;
    wxColour scrollBarColor;
    const wchar_t* fontFace;
    int fontSize;
    int lineNumFontSize;
    // configure file Path
    std::string confFilePath;
    std::string openedFile;
    DECLARE_EVENT_TABLE()
};

// Declare some IDs. These are arbitrary.
const int FILE_NEW            = wxID_NEW;
const int FILE_OPEN           = wxID_OPEN;
const int FILE_SAVE           = wxID_SAVE;
const int FILE_SAVEAS         = wxID_SAVEAS;
const int FILE_COPYHTML       = 2000;
const int FILE_EXPORTHTML     = wxID_PRINT;
const int FILE_QUIT           = wxID_EXIT;
const int EDIT_UNDO           = wxID_UNDO;
const int EDIT_REDO           = wxID_REDO;
const int EDIT_CUT            = wxID_CUT;
const int EDIT_COPY           = wxID_COPY;
const int EDIT_PASTE          = wxID_PASTE;
const int EDIT_DELETE         = wxID_DELETE;
const int EDIT_SELALL         = wxID_SELECTALL;
const int EDIT_OPTIONS        = wxID_PREFERENCES;
const int VIEW_THEMELIGHT     = 2001;
const int VIEW_THEMEDARK      = 2002;
const int VIEW_LINENUMBER     = 2003;
const int VIEW_FONT_MSYAHEI   = 2004;
const int VIEW_FONT_LUCIDA    = 2005;
const int VIEW_FONT_COURIER   = 2006;
const int VIEW_FONT_CONSOLAS  = 2007;
const int VIEW_FONT_SELECT    = wxID_SELECT_FONT;
const int FORMAT_BROWSER      = wxID_NETWORK;
const int FORMAT_WORDWRAP     = 2008;
const int HELP_ABOUT          = wxID_ABOUT;

class MyDropTarget : public wxFileDropTarget
{
    MyFrame *frame;
public:
    MyDropTarget(MyFrame *myFrame) : frame(myFrame) {}
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
};

#endif /* __MAIN_H__ */
