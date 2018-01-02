#pragma once
#include <list>
#include <tuple>

class GDIUtil
{
public:

	static void fill(HDC hdc, DWORD color, int left, int top, int width, int height) {
		RECT rc = { left, top, left + width, top + height };
		SetBkColor(hdc, color);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	static void line(HDC hdc, DWORD color, int x1, int y1, int x2, int y2)
	{
		HPEN hp = CreatePen(BS_SOLID, 1, color);
		HGDIOBJ ho = SelectObject(hdc, hp);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
		SelectObject(hdc, ho);
		DeleteObject(hp);
	}
};

class MemDC {
	HDC hdc;
	HDC compatibleDC;
	HBITMAP bmp;
	HBITMAP oldBmp;

public:

	MemDC(HDC hdc, int width, int height) :hdc(hdc) {
		compatibleDC = CreateCompatibleDC(NULL);

		bmp = CreateCompatibleBitmap(hdc, width, height);
		oldBmp = (HBITMAP)SelectObject(compatibleDC, bmp);
	}

	operator HDC() const { return compatibleDC; }

	~MemDC() {
		SelectObject(compatibleDC, oldBmp);
		DeleteObject(oldBmp);
		DeleteObject(bmp);
		DeleteDC(compatibleDC);
	}
};

struct CharStyle
{
	uint8_t bold : 1;
	uint8_t itatic : 1;
	uint8_t underline : 1;
	uint8_t reserved : 5;
};

const CharStyle default_style = { 0 };

class Font {
	HFONT hf;
	HGDIOBJ ho;
	HDC hdc;
	// UINT format = DT_NOPREFIX;

public:
	DWORD color;
	CharStyle style;	// unuse now

	Font(int size, LPCTSTR fontname, int weight = 100, DWORD color = 0x00000000, CharStyle style = default_style)
		:color(color)
	{
		hf = CreateFont(size, 0, 0, 0, weight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_SWISS, fontname);
		this->style = style;
	}

	Font& bind(HDC hdc) {
		this->hdc = hdc;
		ho = SelectObject(hdc, hf);
		return *this;
	}

	void unbind() {
		SelectObject(hdc, ho);
	}

	//Font& setWordBreak() {
	//	format |= DT_WORDBREAK | DT_EDITCONTROL;
	//	return *this;
	//}

	//Font& calcPrintArea(LPCTSTR str, int length, int* width, int* height) {
	//	RECT rc = { 0,0,0,0 };
	//	DrawTextW(hdc, str, length, &rc, format | DT_CALCRECT);
	//	*width = rc.right - rc.left;
	//	*height = rc.bottom - rc.top;
	//	return *this;
	//}

	//Font& print(LPCTSTR str, int length, int left, int top, int width, int height) {
	//	RECT rc = { left, top, left + width, top + height };

	//	SetBkMode(hdc, TRANSPARENT);
	//	SetTextColor(hdc, color);

	//	DrawTextW(hdc, str, length, &rc, format);
	//	return *this;
	//}

	Font& printLine(LPCTSTR str, size_t length, int left, int top) {
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, color);

		TextOutW(hdc, left, top, str, length);
		return *this;
	}

	~Font() {
		DeleteObject(hf);
	}
};

struct Character
{
	TCHAR c;
	int width;
	DWORD color;
	CharStyle style;

	Character(TCHAR c, DWORD color, CharStyle style)
	{
		this->c = c;
		this->color = color;
		this->style = style;
		
#ifdef DEBUG
		width = 0;
#endif // DEBUG
	}

	Character(TCHAR c)
	{
		this->c = c;
		this->color = MNP_FONTCOLOR;
		this->style = { 0,0 };

#ifdef DEBUG
		width = 0;
#endif // DEBUG
	}
	operator TCHAR() const { return c; }
};

typedef std::list<Character> Sentence;

struct Line
{
	uint16_t height;
	uint16_t padding_left;
	uint16_t padding_top;
	bool transparent;
	DWORD background_color;
	Sentence sentence;

	explicit Line(const std::wstring& s, uint16_t padding_left = 0, uint16_t padding_top = 0)
		:transparent(true),
		background_color(MNP_BGCOLOR_EDIT),
		height(MNP_LINEHEIGHT),
		padding_left(padding_left),
		padding_top(padding_top)
	{
		HDC hdc = GetDC(hWnd);
		Font f(MNP_FONTSIZE, MNP_FONTFACE, MNP_FONTCOLOR);
		f.bind(hdc);
		for (TCHAR c : s)
		{
			Character ch(c, MNP_FONTCOLOR, { 0,0 });
			GetCharWidth32W(hdc, ch.c, ch.c, &ch.width);
			sentence.push_back(ch);
		}
		f.unbind();
		ReleaseDC(hWnd, hdc);
	}

	operator std::wstring() const 
	{
		return std::wstring(sentence.begin(), sentence.end());
	}
};

typedef std::list<Line> Article;

class Cursor
{
	Article& a;
	Article::iterator l;		// index of the first line is 0.
	Sentence::iterator c;		// index of the first char is 0.

public:
	Cursor(Article& a) : a(a)
	{
		reset();
	}

	Cursor(Article& a, Article::iterator l, Sentence::iterator c) :
		a(a),
		l(l),
		c(c)
	{
		;
	}

	void reset()
	{
		toFirstLine();
		toFirstChar();
	}

	void end()
	{
		toLastLine();
		toLastChar();
	}

	void toFirstLine()
	{
		l = a.begin();
	}

	void toLastLine()
	{
		l = --a.end();
	}

	void toFirstChar()
	{
		c = l->sentence.begin();
	}

	void toLastChar()
	{
		c = l->sentence.end();
	}
	
	void move_left()
	{
		if (c == l->sentence.begin())
		{
			// the first char in this line
			if (l == a.begin())
				return;	// first of all, nothing to do
			else
			{
				// not the first line, back to the end of previous line
				--l;
				toLastChar();
			}
		}
		else
			--c;
	}

	void move_right()
	{
		// the last char in this line, go to the beginning of next line
		if (c == l->sentence.end())
		{
			if (l == --a.end())
				return;	// end of all, nothing to do
			else
			{
				// not the last line, to the first of next line
				++l;
				toFirstChar();
			}
		}
		else
			++c;
	}

	Article::const_iterator getSentence() const
	{
		return l;
	}
	
	Sentence::const_iterator getCharacter() const
	{
		return c;
	}
	
	void eraseChar() 
	{
		Article::iterator next_line = l;
		if (c == l->sentence.end())
		{
			l->sentence.splice(l->sentence.end(), (++next_line)->sentence);
			a.erase(next_line);
		}
		else
			c = l->sentence.erase(c);
	}

	void backspace()
	{
		if (c == l->sentence.begin())
		{
			if (l == a.begin())
				return;
			Article::iterator previous_line = l;
			c = (--previous_line)->sentence.end();
			previous_line->sentence.splice(previous_line->sentence.end(), l->sentence);
			a.erase(l);
			l = previous_line;
		}
		else
		{
			Sentence::iterator t = c;
			l->sentence.erase(--t);
		}
	}

	int distance_x(const Cursor& right) const
	{
		_ASSERT(l == right.l);
		return (int)(std::distance(l->sentence.begin(), c) -
			std::distance(l->sentence.begin(), right.c));
	}

	int distance_y(const Cursor& right) const
	{
		return (int)(std::distance(a.begin(), l) -
			std::distance(a.begin(), right.l));
	}
	
	// return true for success, false for no selected char.
	bool removeSelectedChars(Cursor& to) {
		int dist_y = distance_y(to);
		if (dist_y == 0)
		{
			int dist_x = distance_x(to);
			if (dist_x < 0)
				c = l->sentence.erase(c, to.c);
			else if (dist_x > 0)
				to.c = to.l->sentence.erase(to.c, c);
			else
				return false;
		}
		else if (dist_y < 0)	// forward selection
		{
			auto t_c = c;
			if (t_c != l->sentence.begin())
				--t_c;
			c = l->sentence.erase(c, l->sentence.end());
			l->sentence.splice(c, to.l->sentence, to.c, to.l->sentence.end());
			auto t_l = l;
			a.erase(++t_l, ++to.l);
			to.l = l;
			to.c = c = ++t_c;
		}
		else	// backward selection
		{
			auto t_c = to.c;
			if (t_c != to.l->sentence.begin())
				--t_c;
			to.c = to.l->sentence.erase(to.c, to.l->sentence.end());
			to.l->sentence.splice(to.c, l->sentence, c, l->sentence.end());
			auto t_l = to.l;
			to.a.erase(++t_l, ++l);
			l = to.l;
			c = to.c = ++t_c;
		}
		return true;
	}

	// return the selected characters
	std::wstring getSelectedChars(Cursor& to) {
		int dist_y = distance_y(to);
		if (dist_y == 0)
		{
			int dist_x = distance_x(to);
			if (dist_x > 0)
				return std::wstring(to.c, c);
			else if (dist_x < 0)
				return std::wstring(c, to.c);
			else
				return L"";
		}
		else if (dist_y < 0)	// forward selection
		{
			std::wstring str(c, l->sentence.end());
			auto i = l;
			for (++i; i != to.l; ++i)
				str.append(i->sentence.begin(), i->sentence.end());
			str += std::wstring(to.l->sentence.begin(), to.c);
			return str;
		}
		else	// backward selection
		{
			std::wstring str(to.c, to.l->sentence.end());
			auto& i = to.l;
			for (++i; i != l; ++i)
				str.append(i->sentence.begin(), i->sentence.end());
			str += std::wstring(l->sentence.begin(), c);
			return str;
		}
	}
	
	void insertCharacter(const Character& ch)
	{
		if (ch.c == L'\n')
		{
			Article::iterator old_l = l;
			a.insert(++l, Line(L""));
			--l;// go to the new line
			if (c != old_l->sentence.end()) {
				// cursor at the middle of the line,
				// move the string after the cursor to new line
				l->sentence.splice(l->sentence.end(), old_l->sentence, c, old_l->sentence.end());
			}
			c = l->sentence.begin();
		}
		else
		{
			l->sentence.insert(c, ch);
		}
	}
	
	//size_t getLineLength(Article& a) const
	//{
	//	if (!l->sentence.empty() && l->sentence.back().c == L'\n')
	//		return l->sentence.size() - 1;	// - 1 for '\n'
	//	else
	//		return l->sentence.size();
	//}

	bool operator== (const Cursor& right) const 
	{
		return (l == right.l) && (c == right.c);
	}

	bool operator!= (const Cursor& right) const
	{
		return (l != right.l) || (c != right.c);
	}

	Cursor& operator= (const Cursor& right)
	{
		l = right.l;
		c = right.c;
		return *this;
	}
};
