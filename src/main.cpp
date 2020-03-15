#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/dir.h>
#include <wx/dnd.h>
#include <wx/colour.h>
#include <wx/caret.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/filedlg.h>
#include <wx/clipbrd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <iomanip>

#include "config.h"
#include "main.h"
#include "lex_parse.h"
#if _WIN32
#include "resource.h"
#else
#include "mynotepad.xpm"
#endif

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


/* ===========================================
 * INPUT METHOD
 * ===========================================
 */
#include "imm.h"
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

WXLRESULT MyFrame::notepadCtrl::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    HWND hWnd = GetHWND();
    HIMC hImc = ImmGetContext(hWnd);
    COMPOSITIONFORM Composition;
    wxPoint p;
    switch (message) {
    case WM_IME_STARTCOMPOSITION:
        Composition.dwStyle = CFS_POINT;
        p = PointFromPosition(GetCurrentPos());
        Composition.ptCurrentPos.x = p.x;
        Composition.ptCurrentPos.y = p.y + 10;
        ImmSetCompositionWindow(hImc, &Composition);
        ImmReleaseContext(hWnd, hImc);
        return TRUE;
    }
    return wxStyledTextCtrl::MSWWindowProc(message, wParam, lParam);
}
#endif

#define LOG_MESSAGE(msg) do {wxLogDebug(L"%s: %s", __func__, msg);} while(0)
#define LOG_ERROR(msg) do {wxLogDebug(L"%s: %s", __func__, msg);} while(0)

/*
 * APPLICATION
 */
class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    /*
     * Mouse event
     */
    EVT_LEFT_DOWN(MyFrame::OnLeftButtonDown)
    EVT_LEFT_UP(MyFrame::OnLeftButtonUp)
    EVT_LEFT_DCLICK(MyFrame::OnLeftButtonDBClick)
    EVT_RIGHT_DOWN(MyFrame::OnRightButtonDown)
    EVT_RIGHT_UP(MyFrame::OnRightButtonUp)
    EVT_CHAR(MyFrame::OnChar)

    /*
     * Menu items
     */
    EVT_MENU(FILE_NEW, MyFrame::OnNew)
    EVT_MENU(FILE_OPEN, MyFrame::OnOpen)
    EVT_MENU(FILE_SAVE, MyFrame::OnSave)
    EVT_MENU(FILE_SAVEAS, MyFrame::OnSaveAs)
    EVT_MENU(FILE_COPYHTML, MyFrame::OnCopyHtml)
    EVT_MENU(FILE_EXPORTHTML, MyFrame::OnExportHTML)
    EVT_MENU(FILE_QUIT, MyFrame::OnQuit)

    EVT_MENU(EDIT_UNDO, MyFrame::OnUndo)
    EVT_MENU(EDIT_REDO, MyFrame::OnRedo)
    EVT_MENU(EDIT_CUT, MyFrame::OnCut)
    EVT_MENU(EDIT_COPY, MyFrame::OnCopy)
    EVT_MENU(EDIT_PASTE, MyFrame::OnPaste)
    EVT_MENU(EDIT_DELETE, MyFrame::OnDelete)
    EVT_MENU(EDIT_SELALL, MyFrame::OnSelAll)
    EVT_MENU(EDIT_OPTIONS, MyFrame::OnOptions)
    EVT_MENU_OPEN(MyFrame::OnMenuOpen)

    EVT_MENU(VIEW_THEMELIGHT, MyFrame::OnThemeLight)
    EVT_MENU(VIEW_THEMEDARK, MyFrame::OnThemeDark)
    EVT_MENU(VIEW_LINENUMBER, MyFrame::OnLineNumber)
    EVT_MENU(VIEW_FONT_MSYAHEI, MyFrame::OnFont)
    EVT_MENU(VIEW_FONT_LUCIDA, MyFrame::OnFont)
    EVT_MENU(VIEW_FONT_COURIER, MyFrame::OnFont)
    EVT_MENU(VIEW_FONT_CONSOLAS, MyFrame::OnFont)
    EVT_MENU(VIEW_FONT_SELECT, MyFrame::OnFontSelect)

    EVT_MENU(FORMAT_BROWSER, MyFrame::OnBrowser)
    EVT_MENU(FORMAT_WORDWRAP, MyFrame::OnWordWrap)

    EVT_MENU(HELP_ABOUT, MyFrame::OnAbout)

    EVT_CLOSE(MyFrame::OnClose)

END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame(MNP_APPNAME);
    frame->SetSize(400, 300, 800, 600);

    frame->loadSettings();

    // load the file specified in cmdLine
    if(argc == 2 && argv[1][0] != '\0')
        frame->loadFile(argv[1].ToStdString());

    frame->Show(true);
    return true;
}

/* ===========================================
 * CORE FUNCTIONS
 * ===========================================
 */
void MyFrame::themeLight()
{
    bgColorEdit = wxColour(0xEE, 0xEE, 0xEE);
    bgColorLight = wxColour(0xF7, 0xF7, 0xF7);
    bgColorSel = wxColour(0xCC, 0xCC, 0xCC);
    fontColor = wxColour(0x44, 0x44, 0x44);
    linkColor = wxColour(0x00, 0x63, 0xB1);
    strongColor = *wxBLACK;
    quoteColor = wxColour(0x77, 0x77, 0x77);

    lineNumFontColor = bgColorSel;
    scrollBarBgColor = wxColour(0xE5, 0xE5, 0xE5);
    scrollBarColor = wxColour(0xD1, 0xD1, 0xD1);

    theme = THEME::THEME_LIGHT;
    GetMenuBar()->Check(VIEW_THEMELIGHT, true);
    GetMenuBar()->Check(VIEW_THEMEDARK, false);
}

void MyFrame::themeDark()
{
    bgColorEdit = wxColour(0x1E, 0x1E, 0x1E);
    bgColorLight = wxColour(0x27, 0x27, 0x27);
    bgColorSel = wxColour(0x2C, 0x5C, 0x8B);
    fontColor = wxColour(0xDD, 0xDD, 0xDD);
    linkColor = wxColour(0x00, 0x63, 0xB1);
    strongColor = *wxWHITE;
    quoteColor = wxColour(0xCC, 0xCC, 0xCC);

    lineNumFontColor = bgColorSel;
    scrollBarBgColor = wxColour(0x3E, 0x3E, 0x3E);
    scrollBarColor = wxColour(0x68, 0x68, 0x68);

    theme = THEME::THEME_DARK;
    GetMenuBar()->Check(VIEW_THEMELIGHT, false);
    GetMenuBar()->Check(VIEW_THEMEDARK, true);
}

void MyFrame::loadSettings()
{
    themeLight();
    GetMenuBar()->Check(VIEW_FONT_MSYAHEI, true);

#ifdef _WIN32
    confFilePath = wxStandardPaths::Get().GetExecutablePath();
    while (confFilePath.back() != '\\')
        confFilePath.pop_back();
#else
    confFilePath = wxStandardPaths::Get().GetUserDataDir();
    confFilePath += '/';
#endif
    LOG_MESSAGE(confFilePath);

    // load user data
    std::ifstream in(confFilePath + MNP_CONFIG_FILE);
    char buff[1024];
    if (in.good())
    {
        while (in.getline(buff, 1024))
        {
            if (buff[0] == '#')
                continue;            // # this is comment
            const char* pos = strchr(buff, '=');      // eg. best = 999
            if (pos != NULL)
            {
                std::string key(buff, pos - buff);    // acquire key
                std::string val(pos + 1);             // acquire value
                if (val.back() == '\r') val.pop_back();    // remove '\r' at the end
                if (key == MNP_CONFIG_THEME && std::atoi(val.c_str()) == THEME::THEME_DARK)
                    themeDark();
                else if (key == MNP_CONFIG_WORDWRAP && val == "0")
                {
                    bWordWrap = false;
                    GetMenuBar()->Check(FORMAT_WORDWRAP, false);
                    article->SetWrapMode(wxSTC_WRAP_NONE);
                }
                else if (key == MNP_CONFIG_LINENUMBER && val == "0")
                {
                    bShowLineNumber = false;
                    GetMenuBar()->Check(VIEW_LINENUMBER, false);
                    article->SetMarginType(wxSTC_MARGINOPTION_NONE, wxSTC_MARGIN_SYMBOL);
                    article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, MNP_PADDING_LEFT / 2);
                }
                else if (key == MNP_CONFIG_FONTNAME)
                {
                    if (val == "Microsoft Yahei UI") {
                        fontFace = L"Microsoft Yahei UI";
                    }
                    else if (val == "Lucida Console") {
                        fontFace = L"Lucida Console";
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_LUCIDA, true);
                    }
                    else if (val == "Courier New") {
                        fontFace = L"Courier New";
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_COURIER, true);
                    }
                    else if (val == "Consolas") {
                        fontFace = L"Consolas";
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_CONSOLAS, true);
                    }
                    //else if (val == "Noto Mono") {
                    //    fontFace = L"Noto Mono";
                    //    GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                    //    GetMenuBar()->Check(VIEW_FONT_NOTOMONO, true);
                    //}
                    for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_MARKDOWN_CODEBK; i++)
                        article->StyleSetFaceName(i, fontFace);
                }
            }
        }
    }
    ApplyTheme();
}

void MyFrame::saveSettings()
{
    std::string pathname = confFilePath + MNP_CONFIG_FILE;
    std::ofstream out(pathname);
    if (!out.good())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    out << "# Edit this file to config MyNotePad\r\n";
    out << MNP_CONFIG_THEME << '=' << theme << "\r\n";
    out << MNP_CONFIG_WORDWRAP << '=' << bWordWrap << "\r\n";
    out << MNP_CONFIG_LINENUMBER << '=' << bShowLineNumber << "\r\n";
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    out << MNP_CONFIG_FONTNAME << '=' << cvt.to_bytes(fontFace);
    out.close();
    LOG_MESSAGE(pathname);
}

void MyFrame::updateSaveState(bool saved)
{
    bSaved = saved;
    if (bSaved)
        SetTitle(openedFile + MNP_DOC_TITLE);
    else
        SetTitle(openedFile + " *" + MNP_DOC_TITLE);
}

void MyFrame::loadFile(const std::string pathname)
{
    std::string path(pathname);
    // remove quote
    if (path[0] == '\"') {
        path = path.substr(1, path.size() - 1);
    }
    LOG_MESSAGE(path);
    // load file
    std::ifstream f(path);
    if (f.fail()) {
        std::wstringstream ss;
        ss << L"Failed to load \"" << path << L"\".";
        wxMessageBox(ss.str(), MNP_APPNAME, wxOK | wxICON_WARNING, this);
        return;
    }
    // UTF-8 decode
    std::istreambuf_iterator<char> begin(f), end;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    std::wstring str;
    try {
        std::string bytes(begin, end);
        str = cvt.from_bytes(bytes);
    }
    catch (std::range_error re) {
        wxMessageBox(ERR_FAULT_ENCODING, MNP_APPNAME, wxOK | wxICON_WARNING, this);
        return;
    }

    openedFile = path;
    updateSaveState(true);
    article->SetValue(str);
}

/*
 * return true if the user decide to quit
 */
bool MyFrame::sureToQuit(wxCommandEvent& event)
{
    if (!bSaved)
    {
        int r = wxMessageBox(INFO_SAVE, MNP_APPNAME, wxYES_NO | wxCANCEL | wxICON_WARNING, this);
        if (r == wxCANCEL)
            return false;
        else if (r == wxYES)
        {
            OnSave(event);
            return true;
        }
    }
    return true;
}

void MyFrame::saveHTML(const std::string pathname)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    std::ofstream f(pathname);
    if (!f.good())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }

    f << "<!DOCTYPE><html><head><meta charset=\"utf-8\"/>";
#ifdef WIN32
    f << R"raw(
<link href="style.css" rel="stylesheet">
<link href="highlight.css" rel="stylesheet">
<script type="text/javascript" src="highlight.pack.js"></script>
)raw";
#else
    f << "<link href='" << MNP_INSTALL_PATH << "style.css' rel='stylesheet'>\n";
    f << "<link href='" << MNP_INSTALL_PATH << "highlight.css' rel='stylesheet'>\n";
    f << "<script type='text/javascript' src='" << MNP_INSTALL_PATH << "highlight.pack.js'></script>\n";
#endif
    f << R"raw(<script type="text/javascript">
    window.onload = function() {
        var aCodes = document.getElementsByTagName('pre');
        for (var i=0; i < aCodes.length; i++) {
            hljs.highlightBlock(aCodes[i]);
        }
    };
    // hljs.initHighlightingOnLoad();
</script>
</head><body><main>)raw";
    std::wstring str = all_to_string();
    md2html(str);

    f << cvt.to_bytes(str);
    f << "</main><center><small>Created by <a href=\"https:/"\
            "/github.com/mooction/MyNotePad\">MyNotePad</a>.</small></center>";
    if (str.find(L'$') != std::string::npos ||
        str.find(L"\\[") != std::string::npos ||
        str.find(L"\\(") != std::string::npos)
    {
        f << R"raw(
<script type="text/x-mathjax-config">
MathJax.Hub.Config({
    TeX: { equationNumbers: { autoNumber: "AMS" } },
    tex2jax: {inlineMath: [['$','$'], ['\\(','\\)']]}
});
</script>
<script type="text/javascript" src="https://cdn.bootcss.com/mathjax/2.7.5/MathJax.js?config=default"></script>)raw";
    }
    f << "</body><html>";
    f.close();
}

std::wstring MyFrame::all_to_string()
{
    return article->GetValue().ToStdWstring();
}

#ifdef _DEBUG
#define SHOW_PARSING_TIME
#endif

// convert markdown to html, which is stored in @str.
void MyFrame::md2html(std::wstring& str)
{
    //std::wregex regex_table_align(L"^([:\\s]?-+[:\\s]?\\|?)+$");
    //bool re = std::regex_match(str, regex_table_align);
    //int j = 5;
#ifdef SHOW_PARSING_TIME
        clock_t begin = clock();
#endif
    std::wostringstream wos;

    auto scanned = scanner(str);
    parse_fromlex(wos, std::begin(scanned), std::end(scanned));

#ifdef SHOW_PARSING_TIME
    clock_t end = clock();
    double t = double(end - begin) / CLOCKS_PER_SEC;
    wos << L"<br><p>elapsed time: " << std::setprecision(2) << t << L" s</p>";
#endif // SHOW_PARSING_TIME

    str = wos.str();
}

/* ==========================================
 * CREATE WINDOW AND EVENT HANDLERS
 * ==========================================
 */
MyFrame::MyFrame(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title),
    article(nullptr),
    bShowLineNumber(true),
    bResized(false),
    bSaved(true),
    bWordWrap(true),
    fontSize(28),
    lineNumSize(23),
    lineHeight(28),
    fontFace(FONT_MSYAHEI),
    openedFile(MNP_DOC_NOTITLE)
{
#ifdef _WIN32
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICON_MYNOTEPAD));
    SendMessage(this->GetHWND(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
#else
    SetIcon(wxICON(ICON_MYNOTEPAD));
#endif
    /*
     * Create menu items
     */
    wxMenu *fileMenu = new wxMenu();
    fileMenu->Append(FILE_NEW, L"&New\tCtrl+N", L"Create new file");
    fileMenu->Append(FILE_OPEN, L"&Open...\tCtrl+O", L"Quit this program");
    fileMenu->Append(FILE_SAVE, L"&Save\tCtrl+S", L"Write changes to the file");
    fileMenu->Append(FILE_SAVEAS, L"Save &As...\tCtrl+Shift+S", L"Save current document to a file");
    fileMenu->AppendSeparator();
    fileMenu->Append(FILE_COPYHTML, L"Copy &HTML", L"Copy HTML to clipboard");
    fileMenu->Append(FILE_EXPORTHTML, L"&Export HTML...", L"Export HTML to file");
    fileMenu->AppendSeparator();
    fileMenu->Append(FILE_QUIT, L"E&xit\tAlt+F4", L"Quit this program");

    wxMenu *editMenu = new wxMenu();
    editMenu->Append(EDIT_UNDO, L"&Undo\tCtrl+Z", L"Undo");
    editMenu->Append(EDIT_REDO, L"&Redo\tCtrl+Y", L"Redo");
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_CUT, L"C&ut\tCtrl+X", L"Cut to clipboard");
    editMenu->Append(EDIT_COPY, L"&Copy\tCtrl+C", L"Copy to clipboard");
    editMenu->Append(EDIT_PASTE, L"&Paste\tCtrl+V", L"Paste from clipboard");
    editMenu->Append(EDIT_DELETE, L"&Delete\tDel", L"Delete selected items");
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_SELALL, L"&Select All\tCtrl+A", L"Select all");
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_OPTIONS, L"&Options...", L"Optional settings");
    editMenu->Enable(EDIT_OPTIONS, false);

    wxMenu *viewMenu = new wxMenu();
    viewMenu->AppendCheckItem(VIEW_THEMELIGHT, L"&Light", L"Use white theme");
    viewMenu->AppendCheckItem(VIEW_THEMEDARK, L"&Black", L"Use black theme");
    viewMenu->AppendSeparator();
    viewMenu->AppendCheckItem(VIEW_LINENUMBER, L"&Line Number", L"Show line number");
    viewMenu->Check(VIEW_LINENUMBER, true);
    wxMenu *fontMenu = new wxMenu();
    fontMenu->AppendCheckItem(VIEW_FONT_MSYAHEI, FONT_MSYAHEI, FONT_MSYAHEI);
    fontMenu->AppendCheckItem(VIEW_FONT_LUCIDA, FONT_LUCIDA, FONT_LUCIDA);
    fontMenu->AppendCheckItem(VIEW_FONT_COURIER, FONT_COURIER, FONT_COURIER);
    fontMenu->AppendCheckItem(VIEW_FONT_CONSOLAS, FONT_CONSOLAS, FONT_CONSOLAS);
    fontMenu->AppendSeparator();
    fontMenu->Append(VIEW_FONT_SELECT, L"Select Font", L"Select Font");
    fontMenu->Enable(VIEW_FONT_SELECT, false);
    viewMenu->AppendSubMenu(fontMenu, L"Font");

    wxMenu *formatMenu = new wxMenu();
    formatMenu->Append(FORMAT_BROWSER, L"&Show in browser...\tF5", L"Show in browser");
    formatMenu->AppendCheckItem(FORMAT_WORDWRAP, L"Line &Wrap", L"Line wrap");
    formatMenu->Check(FORMAT_WORDWRAP, true);

    wxMenu *helpMenu = new wxMenu();
    helpMenu->Append(HELP_ABOUT, L"&About...\tAlt+A", L"Show about dialog");

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu,L"&File");
    menuBar->Append(editMenu, L"&Edit");
    menuBar->Append(viewMenu, L"&View");
    menuBar->Append(formatMenu, L"&Format");
    menuBar->Append(helpMenu, L"&Help");
    SetMenuBar(menuBar);

    /*
     * Initialize edit area
     */
    wxBoxSizer *boxSizer = new wxBoxSizer( wxVERTICAL );
    // Regard to the control, see https://wiki.wxwidgets.org/WxStyledTextCtrl
    article = new MyFrame::notepadCtrl(this, 0, wxDefaultPosition, wxDefaultSize, wxSTC_EDGE_LINE);
    article->SetEOLMode(wxSTC_EOL_LF);
    article->SetWrapMode(wxSTC_WRAP_WORD);
    article->SetLexer(wxSTC_LEX_MARKDOWN);
    // Left-margin for line number 
    article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, MNP_PADDING_LEFT * 2);
    article->StyleSetSize(wxSTC_STYLE_LINENUMBER, fontSize / 2);

    boxSizer->Add(article, 1, wxEXPAND | wxALL, 1);
    this->SetSizer(boxSizer);
    article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnRightButtonDown), NULL, this);
    article->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(MyFrame::OnRightButtonUp), NULL, this);
    //article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnLeftButtonDown), NULL, this);
    //article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnLeftButtonUp), NULL, this);
    //article->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(MyFrame::OnLeftButtonDBClick), NULL, this);
    //article->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(MyFrame::OnKeyDown), NULL, this);
    //article->Connect(wxEVT_KEY_UP, wxKeyEventHandler(MyFrame::OnKeyUp), NULL, this);
    article->Connect(wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnChar), NULL, this);
    // Drag and Drop to open file
    article->SetDropTarget(new MyDropTarget(this));
    Centre();
}

void MyFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
    wxString s = wxStandardPaths::Get().GetExecutablePath();
    LOG_MESSAGE(s);
    wxExecute(s);
}

void MyFrame::OnOpen(wxCommandEvent& event)
{
    sureToQuit(event);   // notify the user to save data
    wxFileDialog openFileDialog(this,
                                L"Open file", "", "",
                                L"MarkDown (*.md)|*.md|All Files (*.*)|*.*",
                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::string filepath = openFileDialog.GetPath().ToStdString();
    LOG_MESSAGE(filepath);
    loadFile(filepath);
}

void MyFrame::OnSave(wxCommandEvent& event)
{
    if (openedFile == MNP_DOC_NOTITLE)
        OnSaveAs(event);
    else
    {
        std::ofstream f(openedFile);
        if (!f.good())
        {
            wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
            return;
        }
        std::wstring str = all_to_string();
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        f << cvt.to_bytes(str);
        updateSaveState(true);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog saveFileDialog(this,
                                L"Save As", "", "",
                                L"MarkDown (*.md)|*.md|All Files (*.*)|*.*",
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filepath = saveFileDialog.GetPath();
    if (filepath.Right(3) != ".md")
        filepath += ".md";
    LOG_MESSAGE(filepath);
    std::ofstream f(filepath.ToStdString());
    if (!f.good())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    std::wstring str = all_to_string();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    f << cvt.to_bytes(str);

    openedFile = filepath;
    updateSaveState(true);
}

void MyFrame::OnCopyHtml(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    std::wstring str = all_to_string();
    if (str.empty())
        return;
    if (wxTheClipboard->Open())
    {
        md2html(str);
        wxTheClipboard->SetData( new wxTextDataObject(str) );
        wxTheClipboard->Close();
        wxTheClipboard->Flush();
    }
}

void MyFrame::OnExportHTML(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    wxFileDialog saveFileDialog(this,
                                L"Export HTML", "", "",
                                L"HTML (*.html;*.htm)|*.html;*.htm|All Files (*.*)|*.*",
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::string filepath = saveFileDialog.GetPath().ToStdString();
    saveHTML(filepath);
}

void MyFrame::OnQuit(wxCommandEvent& event)
{
    LOG_MESSAGE("");
    Close(true);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    LOG_MESSAGE("");
    wxCommandEvent evt;
    if (sureToQuit(evt))
        event.Skip(true);
}

void MyFrame::OnUndo(wxCommandEvent& WXUNUSED(event))
{
    article->Undo();
}

void MyFrame::OnRedo(wxCommandEvent& WXUNUSED(event))
{
    article->Redo();
}

void MyFrame::OnCut(wxCommandEvent& event)
{
    LOG_MESSAGE("");
    if (article->CanCut())
    {
        OnCopy(event);
        article->RemoveSelection();
        updateSaveState(false);
    }
}

void MyFrame::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    wxString str;
    if (article->CanCopy())
        str = article->GetSelectedText();
    if (str.IsEmpty())
        return;
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData( new wxTextDataObject(str) );
        wxTheClipboard->Close();
        wxTheClipboard->Flush();
    }
}

void MyFrame::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    updateSaveState(false);
    article->Paste();
}

void MyFrame::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    if (article->CanCut())
    {
        updateSaveState(false);
        article->RemoveSelection();
    }
    else {
        int p = article->GetCurrentPos();
        article->Remove(p, p + 1);
    }
}

void MyFrame::OnSelAll(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    article->SelectAll();
}

void MyFrame::OnOptions(wxCommandEvent& WXUNUSED(event))
{
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnMenuOpen(wxMenuEvent& event)
{
    wxMenu *menu = event.GetMenu();
    if (menu == NULL)
        return;
    wxMenuItem *item = menu->FindItem(EDIT_CUT);
    if (item != NULL)
    {
        item->Enable(article->CanCut());
    }
    item = menu->FindItem(EDIT_COPY);
    if (item != NULL)
    {
        item->Enable(article->CanCut());
    }
    item = menu->FindItem(EDIT_UNDO);
    if (item != NULL)
    {
        item->Enable(article->CanUndo());
    }
    item = menu->FindItem(EDIT_REDO);
    if (item != NULL)
    {
        item->Enable(article->CanRedo());
    }
}

void MyFrame::ApplyTheme()
{
    // Syntax highlight background
    for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_MARKDOWN_CODEBK; i++)
        article->StyleSetBackground(i, bgColorEdit);
    article->StyleSetBackground(wxSTC_MARKDOWN_CODE, bgColorLight);
    article->StyleSetBackground(wxSTC_MARKDOWN_CODE2, bgColorLight);
    article->StyleSetBackground(wxSTC_MARKDOWN_CODEBK, bgColorLight);

    // Syntax highlight foreground
    //article->StyleSetForeground(wxSTC_MARKDOWN_LINE_BEGIN, *wxRED);
    //article->StyleSetForeground(wxSTC_MARKDOWN_EM1, *wxRED);
    //article->StyleSetForeground(wxSTC_MARKDOWN_EM2, *wxRED);
    //article->StyleSetForeground(wxSTC_MARKDOWN_PRECHAR, *wxRED);
    //article->StyleSetForeground(wxSTC_MARKDOWN_HRULE, *wxRED);
    article->StyleSetForeground(wxSTC_MARKDOWN_DEFAULT, fontColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_STRONG1, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_STRONG2, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER1, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER2, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER3, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER4, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER5, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_HEADER6, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_ULIST_ITEM, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_OLIST_ITEM, strongColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_BLOCKQUOTE, quoteColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_STRIKEOUT, quoteColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_LINK, linkColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_CODE, quoteColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_CODE2, quoteColor);
    article->StyleSetForeground(wxSTC_MARKDOWN_CODEBK, quoteColor);

    // Other
    article->SetCaretForeground(fontColor);
    article->SetSelBackground(true, bgColorSel);
    article->StyleSetBackground(wxSTC_STYLE_DEFAULT, bgColorEdit);
    article->StyleSetForeground(wxSTC_STYLE_LINENUMBER, lineNumFontColor);
    article->StyleSetBackground(wxSTC_STYLE_LINENUMBER, bgColorEdit);

    // Styles in range 32..38 are predefined for parts of the UI and are not used as normal styles.
    //for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_STYLE_DEFAULT; i++)
    for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_MARKDOWN_CODEBK; i++)
        article->StyleSetSize(i, fontSize / 2);
    //article->StyleSetSize(wxSTC_STYLE_LINENUMBER, fontSize / 2);

    article->StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
    article->StyleSetBold(wxSTC_MARKDOWN_STRONG2, true);
}

void MyFrame::OnThemeLight(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    themeLight();
    ApplyTheme();
    saveSettings();
}

void MyFrame::OnThemeDark(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    themeDark();
    ApplyTheme();
    saveSettings();
}

void MyFrame::OnLineNumber(wxCommandEvent& WXUNUSED(event))
{
    bShowLineNumber = !bShowLineNumber;
    if (GetMenuBar()->IsChecked(VIEW_LINENUMBER)) {
        article->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, MNP_PADDING_LEFT * 2);

    } else {
        article->SetMarginType(0, wxSTC_MARGIN_SYMBOL);
        article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, MNP_PADDING_LEFT / 2);
    }
    saveSettings();
}

void MyFrame::OnFont(wxCommandEvent& event)
{
    wxMenuBar *menuBar = GetMenuBar();
    menuBar->Check(VIEW_FONT_MSYAHEI, false);
    menuBar->Check(VIEW_FONT_LUCIDA, false);
    menuBar->Check(VIEW_FONT_COURIER, false);
    menuBar->Check(VIEW_FONT_CONSOLAS, false);
    switch (event.GetId())
    {
        case VIEW_FONT_MSYAHEI:
            menuBar->Check(VIEW_FONT_MSYAHEI, true);
            fontFace = FONT_MSYAHEI;
            break;
        case VIEW_FONT_LUCIDA:
            menuBar->Check(VIEW_FONT_LUCIDA, true);
            fontFace = FONT_LUCIDA;
            break;
        case VIEW_FONT_COURIER:
            menuBar->Check(VIEW_FONT_COURIER, true);
            fontFace = FONT_COURIER;
            break;
        case VIEW_FONT_CONSOLAS:
            menuBar->Check(VIEW_FONT_CONSOLAS, true);
            fontFace = FONT_CONSOLAS;
            break;
        default:
            break;
    }
    LOG_MESSAGE(fontFace);

    for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_MARKDOWN_CODEBK; i++)
        article->StyleSetFaceName(i, fontFace);
    saveSettings();
}

void MyFrame::OnFontSelect(wxCommandEvent& WXUNUSED(event))
{
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnBrowser(wxCommandEvent& WXUNUSED(event))
{
#ifdef _WIN32
    std::string s = confFilePath + "preview.html";
#else
    wxString path = wxStandardPaths::Get().GetTempDir() + "/preview.html";
    std::string s = path.ToStdString();
#endif
    saveHTML(s);
    LOG_MESSAGE(s);
    wxLaunchDefaultApplication(s);
}

void MyFrame::OnWordWrap(wxCommandEvent& WXUNUSED(event))
{
    bWordWrap = !bWordWrap;
    if (GetMenuBar()->IsChecked(FORMAT_WORDWRAP)) {
        article->SetWrapMode(wxSTC_WRAP_WORD);
    } else {
        article->SetWrapMode(wxSTC_WRAP_NONE);
    }
    saveSettings();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    std::wstringstream detail;
    detail << MNP_APPNAME << '\t' << MNP_VERSION_MAJOR << '.' 
           << MNP_VERSION_MINOR << '.' << MNP_VERSION_PATCH << '\n'
           << wxVERSION_STRING << '\n'
           << MNP_COPYRIGHT;

    wxMessageBox(detail.str(), "About", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnLeftButtonDown(wxMouseEvent& event)
{
    LOG_MESSAGE("");
    event.Skip(); // Default behaviour
}

/*
 * Double-clicking the left mouse button
 * actually generates a sequence of four messages:
 * WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, and WM_LBUTTONUP.
 * Only windows having CS_DBLCLKS style can receive WM_LBUTTONDBLCLK.
 */
void MyFrame::OnLeftButtonDBClick(wxMouseEvent& event)
{
    LOG_MESSAGE("");
    event.Skip(); // Default behaviour
}

void MyFrame::OnLeftButtonUp(wxMouseEvent& event)
{
    LOG_MESSAGE("");
    event.Skip(); // Default behaviour
}

void MyFrame::OnRightButtonDown(wxMouseEvent& event)
{
    LOG_MESSAGE("");
    event.Skip(); // Default behaviour
}

void MyFrame::OnRightButtonUp(wxMouseEvent& event)
{
    LOG_MESSAGE("");
    //wxMenu* editMenu = new wxMenu();
    //editMenu->Append(EDIT_CUT, L"C&ut\tCtrl+X", L"Cut to clipboard");
    //editMenu->Append(EDIT_COPY, L"&Copy\tCtrl+C", L"Copy to clipboard");
    //editMenu->Append(EDIT_PASTE, L"&Paste\tCtrl+V", L"Paste from clipboard");
    //editMenu->Append(EDIT_DELETE, L"&Delete\tDel", L"Delete selected items");
    //editMenu->AppendSeparator();
    //editMenu->Append(EDIT_SELALL, L"&Select All\tCtrl+A", L"Select all");
    //if (article->CanCut())
    //{
    //    editMenu->Enable(EDIT_CUT, true);
    //    editMenu->Enable(EDIT_COPY, true);
    //}
    //PopupMenu(editMenu);
    //event.Skip(false);
    event.Skip();
}

void MyFrame::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    LOG_MESSAGE(wxString() + uc);
    if ( uc != WXK_NONE )
    {
        // It's a "normal" character. Notice that this includes
        // control characters in 1..31 range, e.g. WXK_RETURN or
        // WXK_BACK, so check for them explicitly.
        if ( uc >= 32 )
        {
            switch (uc)
            {
            case WXK_BACK:
                if (article->IsEmpty())
                    return;
                break;
            case WXK_RETURN:
                break;
            default:
                break;
            }
            updateSaveState(false);
            event.Skip(true);
        }
        else
        {
            // It's a control character, eg. WXK_ESCAPE, WXK_F1, ...
            event.Skip(true);
            switch (event.GetKeyCode())
            {
            case WXK_CONTROL_X:
            case WXK_CONTROL_V:
                updateSaveState(false);
                return;
            default:
                break;
            }
        }
    }
    else // No Unicode equivalent.
    {
        event.Skip(true);
    }
}

bool MyDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
    wxCommandEvent evt;
    frame->sureToQuit(evt);
    frame->loadFile(filenames[0].ToStdString());
    return true;
}

