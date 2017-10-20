#pragma once
#include "MarkdownParser.h"

//---------------------------------
MemDC* textView = nullptr;
Article article;			// article text
Cursor caret(0, 0);			// current position = end of selection
Cursor sel_begin(0, 0);		// beginning of selection 

int textView_width = 0;
int textView_height = MNP_LINEHEIGHT;
int caret_x, caret_y;		// for ime
int xoffset = 0;			// offset-x of textView
int yoffset = 0;			// offset-y of textView

//---------------------------------

// return true for success, false for no selected char.
bool removeSelectedChars() {
	if (sel_begin.n_line == caret.n_line)
	{
		if (sel_begin.n_character == caret.n_character)
			return false;
		else
			caret.eraseInLine(article, sel_begin.n_character);
	}
	else if (sel_begin.n_line < caret.n_line)	// forward selection
	{
		std::wstring str = sel_begin.subStringToEnd(article) +
			caret.subStringToStarting(article);
		article.erase(sel_begin.getSentence(article), caret.getSentence(article));
		caret.n_line = sel_begin.n_line;
		article.insert(caret.getSentence(article), Line(str));
	}
	else	// backward selection
	{
		std::wstring str = caret.subStringToEnd(article) +
			sel_begin.subStringToStarting(article);
		article.erase(caret.getSentence(article), sel_begin.getSentence(article));
		sel_begin.n_line = caret.n_line;
		article.insert(caret.getSentence(article), Line(str));
	}
	bSaved = false;
	return true;
}

// return the selected characters
std::wstring getSelectedChars() {
	if (sel_begin.n_line == caret.n_line)
	{
		if (sel_begin.n_character == caret.n_character)
			return L"";
		else
			return caret.subString(article, sel_begin.n_character);
	}
	else if (sel_begin.n_line < caret.n_line)	// forward selection
	{
		std::wstring str = sel_begin.subStringToEnd(article);
		for (auto& i = sel_begin.getSentence(article);
			i != caret.getSentence(article);)
			str.append(i->sentence.begin(), i->sentence.end());
		str += caret.subStringToStarting(article);
		return str;
	}
	else	// backward selection
	{
		std::wstring str = caret.subStringToEnd(article);
		for (auto& i = caret.getSentence(article);
			i != sel_begin.getSentence(article);)
			str.append(i->sentence.begin(), i->sentence.end());
		str += sel_begin.subStringToStarting(article);
		return str;
	}
}

void insertAtCursor(Article& a, const TCHAR& c)
{
	Sentence& s = caret.getSentence(a)->sentence;
	Sentence::iterator i = s.begin();
	std::advance(i, caret.n_character);
	Character ch(c);
	s.insert(i, ch);
	++caret.n_character;
	if (c == L'\n')
	{
		if (i == s.end())
		{
			++caret.n_line;
			caret.n_character = 0;
			article.insert(caret.getSentence(article), Line(std::wstring(L"")));
		}
		else
		{
			std::wstring t_str = caret.subStringToEnd(article);
			caret.eraseToEnd(article);
			++caret.n_line;
			caret.n_character = 0;
			article.insert(caret.getSentence(article), Line(t_str));
		}
	}
}

void insertAtCursor(Article& a, const std::wstring& str)
{
	Sentence& s = caret.getSentence(a)->sentence;
	Sentence::iterator i_sentence = s.begin();
	std::advance(i_sentence, caret.n_character);
	for (auto i_str = str.begin(); i_str != str.end(); ++i_str)
	{
		if (*i_str == L'\r')
		{
			;
		}
		else if (*i_str == L'\t')
		{
			Character c(L' ');
			Sentence t_sentence;
			t_sentence.push_back(c);		// convert tab to space
			t_sentence.push_back(c);
			t_sentence.push_back(c);
			t_sentence.push_back(c);
			s.insert(i_sentence, t_sentence.begin(), t_sentence.end());
			caret.n_character += 4;
		}
		else if (*i_str == L'\n')
		{
			insertAtCursor(article, L'\n');
		}
		else
		{
			s.insert(i_sentence, Character(*i_str));
			++caret.n_character;
		}
	}
}

//---------------------------------

// repaint edit area
void repaintView(HDC hdc) {
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(hdc);
	for (Line& l : article)
	{
		int line_width = 0;
		for (Character& c : l.sentence)
		{
			GetCharWidth32W(hdc, c.c, c.c, &c.width);
			if ((line_width += c.width) > textView_width)
				textView_width = line_width;
		}
		textView_height += l.height + l.padding_top;
	}
	f.unbind();

	int canvasWidth = 500 + textView_width;
	int canvasHeight = 500 + textView_height;
	int caret_h;

	delete textView;
	textView = new MemDC(hdc, canvasWidth, canvasHeight);
	f.bind(*textView);

	// fill background
	GDIUtil::fill(*textView, MNP_BGCOLOR_EDIT, 0, 0, canvasWidth, canvasHeight);

	// fill selection background
	if (sel_begin.n_line == caret.n_line)
	{
		int x, y, width;
		x = y = width = 0;
		Article::const_iterator a_iter = article.begin();
		for (int i = 0; i != sel_begin.n_line; ++i, ++a_iter)
			y += a_iter->height + a_iter->padding_top;
		caret_y = y;
		caret_h = a_iter->height;
		
		Sentence::const_iterator s_iter;
		if (!a_iter->sentence.empty())
			s_iter = a_iter->sentence.begin();
		if (sel_begin.n_character <= caret.n_character)
		{
			int i = 0;
			for (; i < sel_begin.n_character; ++i, ++s_iter)
				x += s_iter->width;
			for (; i < caret.n_character; ++i, ++s_iter)
				width += s_iter->width;
			caret_x = x + width;
			if (sel_begin.n_character == caret.n_character)
				goto there;
		}
		else if (sel_begin.n_character > caret.n_character)
		{
			int i = 0;
			for (; i < caret.n_character; ++i, ++s_iter)
				x += s_iter->width;
			caret_x = x;
			for (; i < sel_begin.n_character; ++i, ++s_iter)
				width += s_iter->width;
		}
		GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, x, y,
			width, a_iter->height + a_iter->padding_top);
	}
	else
	{
#define _FROM_ (sel_begin.n_line < caret.n_line ? sel_begin : caret)
#define _TO_ (sel_begin.n_line < caret.n_line ? caret : sel_begin)
		int x, y, width;
		x = y = width = 0;
	
		Article::const_iterator a_iter = article.begin();
		for (int i = 0; i != _FROM_.n_line; ++i, ++a_iter)
			y += a_iter->height + a_iter->padding_top;

		{
			Sentence::const_iterator s_iter;
			if (!a_iter->sentence.empty())
				s_iter = a_iter->sentence.begin();
			
			int i = 0;
			for (; i < _FROM_.n_character; ++i, ++s_iter)
				x += s_iter->width;
			for (; i < a_iter->sentence.size(); ++i, ++s_iter)
				width += s_iter->width;
		
			GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, x, y,
							width, a_iter->height + a_iter->padding_top);

			if (sel_begin.n_line >= caret.n_line)
			{
				caret_x = x;
				caret_y = y;
				caret_h = a_iter->height;
			}
		}

		y += a_iter->height + a_iter->padding_top;
		width = 0;
		++a_iter;
		for (int i = _FROM_.n_line + 1; i != _TO_.n_line; ++i, ++a_iter)
		{
			Sentence::const_iterator s_iter;
			if (!a_iter->sentence.empty())
				s_iter = a_iter->sentence.begin();
			for (int j = 0; j < a_iter->sentence.size(); ++j, ++s_iter)
				width += s_iter->width;

			GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, 0, y,
				width, a_iter->height + a_iter->padding_top);
			width = 0;
			y += a_iter->height + a_iter->padding_top;
		}
		
		Sentence::const_iterator s_iter;
		if (!a_iter->sentence.empty())
			s_iter = a_iter->sentence.begin();
		for (int i = 0; i < _TO_.n_character; ++i, ++s_iter)
			width += s_iter->width;
		
		if (sel_begin.n_line < caret.n_line)
		{
			caret_x = width;
			caret_y = y;
			caret_h = a_iter->height;
		}

		GDIUtil::fill(*textView, MNP_BGCOLOR_SEL, 0, y,
			width, a_iter->height + a_iter->padding_top);
#undef _FROM_
#undef _TO_
	}
there:

	int y = 0;
	for (Line& l : article)
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
inline void seeCaret() {
	RECT rc;
	GetClientRect(hWnd, &rc);
	int ClientWidth = rc.right - rc.left;
	int ClientHeight = rc.bottom - rc.top;

	int x, y;
	x = y = 0;
	Article::const_iterator a_iter = article.begin();
	for (int i = 0; i != caret.n_line; ++i, ++a_iter)
		y += a_iter->height + a_iter->padding_top;
	Sentence::const_iterator s_iter;
	if (!a_iter->sentence.empty())
		s_iter = a_iter->sentence.begin();
	for (int i = 0; i < caret.n_character; ++i, ++s_iter)
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
		if (removeSelectedChars())
			;
		else if (caret.n_line != article.size() - 1 
			|| caret.n_character != caret.getLineLength(article))
		{
			// delete the character
			caret.eraseChar(article);
			bSaved = false;
		}
		else return;	// end of all, nothing to delete
		break;
	case VK_LEFT:
		if (caret.n_character == 0)
		{
			// the first char in this line
			if (caret.n_line == 0)
				return;	// first of all, nothing to do
			else
			{
				// not the first line, back to the end of previous line
				--caret.n_line;
				caret.n_character = caret.getLineLength(article);
			}
		}
		else
			--caret.n_character;
		break;
	case VK_RIGHT:
		if (caret.n_character == caret.getLineLength(article))
		{
			// the last char in this line, go to the front of next line
			if (caret.n_line == article.size() - 1)
				return;	// end of all, nothing to do
			else
			{
				// not the last line
				++caret.n_line;
				caret.n_character = 0;
			}
		}
		else
			++caret.n_character;
		break;
	case VK_UP:
		// the first line
		if (caret.n_line == 0)
			return;	// nothing to do
		else
		{
			// not the first line, back to the end of previous line
			--caret.n_line;
			size_t len = caret.getLineLength(article);
			if (len < caret.n_character)
				caret.n_character = len;
		}
		break;
	case VK_DOWN:
		// the last char in this line, go to the front of next line
		if (caret.n_line == article.size() - 1)
			return;	// nothing to do
		else
		{
			// not the last line
			++caret.n_line;
			size_t len = caret.getLineLength(article);
			if (len < caret.n_character)
				caret.n_character = len;
		}
		break;
	case VK_HOME:
		if (GetKeyState(VK_CONTROL) < 0)
		{
			if (caret == Cursor{ 0,0 })
				return;
			else
			{
				caret.n_line = 0;
				caret.n_character = 0;
			}
		}
		else
		{
			if (caret.n_character != 0)
				caret.n_character = 0;
			else
				return;
		}
		break;
	case VK_END:
	{
		size_t n_chars = caret.getLineLength(article);
		if (GetKeyState(VK_CONTROL) < 0)
		{
			size_t n_lines = article.size() - 1;
			if (caret.n_line == n_lines && caret.n_character == n_chars)
				return;
			else
			{
				caret.n_line = n_lines;
				caret.n_character = n_chars;
			}
		}
		else
		{
			if (caret.n_character != n_chars)
				caret.n_character = n_chars;
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
	bool flag = removeSelectedChars();
	switch (nChar)
	{
	case VK_BACK:
		if (flag) break;
		if (caret != Cursor{ 0,0 })
			caret.backspace(article);
		else
			return;
		break;
	case VK_RETURN:
		insertAtCursor(article, L'\n');
		break;
	case VK_TAB:
	{
		Character c(L' ');
		insertAtCursor(article, c);		// convert tab to space
		insertAtCursor(article, c);
		insertAtCursor(article, c);
		insertAtCursor(article, c);
	}
		break;
	default:
		insertAtCursor(article, Character(nChar));
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
	else if (cursor_y >= textView_height)
		cursor_y = textView_height - MNP_LINEHEIGHT;

	cursor_x += xoffset - MNP_PADDING_CLIENT;
	if (cursor_x < 0)
		cursor_x = 0;
	
	int x, y;
	x = y = 0;
	Cursor cursor = { 0,0 };
	size_t size = article.size() - 1;
	for (Article::const_iterator a_iter = article.begin(); a_iter != article.end(); ++a_iter)
	{
		y += a_iter->height + a_iter->padding_top;
		if (y < cursor_y && cursor.n_line < size)
			++cursor.n_line;
		else
			break;
	}
	Sentence& s = cursor.getSentence(article)->sentence;
	size = cursor.getLineLength(article);
	for (Sentence::const_iterator s_iter = s.begin(); s_iter != s.end(); ++s_iter)
	{
		if ((x += s_iter->width) < cursor_x + s_iter->width/2 && cursor.n_character < size)
			++cursor.n_character;
		else
			break;
	}

	return cursor;
}

inline void OnLButtonDown(DWORD wParam, int x, int y) {
	HDC hdc = GetDC(hWnd);
	sel_begin = caret = PosToCaret(x, y);
	repaintView(hdc);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnMouseMove(DWORD wParam, int x, int y) {
	if (wParam & MK_LBUTTON)
	{
		HDC hdc = GetDC(hWnd);
		Cursor new_caret = PosToCaret(x, y);
		if (caret != new_caret)
		{
			caret = new_caret;
			repaintView(hdc);
			OnPaint(hdc);
		}
		ReleaseDC(hWnd, hdc);
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
	Character c(L'\n');
	article.front().sentence.push_front(c);
	article.back().sentence.push_back(c);

	// to HTML
	std::wstring str;
	for (const Line& l : article)
		str += l;
	parse_markdown(str);

	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
	LPTSTR p = static_cast<LPTSTR>(GlobalLock(h));
	for (TCHAR& c : str) {
		*p = c;
		++p;
	}
	*p = '\0';

	article.front().sentence.pop_front();
	article.back().sentence.pop_back();

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

		// !important
		Character c(L'\n');
		article.front().sentence.push_front(c);
		article.back().sentence.push_back(c);

		// write to file
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::ofstream f(file);
		f << "<!DOCTYPE><head><meta charset=\"utf-8\"/><head><body>";
		std::wstring str;
		for (const Line& l : article)
			str += l;
		parse_markdown(str);
		f << cvt.to_bytes(str);
		f << "</body>";
		f.close();

		article.front().sentence.pop_front();
		article.back().sentence.pop_back();
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
	return true;
}

// repaintView() & OnPaint()
inline void refresh() {
	HDC hdc = GetDC(hWnd);
	repaintView(hdc);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
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
	textView_width = 0;
	textView_height = MNP_LINEHEIGHT;
	
	caret = { 0,0 };
	insertAtCursor(article, str);

	sel_begin = caret = { 0,0 };
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

	if (GetOpenFileNameW(&ofn) > 0) {
		loadFile(file);
	}
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
	std::wstring str = getSelectedChars();
	h = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
	LPTSTR p = static_cast<LPTSTR>(GlobalLock(h));
	for (TCHAR& c : str)
		*p++ = c;
	*p = L'\0';

	GlobalUnlock(h);
	SetClipboardData(CF_UNICODETEXT, h);
	CloseClipboard();
}

inline void OnMenuCut() {
	OnMenuCopy();
	removeSelectedChars();
	refresh();
}

inline void OnMenuPaste() {

	if (!OpenClipboard(hWnd)) return;

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	if (NULL == h) {
		CloseClipboard();
		return;
	}

	removeSelectedChars();
	LPCTSTR str = (LPCTSTR)GlobalLock(h);
	insertAtCursor(article, str);
	GlobalUnlock(h);
	CloseClipboard();

	refresh();
}

inline void OnMenuAll() {
	sel_begin = { 0, 0 };
	caret.n_line = article.size() - 1;
	caret.n_character = article.end()->sentence.size();
	refresh();
}
