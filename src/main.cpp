#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/dir.h>
#include <wx/dnd.h>
#include <wx/colour.h>
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
#include "MyNotePad.xpm"
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

#ifdef USE_NATIVE_EDIT_BOX
#include "imm.h"
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

// case WM_IME_STARTCOMPOSITION:
//     {
//         HIMC hImc = ImmGetContext(hWnd);
//         COMPOSITIONFORM Composition;
//         Composition.dwStyle = CFS_POINT;
//         Composition.ptCurrentPos.x = caret_x + MNP_PADDING_LEFT;    // IME follow
//         Composition.ptCurrentPos.y = caret_y + fontSize / 4;    // set IME position
//         ImmSetCompositionWindow(hImc, &Composition);
//         ImmReleaseContext(hWnd, hImc);
//     }
//     break;
#endif
#endif

#define LOG_MESSAGE(msg) {wxLogDebug(L"%s: %s", __func__, msg);}
#define LOG_ERROR(msg) {wxLogDebug(L"%s: %s", __func__, msg);}

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

#ifndef USE_NATIVE_EDIT_BOX
    EVT_MOTION(MyFrame::OnMouseMove)
    EVT_MOUSEWHEEL(MyFrame::OnMouseWheel)

    EVT_SIZE(MyFrame::OnSize)
    EVT_PAINT(MyFrame::OnPaint)
#endif
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
#ifndef USE_NATIVE_EDIT_BOX
    EVT_MENU(VIEW_LINENUMBER, MyFrame::OnLineNumber)
#endif
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
    bgColorSel = wxColour(0xCC, 0xCC, 0xCC);
    fontColor = wxColour(0x44, 0x44, 0x44);
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
    bgColorSel = wxColour(44, 92, 139);
    fontColor =  *wxWHITE;
    lineNumFontColor = bgColorSel;
    scrollBarBgColor = 0x003E3E3E;
    scrollBarColor = 0x00686868;

    theme = THEME::THEME_DARK;
    GetMenuBar()->Check(VIEW_THEMELIGHT, false);
    GetMenuBar()->Check(VIEW_THEMEDARK, true);
}

void MyFrame::loadSettings()
{
    themeDark();

    GetMenuBar()->Check(VIEW_FONT_MSYAHEI, true);
    GetMenuBar()->Check(FORMAT_WORDWRAP, true);

    confFilePath = wxStandardPaths::Get().GetUserConfigDir();

#ifdef _WIN32
    confFilePath += '\\';
#else
    confFilePath += '/';
#endif
    confFilePath += MNP_APPNAME_C;
    if (!wxDir::Exists(confFilePath)) {
        if (!wxDir::Make(confFilePath)) {
            LOG_ERROR(L"Failed to create config directory");
        }
    }
#ifdef _WIN32
    confFilePath += '\\';
#else
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
            const char* pos = strchr(buff, '=');    // eg. best = 999
            if (pos != NULL)
            {
                std::string key(buff, pos - buff);    // acquire key
                std::string val(pos + 1);            // acquire value
                if (val.back() == '\r') val.pop_back();    // remove '\r' at the end
                if (key == MNP_CONFIG_THEME && std::atoi(val.c_str()) == THEME::THEME_DARK)
                    themeDark();
                else if (key == MNP_CONFIG_WORDWRAP && val == "0")
                {
                    bWordWrap = false;
                    GetMenuBar()->Check(FORMAT_WORDWRAP, false);
                }
#ifndef USE_NATIVE_EDIT_BOX
                else if (key == MNP_CONFIG_LINENUMBER && val == "0")
                {
                    bShowLineNumber = false;
                    GetMenuBar()->Check(VIEW_LINENUMBER, false);
                }
#endif
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
                        GetMenuBar()->Check(VIEW_FONT_LUCIDA, true);
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
                }
            }
        }
    }
#ifdef USE_NATIVE_EDIT_BOX
 	wxFont font(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fontFace);
    wxTextAttr at(fontColor, bgColorEdit, font);
    at.SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    article->SetDefaultStyle(at);
    article->SetStyle(0, article->GetValue().size(), at);
#else
        // set default line_number_font
//         line_number_font = std::make_unique<Font>(
//             lineNumSize, MNP_LINENUM_FONTFACE, lineNumFontColor);
//
//         HDC hdc = GetDC(hWnd);
//         for (auto& l : article)
//         {
//             l.background_color = bgColorEdit;
//             repaintLine(hdc, l);
//         }
//         ReleaseDC(hWnd, hdc);
#endif
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
#ifndef USE_NATIVE_EDIT_BOX
    out << MNP_CONFIG_LINENUMBER << '=' << bShowLineNumber << "\r\n";
#endif
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    out << MNP_CONFIG_FONTNAME << '=' << cvt.to_bytes(fontFace);
    out.close();
    LOG_MESSAGE(pathname);
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

    bSaved = true;
    openedFile = path;
    SetTitle((openedFile + MNP_DOC_TITLE).c_str());

#ifdef USE_NATIVE_EDIT_BOX
    article->SetValue(str);
#else
//     // clean
//     article.clear();
//     article.push_back(Line());
//
//     // set text
//     caret.reset();
//     insertAtCursor(str);
//
//     // init
//     sel_begin.reset();
//     caret.reset();
//
//     if (bWordWrap)
//     {
//         caret.split_all_long_text(EditAreaWidth);
//         sel_begin = caret;
//     }
//
//     HDC hdc = GetDC(hWnd);
//     for (auto& l : article)
//         repaintLine(hdc, l);
//
//     calc_textView_height();
//
//     seeCaret();
//
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
}

/*
 * return true if the user decide to quit
 */
bool MyFrame::sureToQuit(wxCommandEvent& event)
{
    if (!bSaved)
    {
        int r = wxMessageBox(INFO_SAVE, MNP_APPNAME, wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
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

    f << R"raw(<!DOCTYPE><html><head>
<meta charset="utf-8"/>
<link href="style.css" rel="stylesheet">
<link href="highlight.css" rel="stylesheet">
<script type="text/javascript" src="highlight.pack.js"></script>
<script type="text/javascript">
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
#ifdef USE_NATIVE_EDIT_BOX
    return article->GetValue().ToStdWstring();
#else
    std::wstring str;
    for (Article::const_iterator l = article.begin(); ;)
    {
        str += *l;
        if (++l != article.end())
        {
            if (!l->child_line)
                str.push_back(L'\n');
        }
        else
            break;
    }
    return str;
#endif
}

#ifndef USE_NATIVE_EDIT_BOX
// Cursor MyFrame::PosToCaret(int cursor_x, int cursor_y)
// {
//     if (cursor_y < 0)
//         cursor_y = 0;
//     if (cursor_x < 0)
//         cursor_x = 0;
//     cursor_y += yoffset;
//     if (cursor_y < 0)
//         cursor_y = 0;
//     cursor_x += xoffset - MNP_PADDING_LEFT;
//     if (cursor_x < 0)
//         cursor_x = 0;
//
//     Article::iterator l;
//     int y = 0;
//     for (l = article.begin(); l != --article.end(); ++l)
//     {
//         if ((y += l->text_height + l->padding_top) >= cursor_y)
//             break;
//     }
//
//     Sentence::iterator c;
//     int x = l->padding_left;
//     for (c = l->sentence.begin(); c != l->sentence.end(); ++c)
//     {
//         if ((x += c->width) >= cursor_x + c->width/2)
//             break;
//     }
//
//     if (bWordWrap && c == l->sentence.begin() && l->child_line)
//     {
//         --l;
//         c = l->sentence.end();
//     }
//
//     return Cursor(article, l, c);
// }
//
// void MyFrame::repaintModifiedLine(HDC& hdc)
// {
//     if (bWordWrap)
//     {
//         caret.rebond();
//         Article::const_iterator a = caret.getSentence();
//         Article::const_iterator b = caret.split_long_text(EditAreaWidth);
//         sel_begin = caret;
//         for (auto i = a; i != b; ++i)
//             repaintLine(hdc, *i);
//     }
//     else
//         repaintLine(hdc, *caret.getSentence());
// }

// void MyFrame::insertAtCursor(const TCHAR& c)
// {
//     if (c == L'\n')
//     {
//         caret.insertCharacter(Character(L'\n'));
//     }
//     else if (c == L'\t')
//     {
//         Character tab(L' ');
//
//         // set tab to 4 spaces' width
//         HDC hdc = GetDC(hWnd);
//         Font f(fontSize, fontFace, fontColor);
//         f.bind(hdc);
//         GetCharWidth32W(hdc, tab.c, tab.c, &tab.width);
//         f.unbind();
//         ReleaseDC(hWnd, hdc);
//
//         tab.c = L'\t';
//         tab.width *= 4;
//         caret.insertCharacter(tab);
//     }
//     else
//     {
//         // emoji
//         if (c > 0xD800 && c < 0xDBFF)
//         {
//             two_utf16_encoded_chars = c;
//             return;
//         }
//         if (two_utf16_encoded_chars != 0)
//         {
//             if (c > 0xDC00 && c < 0xDFFF)
//             {
//                 Character ch((two_utf16_encoded_chars << 16) | c);
//                 two_utf16_encoded_chars = 0;
//                 ch.width = 20;    // emoji char width
//                 caret.insertCharacter(ch);
//             }
//             else
//             {
//                 two_utf16_encoded_chars = 0;
//                 wxMessageBox(ERR_FAULT_ENCODING, MNP_APPNAME, wxOK | wxICON_INFORMATION, this);
//             }
//         }
//         else
//         {
//             Character ch(c);
//             HDC hdc = GetDC(hWnd);
//             Font f(fontSize, fontFace, fontColor);
//             f.bind(hdc);
//             GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
//             f.unbind();
//             ReleaseDC(hWnd, hdc);
//             caret.insertCharacter(ch);
//         }
//     }
//}

// void MyFrame::insertAtCursor(const std::wstring& str)
// {
//     HDC hdc = GetDC(hWnd);
//     Font f(fontSize, fontFace, fontColor);
//     f.bind(hdc);
//
//     for (auto i = str.begin(); i != str.end(); ++i)
//     {
//         if (*i == L'\r')
//         {
//             ;
//         }
//         else if (*i == L'\t')
//         {
//             // convert tab to 4 spaces' width
//             Character ch(L' ');
//             GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
//             ch.c = L'\t';
//             ch.width *= 4;
//             caret.insertCharacter(ch);
//         }
//         else if (*i == L'\n')
//         {
//             insertAtCursor(L'\n');
//         }
//         else
//         {
//             TCHAR c = *i;
//
//             // emoji
//             if (c > 0xD800 && c < 0xDBFF)
//             {
//                 two_utf16_encoded_chars = c;
//                 continue;
//             }
//             if (two_utf16_encoded_chars != 0)
//             {
//                 if (c > 0xDC00 && c < 0xDFFF)
//                 {
//                     Character ch((two_utf16_encoded_chars << 16) | c);
//                     two_utf16_encoded_chars = 0;
//                     ch.width = 20;    // emoji char width
//                     caret.insertCharacter(ch);
//                 }
//                 else
//                 {
//                     two_utf16_encoded_chars = 0;
//                     wxMessageBox(ERR_FAULT_ENCODING, MNP_APPNAME, wxOK | wxICON_WARNING, this);
//                 }
//             }
//             else
//             {
//                 Character ch(c);
//                 GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
//                 caret.insertCharacter(ch);
//             }
//         }
//     }
//
//     f.unbind();
//     ReleaseDC(hWnd, hdc);
// }

// update MemDC, text_width, text_height for the line
// and increase textView_width (only if text_width is greater)
// void MyFrame::repaintLine(HDC clientDC, const Line& l, bool whole_line_selected = false)
// {
//     // calc width of each line
//     l.text_width = 0;
//     for (Sentence::const_iterator i = l.sentence.begin(); i != l.sentence.end(); ++i)
//         l.text_width += i->width;
//     if (!bWordWrap && l.text_width + l.padding_left > textView_width)
//         textView_width = l.text_width + l.padding_left;
//
//     // discard present MemDC and recreate
//     l.mdc = std::make_unique<MemDC>(clientDC,
//         l.text_width + l.padding_left,
//         l.text_height + l.padding_top);
//
//     // fill background
//     if (whole_line_selected)
//         GDIUtil::fill(*l.mdc,
//             bgColorSel,
//             0,
//             0,
//             l.text_width + l.padding_left,
//             l.text_height + l.padding_top);
//     else
//         GDIUtil::fill(*l.mdc,
//             l.background_color,
//             0,
//             0,
//             l.text_width + l.padding_left,
//             l.text_height + l.padding_top);
//
//     // draw text
//     Font f(fontSize, fontFace, fontColor);
//     f.bind(*l.mdc);
//     f.printLine(l, l.padding_left, l.padding_top);
//     f.unbind();
// }

// update MemDC, text_width, text_height for the line
// (only if some text selected in this line)
// void MyFrame::repaintLine(HDC clientDC, const Line& l,
//     const Sentence::const_iterator sel_from,
//     const Sentence::const_iterator sel_to)
// {
//     // make sure sel_from and sel_to are in the same line
//     size_t x_sel_from = 0, x_sel_to = 0;
//
//     // calc width of each line
//     l.text_width = 0;
//     for (Sentence::const_iterator i = l.sentence.begin(); i != l.sentence.end(); ++i)
//     {
//         if (i == sel_from)    // if sel_to equals sentence.end(),
//             x_sel_from = l.text_width;    // x_sel_from will not be assigned a value
//         if (i == sel_to)    // if sel_to equals sentence.end(),
//             x_sel_to = l.text_width;    // x_sel_to will not be assigned a value
//         l.text_width += i->width;
//     }
//     if (!bWordWrap && l.text_width + l.padding_left > textView_width)
//         textView_width = l.text_width + l.padding_left;
//     if (l.sentence.end() == sel_from)
//         x_sel_from = l.text_width;
//     if (l.sentence.end() == sel_to)
//         x_sel_to = l.text_width;
//
//     // discard present MemDC and recreate
//     l.mdc = std::make_unique<MemDC>(clientDC,
//         l.text_width + l.padding_left,
//         l.text_height + l.padding_top);
//
//     // fill background
//     GDIUtil::fill(*l.mdc,
//         l.background_color,
//         0,
//         0,
//         l.text_width + l.padding_left,
//         l.text_height + l.padding_top);
//
//     // fill selection background
//     GDIUtil::fill(*l.mdc,
//         bgColorSel,
//         l.padding_left + x_sel_from,
//         l.padding_top,
//         x_sel_to - x_sel_from,
//         l.text_height);
//
//     // draw text
//     Font f(fontSize, fontFace, fontColor);
//     f.bind(*l.mdc);
//     f.printLine(l, l.padding_left, l.padding_top);
//     f.unbind();
// }

// When text was modified, call this function to recalculate textView_height
// void MyFrame::calc_textView_height()
// {
//     textView_height = 0;
//     for (const Line& i : article)
//         textView_height += i.text_height + i.padding_top;
//
//     // textView_width will be updated when repaintLine() or OnSize() is called
// }

// fill selection background
// void MyFrame::repaintSelectedLines()
// {
//     if (sel_begin.getSentence() != caret.getSentence())
//     {
//         HDC hdc = GetDC(hWnd);
//         // repaint selected lines
//         int dist_y = sel_begin.distance_y(caret);
//         if (dist_y < 0)    // forward selection
//         {
//             // repaint first line
//             Article::const_iterator l = sel_begin.getSentence();
//             repaintLine(hdc, *l, sel_begin.getCharacter(), l->sentence.end());
//             // repaint each line
//             for (++l; l != caret.getSentence(); ++l)
//                 repaintLine(hdc, *l, true);
//             // repaint last line
//             repaintLine(hdc, *l, l->sentence.begin(), caret.getCharacter());
//         }
//         else    // backwards selection
//         {
//             // repaint first line
//             Article::const_iterator l = caret.getSentence();
//             repaintLine(hdc, *l, caret.getCharacter(), l->sentence.end());
//             // repaint each line
//             for (++l; l != sel_begin.getSentence(); ++l)
//                 repaintLine(hdc, *l, true);
//             // repaint last line
//             repaintLine(hdc, *l, l->sentence.begin(), sel_begin.getCharacter());
//         }
//         ReleaseDC(hWnd, hdc);
//     }
//     else if (sel_begin.getCharacter() != caret.getCharacter())
//     {
//         HDC hdc = GetDC(hWnd);
//         int dist_x = sel_begin.distance_x(caret);
//         if (dist_x < 0)
//             repaintLine(hdc, *caret.getSentence(), sel_begin.getCharacter(), caret.getCharacter());
//         else
//             repaintLine(hdc, *caret.getSentence(), caret.getCharacter(), sel_begin.getCharacter());
//         ReleaseDC(hWnd, hdc);
//     }
// }

// if caret is out of client area, jump there
// void MyFrame::seeCaret()
// {
//     size_t x = 0, y = 0;
//
//     // count y
//     for (Article::const_iterator a_iter = article.begin();
//         a_iter != caret.getSentence();
//         ++a_iter)
//         y += a_iter->text_height + a_iter->padding_top;
//
//     if (y < yoffset)    // over the client area
//         yoffset = y;
//     else if (y + lineHeight > yoffset + EditAreaHeight)    // below the client area
//         yoffset = y + lineHeight - EditAreaHeight;
//
//     if (bWordWrap)
//         xoffset = 0;
//     else
//     {
//         // count x
//         for (Sentence::const_iterator s_iter = caret.getSentence()->sentence.begin();
//             s_iter != caret.getCharacter();
//             ++s_iter)
//             x += s_iter->width;
//
//         if (x < xoffset)    // on the left of client area
//             xoffset = x;
//         else if (x > xoffset + EditAreaWidth)    // on the right of client area
//             xoffset = x - EditAreaWidth;
//     }
// }

// remove selection background
// void MyFrame::repaintSelectionCanceledLines()
// {
//     // repaint selected lines to remove selection background
//     if (sel_begin.getSentence() != caret.getSentence())
//     {
//         HDC hdc = GetDC(hWnd);
//         // repaint selected lines
//         int dist_y = sel_begin.distance_y(caret);
//         if (dist_y < 0)    // forward selection
//         {
//             for (Article::const_iterator l = sel_begin.getSentence();
//                 l != ++caret.getSentence(); ++l)
//                 repaintLine(hdc, *l);
//         }
//         else    // backwards selection
//         {
//             for (Article::const_iterator l = caret.getSentence();
//                 l != ++sel_begin.getSentence(); ++l)
//                 repaintLine(hdc, *l);
//         }
//         ReleaseDC(hWnd, hdc);
//     }
//     else
//     {
//         HDC hdc = GetDC(hWnd);
//         repaintLine(hdc, *caret.getSentence());
//         ReleaseDC(hWnd, hdc);
//     }
// }
#endif /* USE_NATIVE_EDIT_BOX */

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
#ifdef USE_NATIVE_EDIT_BOX
    article(nullptr),
#else
    article({ Line() }),
    caret(article),
    sel_begin(article),
    bShowLineNumber(true),
#endif
    bResized(false),
    bSaved(true),
    bWordWrap(true),
    fontSize(28),
    lineNumSize(23),
    lineHeight(28),
    fontFace(FONT_MSYAHEI),
    openedFile(MNP_DOC_NOTITLE)
{
    SetDropTarget(this);
    SetIcon(wxICON(ICON_MYNOTEPAD));

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
    editMenu->Enable(EDIT_UNDO, false);
    editMenu->Append(EDIT_REDO, L"&Redo\tCtrl+Y", L"Redo");
    editMenu->Enable(EDIT_REDO, false);
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
#ifndef USE_NATIVE_EDIT_BOX
    viewMenu->AppendCheckItem(VIEW_LINENUMBER, L"&Line Number", L"Show line number");
#endif
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
#ifdef USE_NATIVE_EDIT_BOX
    formatMenu->Enable(FORMAT_WORDWRAP, false);
#endif

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
#ifdef USE_NATIVE_EDIT_BOX
    wxBoxSizer *boxSizer = new wxBoxSizer( wxVERTICAL );
    article = new wxTextCtrl(this, wxID_ANY, "", wxPoint(0, 0), wxSize(10, 10), wxTE_MULTILINE | wxTE_BESTWRAP);
    boxSizer->Add(article, 1, wxEXPAND | wxALL, 0);
    this->SetSizer(boxSizer);
    article->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MyFrame::OnRightButtonDown), NULL, this);
    //article->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(MyFrame::OnRightButtonUp), NULL, this);
    //article->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(MyFrame::OnKeyDown), NULL, this);
    //article->Connect(wxEVT_KEY_UP, wxKeyEventHandler(MyFrame::OnKeyUp), NULL, this);
    article->Connect(wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnChar), NULL, this);
#else
    // TODO: ADD clientDC
#endif
    Centre();
}

bool MyFrame::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
    wxCommandEvent evt;
    sureToQuit(evt);
    loadFile(filenames[0].ToStdString());
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
        f.close();
        bSaved = true;
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
    std::ofstream f(filepath);
    if (!f.good())
    {
        wxMessageBox(ERR_OPEN_FAIL, MNP_APPNAME, wxOK | wxICON_ERROR, this);
        return;
    }
    std::wstring str = all_to_string();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    f << cvt.to_bytes(str);
    f.close();

    openedFile = filepath;

    SetTitle((openedFile + MNP_DOC_TITLE).c_str());
    bSaved = true;
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
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnRedo(wxCommandEvent& WXUNUSED(event))
{
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnCut(wxCommandEvent& event)
{
    LOG_MESSAGE("");
#ifdef USE_NATIVE_EDIT_BOX
    if (article->CanCut())
    {
        OnCopy(event);
        article->RemoveSelection();
        bSaved = false;
    }
#else
    OnCopy(event);
//     if (sel_begin.removeSelectedChars(caret))
//     {
//         bSaved = false;
//         HDC hdc = GetDC(hWnd);
//
//         repaintModifiedLine(hdc);
//
//         calc_textView_height();
//
//         OnPaint(hdc);
//         ReleaseDC(hWnd, hdc);
//     }
#endif
}

void MyFrame::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
#ifdef USE_NATIVE_EDIT_BOX
    wxString str;
    if (article->CanCopy())
        str = article->GetStringSelection();
    if (str.IsEmpty())
        return;
#else
    std::wstring str = sel_begin.getSelectedChars(caret);
    if (str.empty())
        return;
#endif
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
    bSaved = false;
#ifdef USE_NATIVE_EDIT_BOX
    article->Paste();
#else
    const wchar_t* str;
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( wxDF_TEXT ))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData( data );
            str = data.GetText().c_str();
        }
        wxTheClipboard->Close();
    }
//     sel_begin.removeSelectedChars(caret);
//
//     insertAtCursor(str);
//
//     sel_begin = caret;
//     if (bWordWrap)
//     {
//         caret.rebond_all();
//         caret.split_all_long_text(EditAreaWidth);
//         sel_begin = caret;
//     }
//
//     HDC hdc = GetDC(hWnd);
//     for (auto& l : article)
//         repaintLine(hdc, l);
//
//     calc_textView_height();
//
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
}

void MyFrame::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
#ifdef USE_NATIVE_EDIT_BOX
    if (article->CanCut())
    {
        bSaved = false;
        article->RemoveSelection();
    }
#else
//     bool flag_ShiftPressed = (GetKeyState(WXK_SHIFT) < 0);
//     if (sel_begin.removeSelectedChars(caret))
//     {
//         bSaved = false;
//
//         HDC hdc = GetDC(hWnd);
//
//         repaintModifiedLine(hdc);
//
//         calc_textView_height();
//         seeCaret();
//
//         OnPaint(hdc);
//         ReleaseDC(hWnd, hdc);
//         return;
//     }
//     else
//     {
//         // delete the character
//         if (!caret.eraseChar())
//             return;
//         bSaved = false;
//
//         HDC hdc = GetDC(hWnd);
//
//         sel_begin = caret;
//         repaintModifiedLine(hdc);
//
//         calc_textView_height();
//         seeCaret();
//
//         OnPaint(hdc);
//         ReleaseDC(hWnd, hdc);
//         return;
//     }
//     if (flag_ShiftPressed)
//     {
//         if (caret == sel_begin)
//             repaintSelectionCanceledLines();
//         else
//             repaintSelectedLines();    // paint selection background
//     }
//     else
//         sel_begin = caret;
//
//     seeCaret();
//
//     HDC hdc = GetDC(hWnd);
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
}

void MyFrame::OnSelAll(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
#ifdef USE_NATIVE_EDIT_BOX
    article->SelectAll();
#else
//     sel_begin.reset();
//     caret.end();
//     repaintSelectedLines();
//
//     HDC hdc = GetDC(hWnd);
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
}

void MyFrame::OnOptions(wxCommandEvent& WXUNUSED(event))
{
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnMenuOpen(wxMenuEvent& event)
{
    wxMenu *menu = event.GetMenu();
    wxMenuItem *item = menu->FindItem(EDIT_CUT);
    if (item != NULL)
    {
#ifdef USE_NATIVE_EDIT_BOX
        item->Enable(article->CanCut());
#else
        item->Enable(caret != sel_begin);
#endif
    }
    item = menu->FindItem(EDIT_COPY);
    if (item != NULL)
    {
#ifdef USE_NATIVE_EDIT_BOX
        item->Enable(article->CanCut());
#else
        item->Enable(caret != sel_begin);
#endif
    }
    item = menu->FindItem(EDIT_DELETE);
    if (item != NULL)
    {
#ifdef USE_NATIVE_EDIT_BOX
        item->Enable(article->CanCut());
#else
        item->Enable(caret != sel_begin);
#endif
    }
}

void MyFrame::OnThemeLight(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    themeLight();
#ifdef USE_NATIVE_EDIT_BOX
    wxTextAttr at = article->GetDefaultStyle();
    at.SetTextColour(fontColor);
    at.SetBackgroundColour(bgColorEdit);
    at.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    article->SetDefaultStyle(at);
    article->SetStyle(0, article->GetValue().size(), at);
#else
//         if (bShowLineNumber)
//         line_number_font = std::make_unique<Font>(
//             lineNumSize, MNP_LINENUM_FONTFACE, lineNumFontColor);
//
//     HDC hdc = GetDC(hWnd);
//     for (auto& l : article)
//     {
//         l.background_color = bgColorEdit;
//         repaintLine(hdc, l);
//     }
//     repaintSelectedLines();
//
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
    saveSettings();
}

void MyFrame::OnThemeDark(wxCommandEvent& WXUNUSED(event))
{
    LOG_MESSAGE("");
    themeDark();
#ifdef USE_NATIVE_EDIT_BOX
    wxTextAttr at = article->GetDefaultStyle();
    at.SetTextColour(fontColor);
    at.SetBackgroundColour(bgColorEdit);
    at.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    article->SetDefaultStyle(at);
    article->SetStyle(0, article->GetValue().size(), at);
#else
//     if (bShowLineNumber)
//     line_number_font = std::make_unique<Font>(
//         lineNumSize, MNP_LINENUM_FONTFACE, lineNumFontColor);
//
//     HDC hdc = GetDC(hWnd);
//     for (auto& l : article)
//     {
//         l.background_color = bgColorEdit;
//         repaintLine(hdc, l);
//     }
//     repaintSelectedLines();
//
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
   saveSettings();
}

#ifndef USE_NATIVE_EDIT_BOX
void MyFrame::OnLineNumber(wxCommandEvent& WXUNUSED(event))
{
    if (GetMenuBar()->IsChecked(VIEW_LINENUMBER)) {
        LOG_MESSAGE("turn on line number");
    } else {
        LOG_MESSAGE("turn off line number");
    }

//     bShowLineNumber = !bShowLineNumber;
//     HMENU hMenu = GetMenu(hWnd);
//     CheckMenuItem(hMenu, IDM_LINENUMBER, bShowLineNumber ? MF_CHECKED : MF_UNCHECKED);
//
//     if (bShowLineNumber)
//         line_number_font = std::make_unique<Font>(
//             lineNumSize, MNP_LINENUM_FONTFACE, lineNumFontColor);
//     else
//         MNP_PADDING_LEFT = 20;
//
//     HDC hdc = GetDC(hWnd);
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);

    saveSettings();
}
#endif

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

#ifdef USE_NATIVE_EDIT_BOX
    ;
#else
//         HDC hdc = GetDC(hWnd);
//     Font f(fontSize, fontFace, fontColor);
//     f.bind(hdc);
//     // since font face has been changed, we have to update character width
//     for (auto& l : article)
//     {
//         for (Character& c : l.sentence)
//             GetCharWidth32W(hdc, c.c, c.c, &c.width);
//     }
//     f.unbind();
//     // force to adjust line-width
//     resized = true;
//
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
    saveSettings();
}

void MyFrame::OnFontSelect(wxCommandEvent& WXUNUSED(event))
{
    // TODO: NOT IMPLEMENTED
}

void MyFrame::OnBrowser(wxCommandEvent& WXUNUSED(event))
{
    std::string s = confFilePath + "preview.html";
    saveHTML(s);
    LOG_MESSAGE(s);
    wxLaunchDefaultApplication(s);
}

void MyFrame::OnWordWrap(wxCommandEvent& WXUNUSED(event))
{
    if (GetMenuBar()->IsChecked(FORMAT_WORDWRAP)) {
        LOG_MESSAGE("turn on word wrap");
    } else {
        LOG_MESSAGE("turn off word wrap");
    }
#ifdef USE_NATIVE_EDIT_BOX
    ;
#else
//     HDC hdc = GetDC(hWnd);
//     if (bWordWrap)
//     {
//         // exit word wrap mode
//         if (sel_begin == caret)
//         {
//             caret.rebond_all();
//             sel_begin = caret;
//             for (auto& l : article)
//                 repaintLine(hdc, l);
//         }
//         else
//         {
//             int dist_y = sel_begin.distance_y(caret);
//             if (dist_y < 0)    // forward selection
//                 sel_begin.recover(caret);
//             else if (dist_y > 0)
//                 caret.recover(sel_begin);
//             else
//             {
//                 int dist_x = sel_begin.distance_x(caret);
//                 if (dist_x < 0)    // forward selection
//                     sel_begin.recover(caret);
//                 else
//                     caret.recover(sel_begin);
//             }
//             for (auto& l : article)
//                 repaintLine(hdc, l);
//             repaintSelectedLines();
//         }
//     }
//     else
//     {
//         // enter word wrap mode
//         xoffset = 0;
//         resized = true;    // rebond() and split() in OnPaint()
//     }
//     bWordWrap = !bWordWrap;
//     HMENU hMenu = GetMenu(hWnd);
//     CheckMenuItem(hMenu, IDM_WORDWRAP, bWordWrap ? MF_CHECKED : MF_UNCHECKED);
//
//     calc_textView_height();
//     OnPaint(hdc);
//     ReleaseDC(hWnd, hdc);
#endif
    saveSettings();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    std::wstringstream detail;
    detail << MNP_APPNAME << '\t' << MNP_VERSION_MAJOR << '.' << MNP_VERSION_MINOR << '\n'
           << wxVERSION_STRING << '\n'
           << L"\nCopyright(c) moooc.cc 2019";

    wxMessageBox(detail.str(), "About", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnLeftButtonDown(wxMouseEvent& event)
{
    LOG_MESSAGE("");
#ifndef USE_NATIVE_EDIT_BOX
//     bool flag_ShiftPressed = (GetKeyState(WXK_SHIFT) < 0);
//     mouse_down = true;
//     SetCapture(hWnd);
//     repaintSelectionCanceledLines();
//
//     if (flag_ShiftPressed)
//     {
//         caret = PosToCaret(x, y);
//         if (caret != sel_begin)
//             repaintSelectedLines();
//     }
//     else
//         sel_begin = caret = PosToCaret(x, y);
//
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
#endif
}

/*
 * Double-clicking the left mouse button
 * actually generates a sequence of four messages:
 * WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, and WM_LBUTTONUP.
 * Only windows that have the CS_DBLCLKS style
 * can receive WM_LBUTTONDBLCLK messages.
 */
void MyFrame::OnLeftButtonDBClick(wxMouseEvent& event)
{
    LOG_MESSAGE("");
#ifndef USE_NATIVE_EDIT_BOX
//     bool flag_ShiftPressed = (GetKeyState(WXK_SHIFT) < 0);
//     if (flag_ShiftPressed) {
//         OnLButtonDown(wParam, x, y);
//         return;
//     }
//     repaintSelectionCanceledLines();
//     sel_begin = caret = PosToCaret(x, y);
//
//     if (!caret.getSentence()->sentence.empty())
//     {
//         if (caret.isLastChar())
//             sel_begin.move_left();
//         else
//             sel_begin.move_right();
//         repaintSelectedLines();
//     }
//
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
#endif
}

void MyFrame::OnLeftButtonUp(wxMouseEvent& event)
{
    LOG_MESSAGE("");
#ifndef USE_NATIVE_EDIT_BOX
//         if (mouse_down)
//         ReleaseCapture();
//     mouse_down = false;
#endif
}

void MyFrame::OnRightButtonDown(wxMouseEvent& event)
{
    wxMenu *editMenu = new wxMenu();
    editMenu->Append(EDIT_CUT, L"C&ut\tCtrl+X", L"Cut to clipboard");
    editMenu->Append(EDIT_COPY, L"&Copy\tCtrl+C", L"Copy to clipboard");
    editMenu->Append(EDIT_PASTE, L"&Paste\tCtrl+V", L"Paste from clipboard");
    editMenu->Append(EDIT_DELETE, L"&Delete\tDel", L"Delete selected items");
    editMenu->AppendSeparator();
    editMenu->Append(EDIT_SELALL, L"&Select All\tCtrl+A", L"Select all");
#ifdef USE_NATIVE_EDIT_BOX
    if (article->CanCut())
    {
        editMenu->Enable(EDIT_CUT, false);
        editMenu->Enable(EDIT_COPY, false);
        editMenu->Enable(EDIT_DELETE, false);
    }
    event.Skip(false);
#else
    if (sel_begin == caret)
    {
        editMenu->Enable(EDIT_CUT, false);
        editMenu->Enable(EDIT_COPY, false);
        editMenu->Enable(EDIT_DELETE, false);
    }
#endif
    PopupMenu(editMenu);
}

void MyFrame::OnRightButtonUp(wxMouseEvent& event)
{
    return;
}

#ifndef USE_NATIVE_EDIT_BOX
void MyFrame::OnMouseMove(wxMouseEvent& event)
{
//         if (mouse_down) // wParam & MK_LBUTTON
//     {
//         Cursor new_caret = PosToCaret(x, y);
//         if (caret != new_caret)
//         {
//             repaintSelectionCanceledLines();
//             caret = new_caret;
//             repaintSelectedLines();
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
//         }
//     }
}

void MyFrame::OnMouseWheel(wxMouseEvent& event)
{
    if (event.ShiftDown())
    {
        if (bWordWrap)
            return;

        // no scroll bar
//     if (textView_width < EditAreaWidth && xoffset == 0)
//     {
//         if (xoffset != 0)
//         {
//             xoffset = 0;
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
//         }
//         return;
//     }
//     xoffset += zDeta / 2;
//
//     if ((int)xoffset < 0)
//         xoffset = 0;
//     else if (xoffset + EditAreaWidth > textView_width)
//         xoffset = textView_width - EditAreaWidth;
//
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
    }
    else
    {
//         if (!bWordWrap && GetKeyState(WXK_SHIFT) < 0) {
//         OnMouseHWheel(-zDeta, x, y);
//         return;
//     }
//
//     // no scroll bar
//     if (textView_height < EditAreaHeight)
//     {
//         if (yoffset != 0)
//         {
//             yoffset = 0;
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
//         }
//         return;
//     }
//     yoffset -= zDeta / 2;
//
//     if ((int)yoffset < 0)
//         yoffset = 0;
//     else if (yoffset + EditAreaHeight > textView_height)
//         yoffset = textView_height - EditAreaHeight;
//
//                 HDC hdc = GetDC(hWnd);
//                 OnPaint(hdc);
//                 ReleaseDC(hWnd, hdc);
    }
}
#endif /* USE_NATIVE_EDIT_BOX */

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
#ifndef USE_NATIVE_EDIT_BOX
            // replace selected characters
            bool flag_removed = sel_begin.removeSelectedChars(caret);
#endif
            switch (uc)
            {
            case WXK_BACK:
#ifdef USE_NATIVE_EDIT_BOX
                if (article->IsEmpty())
                    return;
#else
                if (!flag_removed && !caret.backspace())
                    return;
#endif
                break;
            case WXK_RETURN:
#ifndef USE_NATIVE_EDIT_BOX
                insertAtCursor(L'\n');
                hdc = GetDC(hWnd);
                repaintLine(hdc, *(--caret.getSentence()));
#endif
                break;
            default:
#ifndef USE_NATIVE_EDIT_BOX
                insertAtCursor(event.GetKeyCode());
                hdc = GetDC(hWnd);
#endif
                break;
            }
            bSaved = false;
#ifdef USE_NATIVE_EDIT_BOX
            event.Skip(true);
#else
            // sel_begin = caret;
            // repaintModifiedLine(hdc);

            // calc_textView_height();
            // seeCaret();

            // OnPaint(hdc);
            // ReleaseDC(hWnd, hdc);
#endif
        }
        else
        {
            // It's a control character, eg. WXK_ESCAPE, WXK_F1, ...
#ifdef USE_NATIVE_EDIT_BOX
            event.Skip(true);
            switch (event.GetKeyCode())
            {
            case WXK_CONTROL_X:
            case WXK_CONTROL_V:
                bSaved = false;
                return;
            default:
                break;
            }
#else
            switch (event.GetKeyCode())
            {
            case WXK_CONTROL_X:
                OnCut(wxCommandEvent());
                return;
            case WXK_CONTROL_V:
                OnPaste(wxCommandEvent());
                return;
            default:
                break;
            }
#endif
        }
    }
    else // No Unicode equivalent.
    {
#ifdef USE_NATIVE_EDIT_BOX
        event.Skip(true);
#else
        bool flag_ShiftPressed = event.ShiftDown();
        // It's a special key, deal with all the known ones:
        switch ( event.GetKeyCode() )
        {
        case WXK_LEFT:
            // // some lines were selected, but Shift was not pressed,
            // // remove selection background
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // caret.move_left();
            break;
        case WXK_RIGHT:
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // caret.move_right();
            break;
        case WXK_UP:
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // // the first line
            // if (caret.isFirstLine())
            // {
            //     HDC hdc = GetDC(hWnd);
            //     OnPaint(hdc);
            //     ReleaseDC(hWnd, hdc);
            //     return;    // nothing to do
            // }
            // else
            // {
            //     // not the first line, go to previous line
            //     caret.toFirstChar();
            //     caret.move_left();
            // }
            break;
        case WXK_DOWN:
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // // the last char in this line, go to the front of next line
            // if (caret.isLastLine())
            // {
            //         HDC hdc = GetDC(hWnd);
            //         OnPaint(hdc);
            //         ReleaseDC(hWnd, hdc);
            //     return;    // nothing to do
            // }
            // else
            // {
            //     // not the last line, go to next line
            //     caret.toLastChar();
            //     caret.move_right();
            // }
            break;
        case WXK_HOME:
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // if (GetKeyState(WXK_CONTROL) < 0)    // Ctrl was pressed
            // {
            //     if (caret.isFirstLine() && caret.isFirstChar())
            //     {
            //         HDC hdc = GetDC(hWnd);
            //         OnPaint(hdc);
            //         ReleaseDC(hWnd, hdc);
            //         return;
            //     }
            //     else
            //         caret.reset();
            // }
            // else
            // {
            //     if (!caret.isFirstChar())
            //         caret.toFirstChar();
            //     else
            //         return;
            // }
            break;
        case WXK_END:
            // if (!flag_ShiftPressed)
            //     repaintSelectionCanceledLines();
            // if (GetKeyState(WXK_CONTROL) < 0)    // Ctrl was pressed
            // {
            //     if (caret.isLastLine() && caret.isLastChar())
            //     {
            //         HDC hdc = GetDC(hWnd);
            //         OnPaint(hdc);
            //         ReleaseDC(hWnd, hdc);
            //         return;
            //     }
            //     else
            //         caret.end();
            // }
            // else
            // {
            //     if (!caret.isLastChar())
            //         caret.toLastChar();
            //     else
            //     {
            //         HDC hdc = GetDC(hWnd);
            //         OnPaint(hdc);
            //         ReleaseDC(hWnd, hdc);
            //         return;
            //     }
            // }
            break;
        case WXK_DELETE:
            // if (sel_begin.removeSelectedChars(caret))
            // {
            //     bSaved = false;

            //     HDC hdc = GetDC(hWnd);

            //     repaintModifiedLine(hdc);

            //     calc_textView_height();
            //     seeCaret();

            //     OnPaint(hdc);
            //     ReleaseDC(hWnd, hdc);
            //     return;
            // }
            // else
            // {
            //     // delete the character
            //     if (!caret.eraseChar())
            //         return;
            //     bSaved = false;

            //     HDC hdc = GetDC(hWnd);

            //     sel_begin = caret;
            //     repaintModifiedLine(hdc);

            //     calc_textView_height();
            //     seeCaret();

            //     OnPaint(hdc);
            //     ReleaseDC(hWnd, hdc);
            //     return;
            // }
            break;
        default:
            break
        }
        // if (flag_ShiftPressed)
        // {
        //     if (caret == sel_begin)
        //         repaintSelectionCanceledLines();
        //     else
        //         repaintSelectedLines();    // paint selection background
        // }
        // else
        //     sel_begin = caret;

        // seeCaret();

        // HDC hdc = GetDC(hWnd);
        // OnPaint(hdc);
        // ReleaseDC(hWnd, hdc);
#endif
    }
}

#ifndef USE_NATIVE_EDIT_BOX
void MyFrame::OnSize(wxSizeEvent& event)
{
//     ClientWidth = width;
//     ClientHeight = height;
//     EditAreaWidth = ClientWidth - MNP_PADDING_LEFT - MNP_SCROLLBAR_WIDTH;
//     EditAreaHeight = ClientHeight - MNP_SCROLLBAR_WIDTH;
//     bResized = true;
//     if (bWordWrap)
//         textView_width = EditAreaWidth;
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
//     // Window size changed so we should adjust text.
//     if (bWordWrap && bResized)
//     {
//         // for selection
//         if (sel_begin == caret)
//         {
//             caret.rebond_all();
//             caret.split_all_long_text(EditAreaWidth);
//
//             sel_begin = caret;
//             for (auto& l : article)
//                 repaintLine(hdc, l);
//         }
//         else
//         {
//             int dist_y = sel_begin.distance_y(caret);
//             if (dist_y < 0)    // forward selection
//                 sel_begin.reform(EditAreaWidth, caret);
//             else if (dist_y > 0)
//                 caret.reform(EditAreaWidth, sel_begin);
//             else
//             {
//                 int dist_x = sel_begin.distance_x(caret);
//                 if (dist_x < 0)    // forward selection
//                     sel_begin.reform(EditAreaWidth, caret);
//                 else
//                     caret.reform(EditAreaWidth, sel_begin);
//             }
//
//             for (auto& l : article)
//                 repaintLine(hdc, l);
//             repaintSelectedLines();
//         }
//         calc_textView_height();
//     }
//     resized = false;
//
//     // create clientDC
//     MemDC clientDC(hdc, ClientWidth, ClientHeight);
//
//     // background color of edit area
//     GDIUtil::fill(clientDC, bgColorEdit, 0, 0, ClientWidth, ClientHeight);
//
//     size_t y = 0;
//     Article::const_iterator l = article.begin();
//     size_t n_line = article.size();    // a counter for line number
//     if (bShowLineNumber)
//     {
//         line_number_font->bind(clientDC);
//         if (n_line > 999)
//             MNP_PADDING_LEFT = 64;
//         else if (n_line > 99)
//             MNP_PADDING_LEFT = 50;
//         else if (n_line > 9)
//             MNP_PADDING_LEFT = 38;
//         else
//             MNP_PADDING_LEFT = 26;
//         n_line = 0;
//     }
//     // draw first line (probably can't be fully seen)
//     for (; l != article.end(); ++l)
//     {
//         y += l->text_height + l->padding_top;
//         if (bShowLineNumber && !l->child_line)
//             ++n_line;
//         // the first line of edit area
//         if (y > yoffset)
//         {
//             if (bShowLineNumber)
//             {
//                 std::wstringstream ss; ss << n_line;
//                 line_number_font->printLine(ss.str(),
//                     MNP_LINENUM_MARGIN_LEFT,
//                     MNP_LINENUM_MARGIN_TOP + (y - l->text_height - l->padding_top) - yoffset);
//             }
//             BitBlt(clientDC,
//                 MNP_PADDING_LEFT,    // dest x
//                 0,                    // dest y
//                 l->text_width + l->padding_left,    // dest width
//                 y - yoffset,        // dest height
//                 *(l->mdc),
//                 xoffset,            // source x
//                 yoffset - (y - l->text_height - l->padding_top),    // source y
//                 SRCCOPY);
//             // caret is in this line
//             if (caret.getSentence() == l)
//             {
//                 caret_x = l->padding_left;    // calc caret_x
//                 for (Sentence::const_iterator c = l->sentence.begin();
//                     c != caret.getCharacter(); ++c)
//                 {
//                     caret_x += c->width;
//                     // caret at the right of edit area
//                     if (caret_x > xoffset + EditAreaWidth)
//                         break;
//                 }
//                 // caret is in edit area, draw caret
//                 if (caret_x >= xoffset && caret_x <= xoffset + EditAreaWidth)
//                 {
//                     caret_x -= xoffset;
//                     caret_y = (y - l->text_height) - yoffset;
//                     GDIUtil::line(clientDC, fontColor, caret_x + MNP_PADDING_LEFT, caret_y,
//                         caret_x + MNP_PADDING_LEFT, caret_y + caret.getSentence()->text_height);
//                 }
//             }
//             ++l;
//             break;
//         }
//     }
//     y -= yoffset;    // Y-Axis to paint the second line
//     // draw subsequent lines
//     for (; l != article.end(); ++l)
//     {
//         if (bShowLineNumber)
//         {
//             if (!l->child_line)
//                 ++n_line;
//             std::wstringstream ss; ss << n_line;
//             line_number_font->printLine(ss.str(),
//                 MNP_LINENUM_MARGIN_LEFT,
//                 MNP_LINENUM_MARGIN_TOP + y);
//         }
//         if (y > EditAreaHeight)    // last line (can't be seen entirely)
//         {
//             BitBlt(clientDC,
//                 MNP_PADDING_LEFT,    // dest x
//                 y,                    // dest y
//                 l->text_width + l->padding_left,    // dest width
//                 EditAreaHeight - (y - l->text_height - l->padding_top),    // dest height
//                 *(l->mdc),
//                 xoffset,            // source x
//                 0,                    // source y
//                 SRCCOPY);
//             break;
//         }
//         BitBlt(clientDC,
//             MNP_PADDING_LEFT,        // dest x
//             y,                        // dest y
//             l->text_width + l->padding_left,    // dest width
//             l->text_height + l->padding_top,    // dest height
//             *(l->mdc),
//             xoffset,                // source x
//             0,                        // source y
//             SRCCOPY);
//         // caret at this line
//         if (caret.getSentence() == l)
//         {
//             caret_x = l->padding_left;
//             for (Sentence::const_iterator c = l->sentence.begin();
//                 c != caret.getCharacter(); ++c)
//             {
//                 caret_x += c->width;
//                 // caret at the right of edit area
//                 if (caret_x > xoffset + EditAreaWidth)
//                     break;
//             }
//             // caret is in edit area, draw caret
//             if (caret_x >= xoffset && caret_x <= xoffset + EditAreaWidth)
//             {
//                 caret_x -= xoffset;
//                 caret_y = y + caret.getSentence()->padding_top;
//                 GDIUtil::line(clientDC, fontColor, caret_x + MNP_PADDING_LEFT, caret_y,
//                     caret_x + MNP_PADDING_LEFT, caret_y + caret.getSentence()->text_height);
//             }
//         }
//         y += l->text_height + l->padding_top;
//     }
//
//     if (bShowLineNumber)
//         line_number_font->unbind();
//
//     // draw vertical scrollbar
//     if (textView_height > ClientHeight - MNP_PADDING_LEFT) {
//         GDIUtil::fill(clientDC,
//             scrollBarBgColor,
//             ClientWidth - MNP_SCROLLBAR_WIDTH,
//             0,
//             MNP_SCROLLBAR_WIDTH,
//             ClientHeight
//         );
//         GDIUtil::fill(clientDC,
//             scrollBarColor,
//             ClientWidth - MNP_SCROLLBAR_WIDTH,
//             yoffset * ClientHeight / textView_height,
//             MNP_SCROLLBAR_WIDTH,
//             ClientHeight * ClientHeight / textView_height
//         );
//     }
//     // draw horizontal scrollbar
//     if (!bWordWrap && textView_width > ClientWidth - MNP_PADDING_LEFT) {
//         GDIUtil::fill(clientDC,
//             scrollBarBgColor,
//             0,
//             ClientHeight - MNP_SCROLLBAR_WIDTH,
//             ClientWidth,
//             MNP_SCROLLBAR_WIDTH
//         );
//         GDIUtil::fill(clientDC,
//             scrollBarColor,
//             xoffset * ClientWidth / textView_width,
//             ClientHeight - MNP_SCROLLBAR_WIDTH,
//             ClientWidth * ClientWidth / textView_width,
//             MNP_SCROLLBAR_WIDTH
//         );
//     }
//
//     // display
//     BitBlt(hdc, 0, 0, ClientWidth, ClientHeight, clientDC, 0, 0, SRCCOPY);
}
#endif /* USE_NATIVE_EDIT_BOX */
