#pragma once
#include "MarkdownParser.h"

//---------------------------------
MemDC* textView = nullptr;
Article article = { Line(std::wstring(L"")) };	// article text
Cursor caret(article);		// current position = end of selection
Cursor sel_begin(article);	// beginning of selection 
bool bSaved = true;			// set false when file is modified
int textView_width = 0, textView_height = 0;
int caret_x, caret_y;		// for IME
int xoffset = 0;			// offset-x of textView
int yoffset = 0;			// offset-y of textView

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

//---------------------------------

// repaint edit area
void repaintView(HDC hdc) {
	textView_width = textView_height = 0;
	for (const Line& l : article)
	{
		int width = 0;
		for (const Character& c : l.sentence)
			width += c.width;
		if (width > textView_width)
			textView_width = width;
		textView_height += l.height + l.padding_top;
	}

	int canvasWidth = 100 + textView_width;
	int canvasHeight = 200 + textView_height;
	int caret_h;

	delete textView;
	textView = new MemDC(hdc, canvasWidth, canvasHeight);

	// fill background
	GDIUtil::fill(*textView, MNP_BGCOLOR_EDIT, 0, 0, canvasWidth, canvasHeight);

	// fill selection background
	int distance_y = sel_begin.distance_y(caret);
	if (distance_y == 0)
	{
		int x, y, width;
		x = y = width = 0;
		
		for (Article::const_iterator a_iter = article.begin();
			a_iter != sel_begin.getSentence();
			++a_iter)
			y += a_iter->height + a_iter->padding_top;
		caret_y = y;
		caret_h = sel_begin.getSentence()->height;
		
		Sentence::const_iterator i = sel_begin.getSentence()->sentence.begin();
		int distance_x = sel_begin.distance_x(caret);
		if (distance_x <= 0)
		{
			for (; i != sel_begin.getCharacter(); ++i)
				x += i->width;
			for (; i != caret.getCharacter(); ++i)
				width += i->width;
			caret_x = x + width;
			if (distance_x == 0)
				goto there;
		}
		else if (distance_x > 0)
		{
			for (; i != caret.getCharacter(); ++i)
				x += i->width;
			caret_x = x;
			for (; i != sel_begin.getCharacter(); ++i)
				width += i->width;
		}
		GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, x, y, width, caret_h);
	}
	else
	{
#define _FROM_ (distance_y < 0 ? sel_begin : caret)
#define _TO_ (distance_y < 0 ? caret : sel_begin)
		int y = 0;
		Article::const_iterator a_iter = article.begin();
		for (; a_iter != _FROM_.getSentence(); ++a_iter)
			y += a_iter->height + a_iter->padding_top;

		// header line
		{
			int x = 0, width = 0;
			Sentence::const_iterator s_iter = a_iter->sentence.begin();
			for (; s_iter != _FROM_.getCharacter(); ++s_iter)
				x += s_iter->width;
			for (; s_iter != a_iter->sentence.end(); ++s_iter)
				width += s_iter->width;

			GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, x, y, width, a_iter->height);

			if (distance_y >= 0)
			{
				caret_x = x;
				caret_y = y;
				caret_h = a_iter->height;
			}
		}
		y += a_iter->height + a_iter->padding_top;

		// middle lines
		for (++a_iter; a_iter != _TO_.getSentence(); ++a_iter)
		{
			int width = 0;
			for (Sentence::const_iterator s_iter = a_iter->sentence.begin();
				s_iter != a_iter->sentence.end();
				++s_iter)
				width += s_iter->width;
			GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, 0, y, width, a_iter->height);
			y += a_iter->height + a_iter->padding_top;
		}

		// tail line
		{
			int width = 0;
			
			for (Sentence::const_iterator s_iter = a_iter->sentence.begin();
				s_iter != _TO_.getCharacter();
				++s_iter)
				width += s_iter->width;

			GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, 0, y, width, a_iter->height);

			if (distance_y < 0)
			{
				caret_x = width;
				caret_y = y;
				caret_h = a_iter->height;
			}
		}
#undef _FROM_
#undef _TO_
	}
there:

	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(*textView);
	size_t y = 0;
	for (const Line& l : article)
	{
		f.printLine(static_cast<std::wstring>(l).c_str(),
			l.sentence.size(),
			l.padding_left,
			y += l.padding_top);
		y += l.height;
	}
	f.unbind();

	// darw caret
	GDIUtil::line(*textView, MNP_FONTCOLOR, caret_x, caret_y, caret_x, caret_y + caret_h);
}

void OnPaint(HDC hdc) {

	RECT rc;
	GetClientRect(hWnd, &rc);
	int ClientWidth = rc.right - rc.left;
	int ClientHeight = rc.bottom - rc.top;

	// create MemDC
	MemDC mdc = MemDC(hdc, rc.right - rc.left, rc.bottom - rc.top);

	// background color of edit area
	GDIUtil::fill(mdc, MNP_BGCOLOR_EDIT, 0, 0, ClientWidth, ClientHeight);

	// paste textView
	if (textView == nullptr) repaintView(mdc);
	BitBlt(mdc, MNP_PADDING_CLIENT, 0, ClientWidth, ClientHeight, *textView, xoffset, yoffset, SRCCOPY);

	// draw vertical scrollbar
	GDIUtil::fill(mdc, MNP_SCROLLBAR_BGCOLOR, ClientWidth - MNP_SCROLLBAR_WIDTH, 0,
		MNP_SCROLLBAR_WIDTH, ClientHeight);
	GDIUtil::fill(mdc, MNP_SCROLLBAR_COLOR,
		ClientWidth - MNP_SCROLLBAR_WIDTH,
		yoffset * ClientHeight / (textView_height + ClientHeight),
		MNP_SCROLLBAR_WIDTH, ClientHeight * ClientHeight / (textView_height + ClientHeight));
	// draw horizontal scrollbar
	if (textView_width > ClientWidth - MNP_PADDING_CLIENT * 2) {
		GDIUtil::fill(mdc, MNP_SCROLLBAR_BGCOLOR, 0, ClientHeight - MNP_SCROLLBAR_WIDTH,
			ClientWidth, MNP_SCROLLBAR_WIDTH);
		GDIUtil::fill(mdc, MNP_SCROLLBAR_COLOR,
			xoffset * ClientWidth / (textView_width + ClientWidth),
			ClientHeight - MNP_SCROLLBAR_WIDTH,
			ClientWidth * ClientWidth / (textView_width + ClientWidth), MNP_SCROLLBAR_WIDTH);
	}

	// display
	BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, mdc, 0, 0, SRCCOPY);
}

// if caret out of user's sight, jump to see the caret
void seeCaret() {
	RECT rc;
	GetClientRect(hWnd, &rc);
	int ClientWidth = rc.right - rc.left;
	int ClientHeight = rc.bottom - rc.top;

	size_t x, y;
	x = y = 0;
	
	// count y
	for (Article::const_iterator a_iter = article.begin();
		a_iter != caret.getSentence();
		++a_iter)
		y += a_iter->height + a_iter->padding_top;

	// count x
	for (Sentence::const_iterator s_iter = caret.getSentence()->sentence.begin();
		s_iter != caret.getCharacter();
		++s_iter)
		x += s_iter->width;

	if (xoffset > x)
		xoffset = x;
	else if (xoffset < x - ClientWidth + MNP_PADDING_CLIENT * 2)
		xoffset = x - ClientWidth + MNP_PADDING_CLIENT * 2;

	if (yoffset > y)
		yoffset = y;
	else if (yoffset < y - ClientHeight + MNP_LINEHEIGHT)
		yoffset = y - ClientHeight + MNP_LINEHEIGHT;
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
	repaintView(hdc);
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
		insertAtCursor( c);
	}
		break;
	default:
		insertAtCursor(nChar);
	}

	bSaved = false;
	sel_begin = caret;

	HDC hdc = GetDC(hWnd);
	repaintView(hdc);

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
	cursor_y = cursor_y + yoffset;
	if (cursor_y < 0)
		cursor_y = 0;
	cursor_x += xoffset - MNP_PADDING_CLIENT;
	if (cursor_x < 0)
		cursor_x = 0;
	
	Article::iterator l;
	int y = 0;
	for (l = article.begin(); l != --article.end(); ++l)
	{
		if ((y += l->height + l->padding_top) >= cursor_y)
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
	repaintView(hdc);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

// repaintView() & OnPaint()
inline void refresh() {
	HDC hdc = GetDC(hWnd);
	repaintView(hdc);
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

inline void OnMouseHWheel(short zDeta, int x, int y) {
	if (textView_width < MNP_FONTSIZE) return;
	xoffset += zDeta / 2;

	if (xoffset < 0)
		xoffset = 0;
	else if (xoffset > textView_width - MNP_FONTSIZE)
		xoffset = textView_width - MNP_FONTSIZE;

	HDC hdc = GetDC(hWnd);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnMouseWheel(short zDeta, int x, int y) {
	if (GetKeyState(VK_SHIFT) < 0) {
		OnMouseHWheel(-zDeta, x, y);
		return;
	}
	yoffset -= zDeta / 2;

	if (yoffset < 0)
		yoffset = 0;
	else if (yoffset > textView_height)
		yoffset = textView_height;

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
	delete textView;
	textView = nullptr;
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
	article.push_back(Line(std::wstring(L"")));
	
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
