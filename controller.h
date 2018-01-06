#pragma once
#include "MarkdownParser.h"

//---------------------------------
Article article = { Line() };		// article text
Cursor caret(article);				// current position = end of selection
Cursor sel_begin(article);			// beginning of selection 
bool bSaved = true;					// set false when file is modified
unsigned int textView_width = 0, textView_height = 0;
unsigned int caret_x, caret_y;		// for IME (relative to EditArea)
unsigned int xoffset = 0;			// offset-x of textView
unsigned int yoffset = 0;			// offset-y of textView
size_t ClientWidth, ClientHeight, EditAreaWidth, EditAreaHeight;
bool word_wrap = false;				// break word
bool resized = false;
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

// update MemDC, text_width, text_height for the line and update
void repaintLine(HDC clientDC, const Line& l)
{
	// calc width of each line
	l.text_width = 0;
	for (Sentence::const_iterator i = l.sentence.begin(); i != l.sentence.end(); ++i)
		l.text_width += i->width;
	if (!word_wrap && l.text_width + l.padding_left > textView_width)
		textView_width = l.text_width + l.padding_left;
	
	// discard present MemDC and recreate
	l.mdc = std::make_unique<MemDC>(clientDC,
		l.text_width + l.padding_left,
		l.text_height + l.padding_top);

	// fill background
	GDIUtil::fill(*l.mdc,
		l.background_color,
		0,
		0,
		l.text_width + l.padding_left,
		l.text_height + l.padding_top);

	// draw text
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(*l.mdc);
	f.printLine(l, l.padding_left, l.padding_top);
	f.unbind();
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
		if (i == sel_from)	// if sel_to equals sentence.end(), 
			x_sel_from = l.text_width;	// x_sel_from will not be assigned a value
		if (i == sel_to)	// if sel_to equals sentence.end(), 
			x_sel_to = l.text_width;	// x_sel_to will not be assigned a value
		l.text_width += i->width;
	}
	if (!word_wrap && l.text_width + l.padding_left > textView_width)
		textView_width = l.text_width + l.padding_left;
	if (l.sentence.end() == sel_from)
		x_sel_from = l.text_width;
	if (l.sentence.end() == sel_to)
		x_sel_to = l.text_width;
	
	// discard present MemDC and recreate
	l.mdc = std::make_unique<MemDC>(clientDC,
		l.text_width + l.padding_left,
		l.text_height + l.padding_top);

	// fill background
	GDIUtil::fill(*l.mdc,
		l.background_color,
		0,
		0,
		l.text_width + l.padding_left,
		l.text_height + l.padding_top);

	// fill selection background
	GDIUtil::fill(*l.mdc,
		MNP_BGCOLOR_SEL,
		l.padding_left + x_sel_from,
		l.padding_top,
		x_sel_to - x_sel_from,
		l.text_height);

	// draw text
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(*l.mdc);
	f.printLine(l, l.padding_left, l.padding_top);
	f.unbind();
}

// When text was modified, call this function to recalculate textView_height
void calc_textView_height()
{
	textView_height = 0;
	for (const Line& i : article)
		textView_height += i.text_height + i.padding_top;

	// textView_width will be updated when repaintLine() or OnSize() is called
}

// fill selection background
void repaintSelectedLines()
{
	if (sel_begin.getSentence() != caret.getSentence())
	{
		HDC hdc = GetDC(hWnd);
		// repaint selected lines
		int dist_y = sel_begin.distance_y(caret);
		if (dist_y < 0)	// forward selection
		{
			// repaint first line
			Article::const_iterator l = sel_begin.getSentence();
			repaintLine(hdc, *l, sel_begin.getCharacter(), l->sentence.end());
			// repaint each line
			for (++l; l != caret.getSentence(); ++l)
				repaintLine(hdc, *l, l->sentence.begin(), l->sentence.end());
			// repaint last line
			repaintLine(hdc, *l, l->sentence.begin(), caret.getCharacter());
		}
		else	// backwards selection
		{
			// repaint first line
			Article::const_iterator l = caret.getSentence();
			repaintLine(hdc, *l, caret.getCharacter(), l->sentence.end());
			// repaint each line
			for (++l; l != sel_begin.getSentence(); ++l)
				repaintLine(hdc, *l, l->sentence.begin(), l->sentence.end());
			// repaint last line
			repaintLine(hdc, *l, l->sentence.begin(), sel_begin.getCharacter());
		}
		ReleaseDC(hWnd, hdc);
	}
	else if (sel_begin.getCharacter() != caret.getCharacter())
	{
		HDC hdc = GetDC(hWnd);
		int dist_x = sel_begin.distance_x(caret);
		if (dist_x < 0)
			repaintLine(hdc, *caret.getSentence(), sel_begin.getCharacter(), caret.getCharacter());
		else
			repaintLine(hdc, *caret.getSentence(), caret.getCharacter(), sel_begin.getCharacter());
		ReleaseDC(hWnd, hdc);
	}
}

void OnPaint(HDC hdc) {
	if (word_wrap && resized)
	{
		// for selection
		if (sel_begin == caret)
		{
			caret.rebond_all();
			caret.split_all_long_text(EditAreaWidth);

			sel_begin = caret;
			for (auto& l : article)
				repaintLine(hdc, l);
		}
		else
		{
			int dist_y = sel_begin.distance_y(caret);
			if (dist_y < 0)	// forward selection
				sel_begin.reform(EditAreaWidth, caret);
			else if (dist_y > 0)
				caret.reform(EditAreaWidth, sel_begin);
			else
			{
				int dist_x = sel_begin.distance_x(caret);
				if (dist_x < 0)	// forward selection
					sel_begin.reform(EditAreaWidth, caret);
				else
					caret.reform(EditAreaWidth, sel_begin);
			}

			for (auto& l : article)
				repaintLine(hdc, l);
			repaintSelectedLines();
		}
		calc_textView_height();
	}
	resized = false;

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
				caret_x = l->padding_left;	// calc caret_x
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
					caret_y = (y - l->text_height) - yoffset;
					GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_CLIENT, caret_y,
						caret_x + MNP_PADDING_CLIENT, caret_y + caret.getSentence()->text_height);
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
			caret_x = l->padding_left;
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
				caret_y = y + caret.getSentence()->padding_top;
				GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_CLIENT, caret_y,
					caret_x + MNP_PADDING_CLIENT, caret_y + caret.getSentence()->text_height);
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
	if (!word_wrap && textView_width > ClientWidth - MNP_PADDING_CLIENT) {
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
	size_t x = 0, y = 0;

	// count y
	for (Article::const_iterator a_iter = article.begin();
		a_iter != caret.getSentence();
		++a_iter)
		y += a_iter->text_height + a_iter->padding_top;

	if (y < yoffset)	// over the client area
		yoffset = y;
	else if (y + MNP_LINEHEIGHT > yoffset + EditAreaHeight)	// below the client area
		yoffset = y + MNP_LINEHEIGHT - EditAreaHeight;

	if (word_wrap)
		xoffset = 0;
	else
	{
		// count x
		for (Sentence::const_iterator s_iter = caret.getSentence()->sentence.begin();
			s_iter != caret.getCharacter();
			++s_iter)
			x += s_iter->width;

		if (x < xoffset)	// on the left of client area
			xoffset = x;
		else if (x > xoffset + EditAreaWidth)	// on the right of client area
			xoffset = x - EditAreaWidth;
	}
}

// remove selection background
void repaintSelectionCanceledLines()
{
	// repaint selected lines to remove selection background
	if (sel_begin.getSentence() != caret.getSentence())
	{
		HDC hdc = GetDC(hWnd);
		// repaint selected lines
		int dist_y = sel_begin.distance_y(caret);
		if (dist_y < 0)	// forward selection
		{
			for (Article::const_iterator l = sel_begin.getSentence();
				l != ++caret.getSentence(); ++l)
				repaintLine(hdc, *l);
		}
		else	// backwards selection
		{
			for (Article::const_iterator l = caret.getSentence();
				l != ++sel_begin.getSentence(); ++l)
				repaintLine(hdc, *l);
		}
		ReleaseDC(hWnd, hdc);
	}
	else if (sel_begin.getCharacter() != caret.getCharacter())
	{
		HDC hdc = GetDC(hWnd);
		repaintLine(hdc, *caret.getSentence());
		ReleaseDC(hWnd, hdc);
	}
}

inline void OnSize(unsigned int width, unsigned int height) {
	ClientWidth = width;
	ClientHeight = height;
	EditAreaWidth = ClientWidth - MNP_PADDING_CLIENT - MNP_SCROLLBAR_WIDTH;
	EditAreaHeight = ClientHeight - MNP_SCROLLBAR_WIDTH;
	resized = true;
	if (word_wrap)
		textView_width = EditAreaWidth;
}

inline void repaintModifiedLine(HDC& hdc)
{
	if (word_wrap)
	{
		caret.rebond();
		Article::const_iterator a = caret.getSentence();
		Article::const_iterator b = caret.split_long_text(EditAreaWidth);
		sel_begin = caret;
		for (auto i = a; i != b; ++i)
			repaintLine(hdc, *i);
	}
	else
		repaintLine(hdc, *caret.getSentence());
}

inline void force_OnPaint()
{
	HDC hdc = GetDC(hWnd);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

void OnKeyDown(int nChar) {
	bool flag_ShiftPressed = (GetKeyState(VK_SHIFT) < 0);

	switch (nChar)
	{
	case VK_DELETE:
		if (sel_begin.removeSelectedChars(caret))
		{
			bSaved = false;

			HDC hdc = GetDC(hWnd);

			repaintModifiedLine(hdc);

			calc_textView_height();
			seeCaret();

			OnPaint(hdc);
			ReleaseDC(hWnd, hdc);
			return;
		}
		else
		{
			// delete the character
			if (!caret.eraseChar())
				return;
			bSaved = false;
			
			HDC hdc = GetDC(hWnd);

			sel_begin = caret;
			repaintModifiedLine(hdc);

			calc_textView_height();
			seeCaret();

			OnPaint(hdc);
			ReleaseDC(hWnd, hdc);
			return;
		}
	case VK_LEFT:
		// some lines were selected, but Shift was not pressed,
		// remove selection background
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		caret.move_left();
		break;
	case VK_RIGHT:
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		caret.move_right();
		break;
	case VK_UP:
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		// the first line
		if (caret.getSentence() == article.begin())
		{
			force_OnPaint();
			return;	// nothing to do
		}
		else
		{
			// not the first line, go to previous line
			caret.toFirstChar();
			caret.move_left();
		}
		break;
	case VK_DOWN:
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		// the last char in this line, go to the front of next line
		if (caret.getSentence() == --article.end())
		{
			force_OnPaint();
			return;	// nothing to do
		}
		else
		{
			// not the last line, go to next line
			caret.toLastChar();
			caret.move_right();
		}
		break;
	case VK_HOME:
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		if (GetKeyState(VK_CONTROL) < 0)	// Ctrl was pressed
		{
			if (caret.getSentence() == article.begin() 
				&& caret.getCharacter() == caret.getSentence()->sentence.begin())
			{
				force_OnPaint();
				return;
			}
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
		if (!flag_ShiftPressed)
			repaintSelectionCanceledLines();
		if (GetKeyState(VK_CONTROL) < 0)	// Ctrl was pressed
		{
			if (caret.getSentence() == --article.end()
				&& caret.getCharacter() == caret.getSentence()->sentence.end())
			{
				force_OnPaint();
				return;
			}
			else
				caret.end();
		}
		else
		{
			if (caret.getCharacter() != caret.getSentence()->sentence.end())
				caret.toLastChar();
			else
			{
				force_OnPaint();
				return;
			}
		}
		break;
	default:
		return;
	};

	if (flag_ShiftPressed)
		repaintSelectedLines();	// paint selection background
	else
		sel_begin = caret;

	seeCaret();
	
	force_OnPaint();
}

void OnChar(WORD nChar)
{
	// replace selected characters
	bool flag_removed = sel_begin.removeSelectedChars(caret);

	HDC hdc = GetDC(hWnd);

	switch (nChar)
	{
	case VK_BACK:
		if (!flag_removed && !caret.backspace())
			return;
		break;
	case VK_RETURN:
		insertAtCursor(L'\n');
		repaintLine(hdc, *(--caret.getSentence()));
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
	repaintModifiedLine(hdc);

	calc_textView_height();
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
	int x = l->padding_left;
	for (c = l->sentence.begin(); c != l->sentence.end(); ++c)
	{
		if ((x += c->width) >= cursor_x + c->width/2)
			break;
	}

	if (word_wrap && c == l->sentence.begin() && l->child_line)
	{
		--l;
		c = l->sentence.end();
	}

	return Cursor(article, l, c);
}

inline void OnLButtonDown(DWORD wParam, int x, int y) {
	repaintSelectionCanceledLines();

	sel_begin = caret = PosToCaret(x, y);
	force_OnPaint();
}

inline void OnRButtonDown(DWORD wParam, int x, int y) {
	POINT p = { x,y };
	ClientToScreen(hWnd, &p);
	HMENU hm = LoadMenu(hInst, MAKEINTRESOURCE(IDC_POPUP));

	TrackPopupMenu(GetSubMenu(hm, 0), TPM_LEFTALIGN, p.x, p.y, NULL, hWnd, NULL);
	DestroyMenu(hm);
}

inline void OnMouseMove(DWORD wParam, int x, int y) {
	if (wParam & MK_LBUTTON)
	{
		Cursor new_caret = PosToCaret(x, y);
		if (caret != new_caret)
		{
			repaintSelectionCanceledLines();
			caret = new_caret;
			repaintSelectedLines();
			force_OnPaint();
		}
	}
}

void OnMouseHWheel(short zDeta, int x, int y) {

	if (textView_width < EditAreaWidth)
		return;
	xoffset += zDeta / 2;

	if ((int)xoffset < 0)
		xoffset = 0;
	else if (xoffset + EditAreaWidth > textView_width)
		xoffset = textView_width - EditAreaWidth;

	force_OnPaint();
}

void OnMouseWheel(short zDeta, int x, int y) {
	if (!word_wrap && GetKeyState(VK_SHIFT) < 0) {
		OnMouseHWheel(-zDeta, x, y);
		return;
	}

	if (textView_height < EditAreaHeight)
		return;
	yoffset -= zDeta / 2;

	if ((int)yoffset < 0)
		yoffset = 0;
	else if (yoffset + EditAreaHeight > textView_height)
		yoffset = textView_height - EditAreaHeight;

	force_OnPaint();
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

inline std::wstring all_to_string()
{
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
		std::wstring str = all_to_string();
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
		std::wstring str = all_to_string();
		
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
	article.push_back(Line());
	
	// set text
	caret.reset();
	insertAtCursor(str);

	// init
	sel_begin.reset();
	caret.reset();
	bSaved = true;
	opened_file = path;
	SetWindowTextW(hWnd, (opened_file + L" - MyNotePad").c_str());

	if (word_wrap)
	{
		caret.rebond_all();
		caret.split_all_long_text(EditAreaWidth);
		sel_begin = caret;
	}

	HDC hdc = GetDC(hWnd);
	for (auto& l : article)
		repaintLine(hdc, l);

	calc_textView_height();

	seeCaret();

	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
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
	{
		bSaved = false;
		HDC hdc = GetDC(hWnd);

		repaintModifiedLine(hdc);

		calc_textView_height();

		OnPaint(hdc);
		ReleaseDC(hWnd, hdc);
	}
}

inline void OnMenuPaste() {
	if (!OpenClipboard(hWnd)) return;

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	if (NULL == h) {
		CloseClipboard();
		return;
	}

	sel_begin.removeSelectedChars(caret);
	
	LPCTSTR str = (LPCTSTR)GlobalLock(h);

	insertAtCursor(str);
	bSaved = false;

	sel_begin = caret;
	if (word_wrap) 
	{
		caret.rebond_all();
		caret.split_all_long_text(EditAreaWidth);
		sel_begin = caret;
	}

	GlobalUnlock(h);
	CloseClipboard();

	HDC hdc = GetDC(hWnd);
	for (auto& l : article)
		repaintLine(hdc, l);

	calc_textView_height();
	
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}

inline void OnMenuAll() {
	sel_begin.reset();
	caret.end();
	repaintSelectedLines();

	force_OnPaint();
}

inline void OnMenuWordWrap()
{
	HDC hdc = GetDC(hWnd);
	if (word_wrap)
	{
		// exit word wrap mode
		if (sel_begin == caret)
		{
			caret.rebond_all();
			sel_begin = caret;
			for (auto& l : article)
				repaintLine(hdc, l);
		}
		else
		{
			int dist_y = sel_begin.distance_y(caret);
			if (dist_y < 0)	// forward selection
				sel_begin.recover(caret);
			else if (dist_y > 0)
				caret.recover(sel_begin);
			else
			{
				int dist_x = sel_begin.distance_x(caret);
				if (dist_x < 0)	// forward selection
					sel_begin.recover(caret);
				else
					caret.recover(sel_begin);
			}
			for (auto& l : article)
				repaintLine(hdc, l);
			repaintSelectedLines();
		}
	}
	else
	{
		// enter word wrap mode
		xoffset = 0;
		resized = true;	// rebond() and split() in OnPaint()
	}
	word_wrap = !word_wrap;
	HMENU hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, IDM_WORDWRAP, word_wrap ? MF_CHECKED : MF_UNCHECKED);

	calc_textView_height();
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
}