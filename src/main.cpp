#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/datstrm.h>
#include <wx/dnd.h>
#include <wx/colour.h>
#include <wx/caret.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/filedlg.h>
#include <wx/clipbrd.h>
#include <iostream>
#include <string>

#include "config.h"
#include "main.h"
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
    EVT_MENU(FILE_QUIT, MyFrame::OnMenuClose)

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
    frame->SetSize(400, 300, frame->FromDIP(800), frame->FromDIP(600));
    frame->loadSettings();

    // load the file specified in cmdLine
    if (argc == 2 && argv[1][0] != '\0')
        frame->loadFile(argv[1]);

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
    article->SetMarginType(wxSTC_MARGINOPTION_NONE, wxSTC_MARGIN_NUMBER);
    article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, FromDIP(MNP_PADDING_LEFT * 2));

#ifdef _WIN32
    confFilePath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + '\\';
#else
    confFilePath = wxStandardPaths::Get().GetUserDataDir();
    confFilePath += '/';
    if (!wxDir::Exists(confFilePath))
        // Disallow group & others to write
        wxDir::Make(confFilePath, ~(wxPOSIX_GROUP_WRITE | wxPOSIX_OTHERS_WRITE));
#endif
    wxLogDebug("loading setting: %s", (confFilePath + MNP_CONFIG_FILE).ToStdString());

    wxLogNull nolog; // Disable logging temporarily to avoid a popup message box
    // Load user data
    wxFileInputStream infile(confFilePath + MNP_CONFIG_FILE);
    if (infile.IsOk())
    {
        wxTextInputStream in(infile);
        wxString line;
        while (!(line = in.ReadLine()).IsEmpty())
        {
            if (line[0] == wxS('#'))
                continue;   // # this is comment

            const int pos = line.Find(wxS('='));    // eg. best = 999
            if (pos != wxNOT_FOUND)
            {
                wxString key = line.substr(0, pos); // acquire key
                wxString val = line.substr(pos);    // acquire value
                if (val.Last() == '\r') val.RemoveLast(1);    // remove '\r' at the end
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
                    article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, 0);
                }
                else if (key == MNP_CONFIG_FONTNAME)
                {
                    if (val == FONT_1) {
                        fontFace = FONT_1;
                    }
                    else if (val == FONT_2) {
                        fontFace = FONT_2;
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_LUCIDA, true);
                    }
                    else if (val == FONT_3) {
                        fontFace = FONT_3;
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_COURIER, true);
                    }
                    else if (val == FONT_4) {
                        fontFace = FONT_4;
                        GetMenuBar()->Check(VIEW_FONT_MSYAHEI, false);
                        GetMenuBar()->Check(VIEW_FONT_CONSOLAS, true);
                    }
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
    wxString pathname = confFilePath + MNP_CONFIG_FILE;
    wxLogDebug("saving setting: %s", pathname.ToStdString());
    wxFileOutputStream outfile(pathname);
    if (!outfile.IsOk())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    wxTextOutputStream out(outfile);
    out << "# Edit this file to config MyNotePad\r\n";
    out << wxString(MNP_CONFIG_THEME) << '=' << theme << "\r\n";
    out << wxString(MNP_CONFIG_WORDWRAP) << '=' << bWordWrap << "\r\n";
    out << wxString(MNP_CONFIG_LINENUMBER) << '=' << bShowLineNumber << "\r\n";
    out << wxString(MNP_CONFIG_FONTNAME) << '=' << wxString(fontFace);
}

int MyFrame::widthOfLineNumber() const
{
    int n = article->GetNumberOfLines();
    int count = 1;
    do {
        count++;
        n /= 10;
    } while (n > 0);
    return MNP_PADDING_LEFT * count;
}

void MyFrame::updateSaveState(bool saved)
{
    bSaved = saved;
    if (bSaved)
        SetTitle(openedFile + MNP_DOC_TITLE);
    else
        SetTitle(openedFile + " *" + MNP_DOC_TITLE);

    // any text edit action will fire this function.
    // so we can make any other update.
    if (bShowLineNumber) {
        article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, FromDIP(widthOfLineNumber()));
    }
}

void MyFrame::loadFile(wxString pathname)
{
    // remove quote
    if (pathname[0] == '\"') {
        pathname = pathname.substr(1, pathname.size() - 1);
    }
    wxLogDebug("loading file: %s", pathname.ToStdString());
    // load file
    wxFileInputStream infile(pathname);
    if (!infile.IsOk()) {
        wxMessageBox(wxS("Failed to load \"") + pathname + wxS("\"."),
            MNP_APPNAME, wxOK | wxICON_WARNING, this);
        return;
    }
    std::string bytes;
    bytes.resize(infile.GetSize());
    infile.ReadAll(&bytes[0], bytes.size());

    openedFile = pathname;
    updateSaveState(true);
    article->SetValue(wxString::FromUTF8(bytes.c_str()));
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

void MyFrame::saveHTML(const wxString& pathname)
{
    wxFileOutputStream outfile(pathname);
    if (!outfile.IsOk())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    wxTextOutputStream out(outfile);
    out << wxS("<!DOCTYPE><html><head><meta charset='utf-8'/>");
#ifdef WIN32
        out << wxS("<link href='file:///")
            << confFilePath << wxS("style.css' rel='stylesheet'>\n")
            << wxS("<link href='file:///")
            << confFilePath << wxS("highlight.css' rel='stylesheet'>\n")
            << wxS("<script type='text/javascript' src='file:///")
            << confFilePath << wxS("highlight.pack.js'></script>\n");
#else
    out << wxS("<link href='file:///")
    << wxString(MNP_INSTALL_PATH) << wxS("style.css' rel='stylesheet'>\n")
    << wxS("<link href='file:///")
    << wxString(MNP_INSTALL_PATH) << wxS("highlight.css' rel='stylesheet'>\n")
    << wxS("<script type='text/javascript' src='file:///")
    << wxString(MNP_INSTALL_PATH) << wxS("highlight.pack.js'></script>\n");
#endif
    out << wxS(R"raw(<script type="text/javascript">
    window.onload = function() {
        var aCodes = document.getElementsByTagName('pre');
        for (var i=0; i < aCodes.length; i++) {
            hljs.highlightBlock(aCodes[i]);
        }
    };
    // hljs.initHighlightingOnLoad();
</script>
</head><body><main>)raw");
    wxString str = article->GetValue();
    md2html(str);
    out << str << wxS("</main><center><small>Created by <a href='https:/"\
            "/github.com/mooction/MyNotePad'>MyNotePad</a>.</small></center>");
    if (str.Find('$') != wxNOT_FOUND ||
        str.Find("\\[") != wxNOT_FOUND ||
        str.Find("\\(") != wxNOT_FOUND)
    {
        out << wxS(R"raw(
<script type="text/x-mathjax-config">
MathJax.Hub.Config({
    TeX: { equationNumbers: { autoNumber: "AMS" } },
    tex2jax: {inlineMath: [['$','$'], ['\\(','\\)']]}
});
</script>
<script type="text/javascript" src="https://cdn.bootcss.com/mathjax/2.7.5/MathJax.js?config=default"></script>)raw");
    }
    out << wxS("</body><html>");
}

#ifdef USE_EXTERN_LIB
    #include "maddy/parser.h"
#else
    #include "lex_parse.h"
#endif

// convert markdown to html, which is stored in @str.
void MyFrame::md2html(wxString& str)
{
#ifdef USE_MADDY
    std::stringstream input(str.ToUTF8());
    maddy::Parser parser;
    str = wxString::FromUTF8(parser.Parse(input));
#else
    auto scanned = scanner(str.ToStdWstring());
    std::wostringstream wos;
    parse_fromlex(wos, std::begin(scanned), std::end(scanned));
    str = wos.str();
#endif
}

/* ==========================================
 * CREATE WINDOW AND EVENT HANDLERS
 * ==========================================
 */
MyFrame::MyFrame(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title),
    article(nullptr),
    bShowLineNumber(true),
    bSaved(true),
    bWordWrap(true),
    fontSize(13),
    lineNumFontSize(10),
    fontFace(FONT_1),
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
    fileMenu->Append(FILE_NEW, wxS("&New\tCtrl+N"), wxS("Create new file"));
    fileMenu->Append(FILE_OPEN, wxS("&Open...\tCtrl+O"), wxS("Open new file"));
    fileMenu->Append(FILE_SAVE, wxS("&Save\tCtrl+S"), wxS("Write changes to the file"));
    fileMenu->Append(FILE_SAVEAS, wxS("Save &As...\tCtrl+Shift+S"), wxS("Save current document to a file"));
    fileMenu->AppendSeparator();
    fileMenu->Append(FILE_COPYHTML, wxS("Copy &HTML"), wxS("Copy HTML to clipboard"));
    fileMenu->Append(FILE_EXPORTHTML, wxS("&Export HTML..."), wxS("Export HTML to file"));
    fileMenu->AppendSeparator();
    fileMenu->Append(FILE_QUIT, wxS("E&xit\tAlt+F4"), wxS("Quit this program"));

    wxMenu *editMenu = new wxMenu();
    editMenu->Append(EDIT_UNDO, wxS("&Undo\tCtrl+Z"), wxS("Undo"));
    editMenu->Append(EDIT_REDO, wxS("&Redo\tCtrl+Y"), wxS("Redo"));
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_CUT, wxS("C&ut\tCtrl+X"), wxS("Cut to clipboard"));
    editMenu->Append(EDIT_COPY, wxS("&Copy\tCtrl+C"), wxS("Copy to clipboard"));
    editMenu->Append(EDIT_PASTE, wxS("&Paste\tCtrl+V"), wxS("Paste from clipboard"));
    editMenu->Append(EDIT_DELETE, wxS("&Delete\tDel"), wxS("Delete selected items"));
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_SELALL, wxS("&Select All\tCtrl+A"), wxS("Select all"));
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_OPTIONS, wxS("&Options..."), wxS("Optional settings"));
    editMenu->Enable(EDIT_OPTIONS, false);

    wxMenu *viewMenu = new wxMenu();
    viewMenu->AppendCheckItem(VIEW_THEMELIGHT, wxS("&Light"), wxS("Use white theme"));
    viewMenu->AppendCheckItem(VIEW_THEMEDARK, wxS("&Black"), wxS("Use black theme"));
    viewMenu->AppendSeparator();
    viewMenu->AppendCheckItem(VIEW_LINENUMBER, wxS("&Line Number"), wxS("Show line number"));
    viewMenu->Check(VIEW_LINENUMBER, true);
    wxMenu *fontMenu = new wxMenu();
    fontMenu->AppendCheckItem(VIEW_FONT_MSYAHEI, FONT_1, FONT_1);
    fontMenu->AppendCheckItem(VIEW_FONT_LUCIDA, FONT_2, FONT_2);
    fontMenu->AppendCheckItem(VIEW_FONT_COURIER, FONT_3, FONT_3);
    fontMenu->AppendCheckItem(VIEW_FONT_CONSOLAS, FONT_4, FONT_4);
    fontMenu->AppendSeparator();
    fontMenu->Append(VIEW_FONT_SELECT, wxS("Select Font"), wxS("Select Font"));
    fontMenu->Enable(VIEW_FONT_SELECT, false);
    viewMenu->AppendSubMenu(fontMenu, wxS("Font"));

    wxMenu *formatMenu = new wxMenu();
    formatMenu->Append(FORMAT_BROWSER, wxS("&Show in browser...\tF5"), wxS("Show in browser"));
    formatMenu->AppendCheckItem(FORMAT_WORDWRAP, wxS("Line &Wrap"), wxS("Line wrap"));
    formatMenu->Check(FORMAT_WORDWRAP, true);

    wxMenu *helpMenu = new wxMenu();
    helpMenu->Append(HELP_ABOUT, wxS("&About...\tAlt+A"), wxS("Show about dialog"));

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxS("&File"));
    menuBar->Append(editMenu, wxS("&Edit"));
    menuBar->Append(viewMenu, wxS("&View"));
    menuBar->Append(formatMenu, wxS("&Format"));
    menuBar->Append(helpMenu, wxS("&Help"));
    SetMenuBar(menuBar);

    /*
     * Initialize edit area
     */
    wxBoxSizer *boxSizer = new wxBoxSizer( wxVERTICAL );
    // Regard to the control, see https://wiki.wxwidgets.org/WxStyledTextCtrl
    article = new MyFrame::notepadCtrl(this, 0, wxDefaultPosition, wxDefaultSize);
    article->SetEOLMode(wxSTC_EOL_LF); // linux line feed
    article->SetWrapMode(wxSTC_WRAP_WORD); // word wrap
    article->SetLexer(wxSTC_LEX_MARKDOWN); // syntax highlight

    boxSizer->Add(article, 1, wxEXPAND | wxALL, 1);
    this->SetSizer(boxSizer);
    article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnRightButtonDown), NULL, this);
    article->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(MyFrame::OnRightButtonUp), NULL, this);
    //article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnLeftButtonDown), NULL, this);
    //article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnLeftButtonUp), NULL, this);
    //article->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(MyFrame::OnLeftButtonDBClick), NULL, this);
    article->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(MyFrame::OnKeyDown), NULL, this);
    //article->Connect(wxEVT_KEY_UP, wxKeyEventHandler(MyFrame::OnKeyUp), NULL, this);
    article->Connect(wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnChar), NULL, this);
    // Drag and Drop to open file
    article->SetDropTarget(new MyDropTarget(this));
    Centre();
}

void MyFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
    wxString exe = wxStandardPaths::Get().GetExecutablePath();
    wxLogDebug("executing: %s", exe.ToStdString());
    wxExecute(exe);
}

void MyFrame::OnOpen(wxCommandEvent& event)
{
    sureToQuit(event);   // notify the user to save data
    wxFileDialog openFileDialog(this,
                                wxS("Open file"), "", "",
                                wxS("MarkDown (*.md)|*.md|All Files (*.*)|*.*"),
                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filepath = openFileDialog.GetPath();
    wxLogDebug("opening: %s", filepath.ToStdString());
    loadFile(filepath);
}

void MyFrame::OnSave(wxCommandEvent& event)
{
    if (openedFile == wxString(MNP_DOC_NOTITLE))
        OnSaveAs(event);
    else
    {
        wxFileOutputStream outfile(openedFile);
        if (!outfile.IsOk())
        {
            wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
            return;
        }
        wxTextOutputStream out(outfile);
        out << article->GetValue();
        updateSaveState(true);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog saveFileDialog(this,
                                wxS("Save As"), "", "",
                                wxS("MarkDown (*.md)|*.md|All Files (*.*)|*.*"),
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filepath = saveFileDialog.GetPath();
    if (filepath.Right(3) != ".md")
        filepath += ".md";

    wxLogDebug("saving: %s", filepath.ToStdString());

    wxFileOutputStream outfile(filepath);
    if (!outfile.IsOk())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    wxTextOutputStream out(outfile);
    out << article->GetValue();

    openedFile = filepath;
    updateSaveState(true);
}

void MyFrame::OnCopyHtml(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
    wxString str = article->GetValue();
    if (str.empty())
        return;
    if (wxTheClipboard->Open())
    {
        md2html(str);
        wxTheClipboard->SetData( new wxTextDataObject(str) );
        wxTheClipboard->Close();
    }
}

void MyFrame::OnExportHTML(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog saveFileDialog(this,
                                wxS("Export HTML"), "", "",
                                wxS("HTML (*.html;*.htm)|*.html;*.htm|All Files (*.*)|*.*"),
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    wxString filepath = saveFileDialog.GetPath();
    if (filepath.Right(3) != ".md")
        filepath += ".md";
    wxLogDebug("exporting html: ", filepath);
    saveHTML(filepath);
}

void MyFrame::OnMenuClose(wxCommandEvent& event)
{
    wxLogDebug("%s", __func__);
    Close(true);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    wxLogDebug("%s", __func__);
    wxCommandEvent evt;
    if (sureToQuit(evt))
        event.Skip(true);

    wxTheClipboard->Flush();
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
    wxLogDebug("%s", __func__);
    if (article->CanCut())
    {
        OnCopy(event);
        article->RemoveSelection();
        updateSaveState(false);
    }
}

void MyFrame::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
    wxString str;
    if (article->CanCopy())
        str = article->GetSelectedText();
    if (str.IsEmpty())
        return;
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData( new wxTextDataObject(str) );
        wxTheClipboard->Close();
    }
}

void MyFrame::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
    updateSaveState(false);
    article->Paste();
}

void MyFrame::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
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
    wxLogDebug("%s", __func__);
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
    for (int i = wxSTC_MARKDOWN_DEFAULT; i <= wxSTC_MARKDOWN_CODEBK; i++)
        article->StyleSetSize(i, FromDIP(fontSize));

    article->StyleSetSize(wxSTC_STYLE_DEFAULT, FromDIP(lineNumFontSize));
    //article->StyleSetSize(wxSTC_STYLE_LINENUMBER, FromDIP(lineNumFontSize));

    article->StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
    article->StyleSetBold(wxSTC_MARKDOWN_STRONG2, true);
}

void MyFrame::OnThemeLight(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
    themeLight();
    ApplyTheme();
    saveSettings();
}

void MyFrame::OnThemeDark(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("%s", __func__);
    themeDark();
    ApplyTheme();
    saveSettings();
}

void MyFrame::OnLineNumber(wxCommandEvent& WXUNUSED(event))
{
    bShowLineNumber = !bShowLineNumber;
    if (GetMenuBar()->IsChecked(VIEW_LINENUMBER)) {
        article->SetMarginType(wxSTC_MARGINOPTION_NONE, wxSTC_MARGIN_NUMBER);
        article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, FromDIP(widthOfLineNumber()));
    } else {
        article->SetMarginType(wxSTC_MARGINOPTION_NONE, wxSTC_MARGIN_SYMBOL);
        article->SetMarginWidth(wxSTC_MARGINOPTION_NONE, 0);
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
            fontFace = FONT_1;
            break;
        case VIEW_FONT_LUCIDA:
            menuBar->Check(VIEW_FONT_LUCIDA, true);
            fontFace = FONT_2;
            break;
        case VIEW_FONT_COURIER:
            menuBar->Check(VIEW_FONT_COURIER, true);
            fontFace = FONT_3;
            break;
        case VIEW_FONT_CONSOLAS:
            menuBar->Check(VIEW_FONT_CONSOLAS, true);
            fontFace = FONT_4;
            break;
        default:
            break;
    }
    wxLogDebug("using fontface: %s", fontFace);

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
    wxString filepath;
    if (openedFile == MNP_DOC_NOTITLE) {
#ifdef _WIN32
        filepath = confFilePath + "preview.html";
    } else {
        filepath = wxPathOnly(openedFile) + "\\preview.html";
#else
        filepath = wxStandardPaths::Get().GetTempDir() + "/preview.html";
    } else {
        filepath = wxPathOnly(openedFile) + "/preview.html";
#endif
    }
    wxLogDebug("generating html file: %s", filepath.ToStdString());
    saveHTML(filepath);
    wxLaunchDefaultApplication(filepath);
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
    detail << wxString(MNP_APPNAME) << ' ' << MNP_VERSION_MAJOR << '.'
           << MNP_VERSION_MINOR << '.' << MNP_VERSION_PATCH << '\n'
           << wxVERSION_STRING << '\n'
           << wxString(MNP_COPYRIGHT);

    wxMessageBox(detail.str(), wxS("About"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnLeftButtonDown(wxMouseEvent& event)
{
    wxLogDebug("%s", __func__);
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
    wxLogDebug("%s", __func__);
    event.Skip(); // Default behaviour
}

void MyFrame::OnLeftButtonUp(wxMouseEvent& event)
{
    wxLogDebug("%s", __func__);
    event.Skip(); // Default behaviour
}

void MyFrame::OnRightButtonDown(wxMouseEvent& event)
{
    wxLogDebug("%s", __func__);
    event.Skip(); // Default behaviour
}

void MyFrame::OnRightButtonUp(wxMouseEvent& event)
{
    wxLogDebug("%s", __func__);
    //wxMenu* editMenu = new wxMenu();
    //editMenu->Append(EDIT_CUT, wxS("C&ut\tCtrl+X"), wxS("Cut to clipboard"));
    //editMenu->Append(EDIT_COPY, wxS("&Copy\tCtrl+C"), wxS("Copy to clipboard"));
    //editMenu->Append(EDIT_PASTE, wxS("&Paste\tCtrl+V"), wxS("Paste from clipboard"));
    //editMenu->Append(EDIT_DELETE, wxS("&Delete\tDel"), wxS("Delete selected items"));
    //editMenu->AppendSeparator();
    //editMenu->Append(EDIT_SELALL, wxS("&Select All\tCtrl+A"), wxS("Select all"));
    //if (article->CanCut())
    //{
    //    editMenu->Enable(EDIT_CUT, true);
    //    editMenu->Enable(EDIT_COPY, true);
    //}
    //PopupMenu(editMenu);
    //event.Skip(false);
    event.Skip();
}

void MyFrame::OnKeyDown(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (uc != WXK_NONE)
    {
        switch (uc)
        {
        case WXK_BACK:
        case WXK_RETURN:
        case WXK_TAB:
            wxLogDebug("OnKeyDown: %s", wxString(uc));
            updateSaveState(false);
            break;
        default:
            break;
        }
        event.Skip(true);
    }
    else // No Unicode equivalent.
    {
        event.Skip(true);
    }
}

void MyFrame::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (uc != WXK_NONE)
    {
        wxLogDebug("OnChar: %s", wxString(uc));
        updateSaveState(false);
        event.Skip(true);
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
    frame->loadFile(filenames[0]);
    return true;
}
