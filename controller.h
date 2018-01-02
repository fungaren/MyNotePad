#pragma once
#include "MarkdownParser.h"

//---------------------------------
Article article = { Line(L"") };				// article text
Cursor caret(article);							// current position = end of selection
Cursor sel_begin(article);						// beginning of selection 
bool bSaved = true;								// set false when file is modified
unsigned int textView_width = 0, textView_height = 0;
unsigned int caret_x, caret_y;					// for IME (relative to EditArea)
unsigned int xoffset = 0;		// offset-x of textView
unsigned int yoffset = 0;		// offset-y of textView

//---------------------------------

void insertAtCursor(const TCHAR& c)
{
	Character ch(c);
	if (c != '\n')
	{
		HDC hdc = GetDC(hWnd);
		Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
		f.bind(hdc);
		GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
		f.unbind();
		ReleaseDC(hWnd, hdc);
	}
	caret.insertCharacter(ch);
}

void insertAtCursor(const std::wstring& str)
{
	HDC hdc = GetDC(hWnd);
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(hdc);

	for (auto i = str.begin(); i != str.end(); ++i)
	{
		if (*i == L'\r')
		{
			;
		}
		else if (*i == L'\t')
		{
			Character ch(L' ');
			GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
			// convert tab to space
			caret.insertCharacter(ch);
			caret.insertCharacter(ch);
			caret.insertCharacter(ch);
			caret.insertCharacter(ch);
		}
		else if (*i == L'\n')
		{
			insertAtCursor(L'\n');
		}
		else
		{
			Character ch(*i);
			GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
			caret.insertCharacter(ch);
		}
	}

	f.unbind();
	ReleaseDC(hWnd, hdc);
}

void repaintLine(HDC clientDC, const Line& l,
	const Sentence::const_iterator sel_from,
	const Sentence::const_iterator sel_to)
{
	size_t x_sel_from = 0, x_sel_to = 0;

	// calc width of each line
	l.text_width = 0;
	for (Sentence::const_iterator i = l.sentence.begin(); i != l.sentence.end(); ++i)
	{
		if (i == sel_from)
			x_sel_from = l.text_width;
		if (i == sel_to)
			x_sel_to = l.text_width;
		l.text_width += i->width;
	}
	if (l.text_width + l.padding_left > textView_width)
		textView_width = l.text_width + l.padding_left;

	// calc textView height
	textView_height = 0;
	for (const Line& i : article)
		textView_height += i.text_height + i.padding_top;

	// discard present MemDC and recreate
	l.mdc = std::make_unique<MemDC>(clientDC, l.text_width + l.padding_left, l.text_height + l.padding_top);

	// fill background
	GDIUtil::fill(*l.mdc, MNP_BGCOLOR_EDIT, 0, 0, l.text_width + l.padding_left, l.text_height + l.padding_top);

	// fill selection background
	GDIUtil::fill(*l.mdc, MNP_BGCOLOR_SEL, l.padding_left, l.padding_top, x_sel_to - x_sel_from, l.text_height);

	// draw text
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(*l.mdc);
	f.printLine(static_cast<std::wstring>(l).c_str(), l.sentence.size(), l.padding_left, l.padding_top);
	f.unbind();
}

void OnPaint(HDC hdc) {

	RECT rc;
	GetClientRect(hWnd, &rc);
	size_t ClientWidth = rc.right - rc.left;
	size_t ClientHeight = rc.bottom - rc.top;
	size_t EditAreaWidth = ClientWidth - MNP_PADDING_CLIENT - MNP_SCROLLBAR_WIDTH;
	size_t EditAreaHeight = ClientHeight - MNP_SCROLLBAR_WIDTH;

	// create clientDC
	MemDC clientDC(hdc, ClientWidth, ClientHeight);

	// background color of edit area
	GDIUtil::fill(clientDC, MNP_BGCOLOR_EDIT, 0, 0, ClientWidth, ClientHeight);

	// paste lines can be seen to mdc
	size_t y = 0;
	Article::const_iterator l = article.begin();
	// draw first line (probably can't be seen entirely)
	for (; l != article.end(); ++l)
	{
		y += l->text_height + l->padding_top;
		if (y > yoffset)	// first line in edit area
		{
			BitBlt(clientDC,
				MNP_PADDING_CLIENT,	// dest x
				0,					// dest y
				l->text_width + l->padding_left,	// dest width
				y - yoffset,		// dest height
				*(l->mdc),
				xoffset,			// source x
				yoffset - (y - l->text_height - l->padding_top),	// source y
				SRCCOPY);
			// caret is in this line
			if (caret.getSentence() == l)
			{
				caret_x = 0;	// calc caret_x
				for (Sentence::const_iterator c = l->sentence.begin();
					c != caret.getCharacter(); ++c)
				{
					caret_x += c->width;
					// caret at the right of edit area
					if (caret_x > xoffset + EditAreaWidth)
						break;
				}
				// caret is in edit area, draw caret
				if (caret_x >= xoffset && caret_x <= xoffset + EditAreaWidth)
				{
					caret_x -= xoffset;
					caret_y = (y - l->text_height - l->padding_top) - yoffset;
					GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_CLIENT, caret_y,
						caret_x + MNP_PADDING_CLIENT, caret_y + caret.getSentence()->text_height
						+ caret.getSentence()->padding_top);
				}
			}
			++l;
			break;
		}
	}
	y -= yoffset;	// Y-Axis to paint the second line 
	// draw medium lines
	for (; l != article.end(); ++l)
	{
		if (y > EditAreaHeight)	// last line (can't be seen entirely) 
		{
			BitBlt(clientDC,
				MNP_PADDING_CLIENT,	// dest x
				y,					// dest y
				l->text_width + l->padding_left,	// dest width
				EditAreaHeight - (y - l->text_height - l->padding_top),	// dest height
				*(l->mdc),
				xoffset,			// source x
				0,					// source y
				SRCCOPY);
			break;
		}
		BitBlt(clientDC,
			MNP_PADDING_CLIENT,		// dest x
			y,						// dest y
			l->text_width + l->padding_left,	// dest width
			l->text_height + l->padding_top,	// dest height
			*(l->mdc),
			xoffset,				// source x
			0,						// source y
			SRCCOPY);
		// caret at this line
		if (caret.getSentence() == l)
		{
			caret_x = 0;
			for (Sentence::const_iterator c = l->sentence.begin();
				c != caret.getCharacter(); ++c)
			{
				caret_x += c->width;
				// caret at the right of edit area
				if (caret_x > xoffset + EditAreaWidth)
					break;
			}
			// caret is in edit area, draw caret
			if (caret_x >= xoffset && caret_x <= xoffset + EditAreaWidth)
			{
				caret_x -= xoffset;
				caret_y = y;
				GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_CLIENT, caret_y,
					caret_x + MNP_PADDING_CLIENT, caret_y + caret.getSentence()->text_height
					+ caret.getSentence()->padding_top);
			}
		}
		y += l->text_height + l->padding_top;
	}

	// draw vertical scrollbar
	if (textView_height > ClientHeight - MNP_PADDING_CLIENT) {
		GDIUtil::fill(clientDC,
			MNP_SCROLLBAR_BGCOLOR,
			ClientWidth - MNP_SCROLLBAR_WIDTH,
			0,
			MNP_SCROLLBAR_WIDTH,
			ClientHeight
		);
		GDIUtil::fill(clientDC,
			MNP_SCROLLBAR_COLOR,
			ClientWidth - MNP_SCROLLBAR_WIDTH,
			yoffset * ClientHeight / textView_height,
			MNP_SCROLLBAR_WIDTH,
			ClientHeight * ClientHeight / textView_height
		);
	}
	// draw horizontal scrollbar
	if (textView_width > ClientWidth - MNP_PADDING_CLIENT) {
		GDIUtil::fill(clientDC,
			MNP_SCROLLBAR_BGCOLOR,
			0,
			ClientHeight - MNP_SCROLLBAR_WIDTH,
			ClientWidth,
			MNP_SCROLLBAR_WIDTH
		);
		GDIUtil::fill(clientDC,
			MNP_SCROLLBAR_COLOR,
			xoffset * ClientWidth / textView_width,
			ClientHeight - MNP_SCROLLBAR_WIDTH,
			ClientWidth * ClientWidth / textView_width,
			MNP_SCROLLBAR_WIDTH
		);
	}

	// display
	BitBlt(hdc, 0, 0, ClientWidth, ClientHeight, clientDC, 0, 0, SRCCOPY);
}

// if caret is out of client area, jump there
void seeCaret() {
	RECT rc;
	GetClientRect(hWnd, &rc);
	size_t EditAreaWidth = rc.right - rc.left - MNP_PADDING_CLIENT - MNP_SCROLLBAR_WIDTH;
	size_t EditAreaHeight = rc.bottom - rc.top - MNP_SCROLLBAR_WIDTH;

	size_t x = 0, y = 0;

	// count y
	for (Article::const_iterator a_iter = article.begin();
		a_iter != caret.getSentence();
		++a_iter)
		y += a_iter->text_height + a_iter->padding_top;

	// count x
	for (Sentence::const_iterator s_iter = caret.getSentence()->sentence.begin();
		s_iter != caret.getCharacter();
		++s_iter)
		x += s_iter->width;

	if (y < yoffset)	// over the client area
		yoffset = y;
	else if (y + MNP_LINEHEIGHT > yoffset + EditAreaHeight)	// below the client area
		yoffset = y + MNP_LINEHEIGHT - EditAreaHeight;

	if (x < xoffset)	// on the left of client area
		xoffset = x;
	else if (x > xoffset + EditAreaWidth)	// on the right of client area
		xoffset = x - EditAreaWidth;
}

void OnKeyDown(int nChar) {
	switch (nChar)
	{
	case VK_DELETE:
		if (sel_begin.removeSelectedChars(caret))
			bSaved = false;
		else if (caret.getSentence() != --article.end()
			|| caret.getCharacter() != caret.getSentence()->sentence.end())
		{
			// delete the character
			caret.eraseChar();
			bSaved = false;
		}
		else return;	// end of all, nothing to do
		break;
	case VK_LEFT:
		caret.move_left();
		break;
	case VK_RIGHT:
		caret.move_right();
		break;
	case VK_UP:
		// the first line
		if (caret.getSentence() == article.begin())
			return;	// nothing to do
		else
		{
			// not the first line, go to previous line
			caret.toFirstChar();
			caret.move_left();
		}
		break;
	case VK_DOWN:
		// the last char in this line, go to the front of next line
		if (caret.getSentence() == --article.end())
			return;	// nothing to do
		else
		{
			// not the last line, go to next line
			caret.toLastChar();
			caret.move_right();
		}
		break;
	case VK_HOME:
		if (GetKeyState(VK_CONTROL) < 0)
		{
			if (caret.getSentence() == article.begin() 
				&& caret.getCharacter() == caret.getSentence()->sentence.begin())
				return;
			else
				caret.reset();
		}
		else
		{
			if (caret.getCharacter() != caret.getSentence()->sentence.begin())
				caret.toFirstChar();
			else
				return;
		}
		break;
	case VK_END:
	{
		if (GetKeyState(VK_CONTROL) < 0)
		{
			if (caret.getSentence() == --article.end()
				&& caret.getCharacter() == caret.getSentence()->sentence.end())
				return;
			else
				caret.end();
		}
		else
		{
			if (caret.getCharacter() != caret.getSentence()->sentence.end())
				caret.toLastChar();
			else
				return;
		}
	}
		break;
	default:
		return;
	};

	if (GetKeyState(VK_SHIFT) >= 0)
		sel_begin = caret;

	HDC hdc = GetDC(hWnd);
	// repaint each line and calc textView area
	for (auto& l : article)
		repaintLine(hdc, l, l.sentence.begin(), l.sentence.end());
	
	seeCaret();

	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnChar(WORD nChar)
{
	// replace selected characters
	bool flag_removed = sel_begin.removeSelectedChars(caret);
	if (flag_removed)
		bSaved = false;
	switch (nChar)
	{
	case VK_BACK:
		if (!flag_removed)
			caret.backspace();
		break;
	case VK_RETURN:
		insertAtCursor(L'\n');
		break;
	case VK_TAB:
	{
		Character c(L' ');
		insertAtCursor(c);		// convert tab to space
		insertAtCursor(c);
		insertAtCursor(c);
		insertAtCursor(c);
	}
		break;
	default:
		insertAtCursor(nChar);
	}

	bSaved = false;
	sel_begin = caret;

	HDC hdc = GetDC(hWnd);
	if (nChar == VK_BACK || nChar == VK_RETURN)
	{
		// repaint each line and calc textView area
		for (auto& l : article)
			repaintLine(hdc, l, l.sentence.begin(), l.sentence.end());
	}
	else
		repaintLine(hdc,
			*caret.getSentence(),
			caret.getSentence()->sentence.begin(),
			caret.getSentence()->sentence.end()
		);
	seeCaret();

	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnKeyUp(int nChar)
{
	;
}

Cursor PosToCaret(int cursor_x, int cursor_y)
{
	cursor_y += yoffset;
	if (cursor_y < 0)
		cursor_y = 0;
	cursor_x += xoffset - MNP_PADDING_CLIENT;
	if (cursor_x < 0)
		cursor_x = 0;
	
	Article::iterator l;
	int y = 0;
	for (l = article.begin(); l != --article.end(); ++l)
	{
		if ((y += l->text_height + l->padding_top) >= cursor_y)
			break;
	}

	Sentence::iterator c;
	int x = 0;
	for (c = l->sentence.begin(); c != l->sentence.end(); ++c)
	{
		if ((x += c->width) >= cursor_x + c->width/2)
			break;
	}

	return Cursor(article, l, c);
}

inline void OnLButtonDown(DWORD wParam, int x, int y) {
	HDC hdc = GetDC(hWnd);
	sel_begin = caret = PosToCaret(x, y);
	// repaint each line and calc textView area
	for (auto& l : article)
		repaintLine(hdc, l, l.sentence.begin(), l.sentence.end());
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

// repaintLine() and OnPaint()
inline void refresh() {
	HDC hdc = GetDC(hWnd);
	// repaint each line and calc textView area
	for (auto& l : article)
		repaintLine(hdc, l, l.sentence.begin(), l.sentence.end());
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnMouseMove(DWORD wParam, int x, int y) {
	if (wParam & MK_LBUTTON)
	{
		Cursor new_caret = PosToCaret(x, y);
		if (caret != new_caret)
		{
			caret = new_caret;
			refresh();
		}
	}
}

inline void OnRButtonDown(DWORD wParam, int x, int y) {
	POINT p = { x,y };
	ClientToScreen(hWnd, &p);
	HMENU hm = LoadMenu(hInst, MAKEINTRESOURCE(IDC_POPUP));

	TrackPopupMenu(GetSubMenu(hm, 0), TPM_LEFTALIGN, p.x, p.y, NULL, hWnd, NULL);
	DestroyMenu(hm);
}

void OnMouseHWheel(short zDeta, int x, int y) {
	RECT rc;
	GetClientRect(hWnd, &rc);
	size_t EditAreaWidth = rc.right - rc.left - MNP_PADDING_CLIENT - MNP_SCROLLBAR_WIDTH;

	if (textView_width < EditAreaWidth) return;
	xoffset += zDeta / 2;

	if ((int)xoffset < 0)
		xoffset = 0;
	else if (xoffset + EditAreaWidth > textView_width)
		xoffset = textView_width - EditAreaWidth;

	HDC hdc = GetDC(hWnd);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

void OnMouseWheel(short zDeta, int x, int y) {
	if (GetKeyState(VK_SHIFT) < 0) {
		OnMouseHWheel(-zDeta, x, y);
		return;
	}
	RECT rc;
	GetClientRect(hWnd, &rc);
	size_t EditAreaHeight = rc.bottom - rc.top - MNP_SCROLLBAR_WIDTH;

	if (textView_height < EditAreaHeight) return;
	yoffset -= zDeta / 2;

	if ((int)yoffset < 0)
		yoffset = 0;
	else if (yoffset + EditAreaHeight > textView_height)
		yoffset = textView_height - EditAreaHeight;

	HDC hdc = GetDC(hWnd);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

#include <shellapi.h>
inline void OnMenuNew() {
	ShellExecuteW(hWnd, L"Open", L"MyNotePad.exe", NULL, NULL, SW_SHOWNORMAL);
}

#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <locale>
#include <codecvt>
std::wstring opened_file = L"Untitled";	// full path

inline void OnMenuCopyHtml() {
	if (!OpenClipboard(hWnd)) return;
	if (!EmptyClipboard()) {
		CloseClipboard();
		return;
	}

	// !important

	// to HTML
	std::wstring str = L"\n";
	for (const Line& l : article)
		str += static_cast<std::wstring>(l) + L"\n";
	str += L'\n';
	parse_markdown(str);

	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
	LPTSTR p = static_cast<LPTSTR>(GlobalLock(h));
	for (TCHAR& c : str) {
		*p = c;
		++p;
	}
	*p = '\0';
	
	GlobalUnlock(h);
	SetClipboardData(CF_UNICODETEXT, h);
	CloseClipboard();
}

inline void setOFN(OPENFILENAME& ofn, LPCTSTR lpstrFilter, LPCTSTR lpstrDefExt) {
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = lpstrFilter;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = lpstrDefExt;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
}

inline void OnMenuExport() {
	TCHAR file[MAX_PATH];		*file = '\0';

	OPENFILENAME ofn;
	ofn.lpstrFile = file;
	setOFN(ofn, L"HTML (*.html)\0*.html\0All Files (*.*)\0*.*\0\0", L"html");

	if (GetSaveFileNameW(&ofn) > 0) {
		// write to file
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::ofstream f(file);
		f << "<!DOCTYPE><head><meta charset=\"utf-8\"/><head><body>";
		std::wstring str = L"\n";
		for (const Line& l : article)
			str += static_cast<std::wstring>(l) + L"\n";
		str += L'\n';
		parse_markdown(str);
		f << cvt.to_bytes(str);
		f << "</body>";
		f.close();
	}
}

void OnMenuSaveAs() {
	TCHAR file[MAX_PATH];	*file = '\0';

	OPENFILENAME ofn;
	ofn.lpstrFile = file;
	setOFN(ofn, L"MarkDown (*.md)\0*.md\0All Files (*.*)\0*.*\0\0", L"md");

	if (GetSaveFileNameW(&ofn) > 0) {
		std::wstring str;
		for (const Line& l : article)
			str += l;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::ofstream f(file);
		f << cvt.to_bytes(str);
		f.close();

		opened_file = file;
		SetWindowTextW(hWnd, (opened_file + L" - MyNotePad").c_str());
		bSaved = true;
	}
}

inline void OnMenuSave() {
	if (opened_file == L"Untitled")
		OnMenuSaveAs();
	else
	{
		std::wstring str;
		for (const Line& l : article)
			str += l;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::ofstream f(opened_file);
		f << cvt.to_bytes(str);
		f.close();
		bSaved = true;
	}
}

// true: quit, false: cancel
bool sureToQuit() {
	if (!bSaved)
	{
		int r = MessageBoxW(hWnd, L"Save changes?", MNP_APPNAME, MB_YESNOCANCEL | MB_ICONQUESTION);
		if (r == IDCANCEL)
			return false;
		else if (r == IDYES)
		{
			OnMenuSave();
			return false;
		}
	}
	return true;
}

void loadFile(LPTSTR path) {
	// remove quote
	if (*path == '\"') {
		path[lstrlenW(path) - 1] = '\0';
		++path;
	}
	// load file
	std::ifstream f(path);
	if (f.fail()) {
		std::wstringstream ss;
		ss << L"Fail to load \"" << path << L"\".";
		MessageBoxW(hWnd, ss.str().c_str(), MNP_APPNAME, MB_OK | MB_ICONWARNING);
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
		MessageBoxW(hWnd, L"Can only open UTF-8 file!", MNP_APPNAME, MB_OK | MB_ICONWARNING);
		return;
	}
	
	// clean
	article.clear();
	article.push_back(Line(L""));
	
	caret.reset();
	insertAtCursor(str);

	sel_begin.reset();
	caret.reset();
	bSaved = true;

	opened_file = path;
	SetWindowTextW(hWnd, (opened_file + L" - MyNotePad").c_str());
	bSaved = true;

	refresh();
}

inline void OnMenuOpen() {
	sureToQuit();
	TCHAR file[MAX_PATH];		*file = '\0';
	OPENFILENAME ofn;
	ofn.lpstrFile = file;
	setOFN(ofn, L"MarkDown (*.md)\0*.md\0All Files (*.*)\0*.*\0\0", L"md");

	if (GetOpenFileNameW(&ofn) > 0)
		loadFile(file);
}

inline void OnMenuUndo() {

}

inline void OnMenuRedo() {

}

inline void OnMenuCopy() {
	if (sel_begin == caret) return;
	if (!OpenClipboard(hWnd)) return;
	if (!EmptyClipboard()) {
		CloseClipboard();
		return;
	}

	HGLOBAL h;
	std::wstring str = sel_begin.getSelectedChars(caret);
	h = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
	if (NULL == h) return;
	LPTSTR p = static_cast<LPTSTR>(GlobalLock(h));
	for (TCHAR& c : str)
		*(p++) = c;
	*p = L'\0';

	GlobalUnlock(h);
	SetClipboardData(CF_UNICODETEXT, h);
	CloseClipboard();
}

inline void OnMenuCut() {
	OnMenuCopy();
	if (sel_begin.removeSelectedChars(caret))
		bSaved = false;
	refresh();
}

inline void OnMenuPaste() {

	if (!OpenClipboard(hWnd)) return;

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	if (NULL == h) {
		CloseClipboard();
		return;
	}

	if (sel_begin.removeSelectedChars(caret))
		bSaved = false;
	LPCTSTR str = (LPCTSTR)GlobalLock(h);
	insertAtCursor(str);
	sel_begin = caret;
	GlobalUnlock(h);
	CloseClipboard();

	refresh();
}

inline void OnMenuAll() {
	sel_begin.reset();
	caret.end();
	refresh();
}
