#pragma once
#include "MarkdownParser.h"
//---------------------------------
Article article = { Line() };		// article text
Cursor caret(article);				// current position = end of selection
Cursor sel_begin(article);			// beginning of selection 
bool bSaved = true;					// set false after file is modified
std::wstring exeFilePath;			// EXE File Path, eg. C:\Example\ 
unsigned int textView_width = 0, textView_height = 0;
unsigned int caret_x, caret_y;		// for IME (relative to EditArea)
unsigned int xoffset = 0;			// offset-x of textView
unsigned int yoffset = 0;			// offset-y of textView
size_t ClientWidth, ClientHeight, EditAreaWidth, EditAreaHeight;
bool word_wrap = false;				// break word (initial false)
bool show_line_number = true;		// display line number (initial true)
std::unique_ptr<Font> line_number_font;
bool resized = false;
char32_t two_utf16_encoded_chars = 0;	// if a unicode between U+10000 and U+10FFFF (eg. emoji)
									// input by user, it will be encoded with UTF-16.
									// System send WM_CHAR message twice,
									// the first TCHAR will be in the range 0xD800..0xDBFF,
									// the second TCHAR will be in the range 0xDC00..0xDFFF.
//---------------------------------

void insertAtCursor(const TCHAR& c)
{
	if (c == L'\n')
	{
		caret.insertCharacter(Character(L'\n'));
	}
	else if (c == L'\t')
	{
		Character tab(L' ');

		// set tab to 4 spaces' width
		HDC hdc = GetDC(hWnd);
		Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
		f.bind(hdc);
		GetCharWidth32W(hdc, tab.c, tab.c, &tab.width);
		f.unbind();
		ReleaseDC(hWnd, hdc);

		tab.c = L'\t';
		tab.width *= 4;
		caret.insertCharacter(tab);
	}
	else
	{
		// emoji
		if (c > 0xD800 && c < 0xDBFF)
		{
			two_utf16_encoded_chars = c;
			return;
		}
		if (two_utf16_encoded_chars != 0)
		{
			if (c > 0xDC00 && c < 0xDFFF)
			{
				Character ch((two_utf16_encoded_chars << 16) | c);
				two_utf16_encoded_chars = 0;
				ch.width = 20;	// emoji char width
				caret.insertCharacter(ch);
			}
			else
			{
				two_utf16_encoded_chars = 0;
				MessageBoxW(hWnd, L"Invalid encoding!", MNP_APPNAME, MB_OK);
			}
		}
		else
		{
			Character ch(c);
			HDC hdc = GetDC(hWnd);
			Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
			f.bind(hdc);
			GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
			f.unbind();
			ReleaseDC(hWnd, hdc);
			caret.insertCharacter(ch);
		}
	}
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
			// convert tab to 4 spaces' width
			Character ch(L' ');
			GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
			ch.c = L'\t';
			ch.width *= 4;
			caret.insertCharacter(ch);
		}
		else if (*i == L'\n')
		{
			insertAtCursor(L'\n');
		}
		else
		{
			TCHAR c = *i;

			// emoji
			if (c > 0xD800 && c < 0xDBFF)
			{
				two_utf16_encoded_chars = c;
				continue;
			}
			if (two_utf16_encoded_chars != 0)
			{
				if (c > 0xDC00 && c < 0xDFFF)
				{
					Character ch((two_utf16_encoded_chars << 16) | c);
					two_utf16_encoded_chars = 0;
					ch.width = 20;	// emoji char width
					caret.insertCharacter(ch);
				}
				else
				{
					two_utf16_encoded_chars = 0;
					MessageBoxW(hWnd, L"Invalid encoding!", MNP_APPNAME, MB_OK);
				}
			}
			else
			{
				Character ch(c);
				GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
				caret.insertCharacter(ch);
			}
		}
	}

	f.unbind();
	ReleaseDC(hWnd, hdc);
}

// update MemDC, text_width, text_height for the line
// and increase textView_width (only if text_width is greater)
void repaintLine(HDC clientDC, const Line& l, bool whole_line_selected = false)
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
	if (whole_line_selected)
		GDIUtil::fill(*l.mdc,
			MNP_BGCOLOR_SEL,
			0,
			0,
			l.text_width + l.padding_left,
			l.text_height + l.padding_top);
	else
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

// update MemDC, text_width, text_height for the line
// (only if some text selected in this line)
void repaintLine(HDC clientDC, const Line& l,
	const Sentence::const_iterator sel_from,
	const Sentence::const_iterator sel_to)
{
	// make sure sel_from and sel_to are in the same line
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
				repaintLine(hdc, *l, true);
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
				repaintLine(hdc, *l, true);
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
	// Window size changed so we should adjust text.
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

	size_t y = 0;
	Article::const_iterator l = article.begin();
	size_t n_line = article.size();	// a counter for line number
	if (show_line_number)
	{
		line_number_font->bind(clientDC);
		if (n_line > 999)
			MNP_PADDING_LEFT = 64;
		else if (n_line > 99)
			MNP_PADDING_LEFT = 50;
		else if (n_line > 9)
			MNP_PADDING_LEFT = 38;
		else
			MNP_PADDING_LEFT = 26;
		n_line = 0;
	}
	// draw first line (probably can't be fully seen)
	for (; l != article.end(); ++l)
	{
		y += l->text_height + l->padding_top;
		if (show_line_number && !l->child_line)
			++n_line;
		// the first line of edit area
		if (y > yoffset)
		{
			if (show_line_number)
			{
				std::wstringstream ss; ss << n_line;
				line_number_font->printLine(ss.str(),
					MNP_LINENUM_MARGIN_LEFT,
					MNP_LINENUM_MARGIN_TOP + (y - l->text_height - l->padding_top) - yoffset);
			}
			BitBlt(clientDC,
				MNP_PADDING_LEFT,	// dest x
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
					GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_LEFT, caret_y,
						caret_x + MNP_PADDING_LEFT, caret_y + caret.getSentence()->text_height);
				}
			}
			++l;
			break;
		}
	}
	y -= yoffset;	// Y-Axis to paint the second line 
	// draw subsequent lines
	for (; l != article.end(); ++l)
	{
		if (show_line_number)
		{
			if (!l->child_line)
				++n_line;
			std::wstringstream ss; ss << n_line;
			line_number_font->printLine(ss.str(),
				MNP_LINENUM_MARGIN_LEFT,
				MNP_LINENUM_MARGIN_TOP + y);
		}
		if (y > EditAreaHeight)	// last line (can't be seen entirely) 
		{
			BitBlt(clientDC,
				MNP_PADDING_LEFT,	// dest x
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
			MNP_PADDING_LEFT,		// dest x
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
				GDIUtil::line(clientDC, MNP_FONTCOLOR, caret_x + MNP_PADDING_LEFT, caret_y,
					caret_x + MNP_PADDING_LEFT, caret_y + caret.getSentence()->text_height);
			}
		}
		y += l->text_height + l->padding_top;
	}

	if (show_line_number)
		line_number_font->unbind();

	// draw vertical scrollbar
	if (textView_height > ClientHeight - MNP_PADDING_LEFT) {
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
	if (!word_wrap && textView_width > ClientWidth - MNP_PADDING_LEFT) {
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
	else
	{
		HDC hdc = GetDC(hWnd);
		repaintLine(hdc, *caret.getSentence());
		ReleaseDC(hWnd, hdc);
	}
}

inline void OnSize(unsigned int width, unsigned int height) {
	ClientWidth = width;
	ClientHeight = height;
	EditAreaWidth = ClientWidth - MNP_PADDING_LEFT - MNP_SCROLLBAR_WIDTH;
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
		if (caret.isFirstLine())
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
		if (caret.isLastLine())
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
			if (caret.isFirstLine() && caret.isFirstChar())
			{
				force_OnPaint();
				return;
			}
			else
				caret.reset();
		}
		else
		{
			if (!caret.isFirstChar())
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
			if (caret.isLastLine() && caret.isLastChar())
			{
				force_OnPaint();
				return;
			}
			else
				caret.end();
		}
		else
		{
			if (!caret.isLastChar())
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
	{
		if (caret == sel_begin)
			repaintSelectionCanceledLines();
		else
			repaintSelectedLines();	// paint selection background
	}
	else
		sel_begin = caret;

	seeCaret();
	
	force_OnPaint();
}

void OnChar(WORD nChar)
{
	if (nChar == VK_ESCAPE)
		return;

	// replace selected characters
	bool flag_removed = sel_begin.removeSelectedChars(caret);

	HDC hdc;
	switch (nChar)
	{
	case VK_BACK:
		if (!flag_removed && !caret.backspace())
			return;
		hdc = GetDC(hWnd);
		break;
	case VK_RETURN:
		insertAtCursor(L'\n');
		hdc = GetDC(hWnd);
		repaintLine(hdc, *(--caret.getSentence()));
		break;
	default:
		insertAtCursor(nChar);
		hdc = GetDC(hWnd);
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
	if (cursor_y < 0)
		cursor_y = 0;
	if (cursor_x < 0)
		cursor_x = 0;
	cursor_y += yoffset;
	if (cursor_y < 0)
		cursor_y = 0;
	cursor_x += xoffset - MNP_PADDING_LEFT;
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

bool mouse_down;
inline void OnLButtonDown(DWORD wParam, int x, int y) {
	bool flag_ShiftPressed = (GetKeyState(VK_SHIFT) < 0);
	mouse_down = true;
	SetCapture(hWnd);
	repaintSelectionCanceledLines();

	if (flag_ShiftPressed)
	{
		caret = PosToCaret(x, y);
		if (caret != sel_begin)
			repaintSelectedLines();
	}
	else
		sel_begin = caret = PosToCaret(x, y);

	force_OnPaint();
}

inline void OnLButtonUp(DWORD wParam, int x, int y) {
	if (mouse_down)
		ReleaseCapture();
	mouse_down = false;
}

/*
 * Double-clicking the left mouse button
 * actually generates a sequence of four messages:
 * WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, and WM_LBUTTONUP.
 * Only windows that have the CS_DBLCLKS style
 * can receive WM_LBUTTONDBLCLK messages.
 */
inline void OnLButtonDBClick(DWORD wParam, int x, int y) {
	bool flag_ShiftPressed = (GetKeyState(VK_SHIFT) < 0);
	if (flag_ShiftPressed) {
		OnLButtonDown(wParam, x, y);
		return;
	}
	repaintSelectionCanceledLines();
	sel_begin = caret = PosToCaret(x, y);

	if (!caret.getSentence()->sentence.empty())
	{
		if (caret.isLastChar())
			sel_begin.move_left();
		else
			sel_begin.move_right();
		repaintSelectedLines();
	}

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
	if (mouse_down) // wParam & MK_LBUTTON
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
	// no scroll bar
	if (textView_width < EditAreaWidth && xoffset == 0)
	{
		if (xoffset != 0)
		{
			xoffset = 0;
			force_OnPaint();
		}
		return;
	}
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

	// no scroll bar
	if (textView_height < EditAreaHeight)
	{
		if (yoffset != 0)
		{
			yoffset = 0;
			force_OnPaint();
		}
		return;
	}
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

template<typename T>
inline T all_to_string()
{
	T str;
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

	std::wstring str = all_to_string<std::wstring>();
	parse_markdown(str);

	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
	LPTSTR p = static_cast<LPTSTR>(GlobalLock(h));
	for (TCHAR& c : str) {
		*p = c;
		++p;
	}
	*p = L'\0';
	
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

template <typename T>
inline void saveHTML(T pathname)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	std::ofstream f(pathname);
	
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
	std::wstring str = all_to_string<std::wstring>();
	parse_markdown(str);
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

inline void OnMenuExport() {
	TCHAR pathname[MAX_PATH];
	*pathname = L'\0';

	OPENFILENAME ofn;
	ofn.lpstrFile = pathname;
	setOFN(ofn, L"HTML (*.html)\0*.html\0All Files (*.*)\0*.*\0\0", L"html");

	if (GetSaveFileNameW(&ofn) > 0) {
		// write to file
		saveHTML(pathname);
	}
}

void OnMenuSaveAs() {
	TCHAR file[MAX_PATH];	*file = L'\0';

	OPENFILENAME ofn;
	ofn.lpstrFile = file;
	setOFN(ofn, L"MarkDown (*.md)\0*.md\0All Files (*.*)\0*.*\0\0", L"md");

	if (GetSaveFileNameW(&ofn) > 0) {
		std::wstring str = all_to_string<std::wstring>();
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
		std::wstring str = all_to_string<std::wstring>();
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
		path[lstrlenW(path) - 1] = L'\0';
		++path;
	}
	// load file
	std::ifstream f(path);
	if (f.fail()) {
		std::wstringstream ss;
		ss << L"Failed to load \"" << path << L"\".";
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
		MessageBoxW(hWnd, L"Can only open UTF-8 files!", MNP_APPNAME, MB_OK | MB_ICONWARNING);
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

inline void SaveConfig()
{
	std::ofstream out(exeFilePath + MNP_CONFIG_FILE);
	out << "# Edit this file to config MyNotePad\r\n";
	out << MNP_CONFIG_THEME << '=' << theme << "\r\n";
	out << MNP_CONFIG_WORDWRAP << '=' << word_wrap << "\r\n";
	out << MNP_CONFIG_LINENUMBER << '=' << show_line_number << "\r\n";
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	out << MNP_CONFIG_FONTNAME << '=' << cvt.to_bytes(MNP_FONTFACE);
	out.close();
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

	SaveConfig();
}

void OnMenuTheme(UINT IDM_THEME)
{
	if (IDM_THEME == IDM_THEMEWHITE)
		themeWhite();
	else if (IDM_THEME == IDM_THEMEDARK)
		themeDark();
	else return;

	if (show_line_number)
		line_number_font = std::make_unique<Font>(
			MNP_LINENUM_SIZE, MNP_LINENUM_FONTFACE, MNP_LINENUM_FONTCOLOR);

	HDC hdc = GetDC(hWnd);
	for (auto& l : article)
	{
		l.background_color = MNP_BGCOLOR_EDIT;
		repaintLine(hdc, l);
	}
	repaintSelectedLines();

	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);

	SaveConfig();
}

void OnMenuLineNumber()
{
	show_line_number = !show_line_number;
	HMENU hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, IDM_LINENUMBER, show_line_number ? MF_CHECKED : MF_UNCHECKED);

	if (show_line_number)
		line_number_font = std::make_unique<Font>(
			MNP_LINENUM_SIZE, MNP_LINENUM_FONTFACE, MNP_LINENUM_FONTCOLOR);
	else
		MNP_PADDING_LEFT = 20;

	HDC hdc = GetDC(hWnd);
	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);

	SaveConfig();
}

void OnMenuFont(uint8_t font_id)
{
	HMENU hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, IDM_FONTNAME_0, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_FONTNAME_1, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_FONTNAME_2, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_FONTNAME_3, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_FONTNAME_4, MF_UNCHECKED);
	//CheckMenuItem(hMenu, IDM_FONTNAME_5, MF_UNCHECKED);
	switch (font_id)
	{
	case 0:
		MNP_FONTFACE = L"Microsoft Yahei UI";
		CheckMenuItem(hMenu, IDM_FONTNAME_0, MF_CHECKED);
		break;
	case 1:
		MNP_FONTFACE = L"Lucida Console";
		CheckMenuItem(hMenu, IDM_FONTNAME_1, MF_CHECKED);
		break;
	case 2:
		MNP_FONTFACE = L"Courier New";
		CheckMenuItem(hMenu, IDM_FONTNAME_2, MF_CHECKED);
		break;
	case 3:
		MNP_FONTFACE = L"Consolas";
		CheckMenuItem(hMenu, IDM_FONTNAME_3, MF_CHECKED);
		break;
	/*case 4:
		MNP_FONTFACE = L"Fira Code";
		CheckMenuItem(hMenu, IDM_FONTNAME_4, MF_CHECKED);
		break;*/
	default:
		break;
	}

	HDC hdc = GetDC(hWnd);
	Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
	f.bind(hdc);
	// since font face has been changed, we have to update character width
	for (auto& l : article)
	{
		for (Character& c : l.sentence) 
			GetCharWidth32W(hdc, c.c, c.c, &c.width);
	}
	f.unbind();
	// force to adjust line-width
	resized = true;

	OnPaint(hdc);
	ReleaseDC(hWnd, hdc);
	SaveConfig();
}

inline void OnMenuShowInBrowser()
{
	std::wstring pathname = exeFilePath + L"\\preview.html";
	saveHTML(pathname);

	ShellExecute(hWnd, L"open", pathname.c_str(), NULL, NULL, SW_SHOWNORMAL);
}