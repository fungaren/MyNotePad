#pragma once
#include <list>
#include <tuple>
#include <memory>

class GDIUtil
{
public:
	static void fill(HDC hdc, DWORD color, int left, int top, int width, int height) {
		if (width == 0 || height == 0)
			return;
		RECT rc = { left, top, left + width, top + height };
		SetBkColor(hdc, color);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	static void line(HDC hdc, DWORD color, int x1, int y1, int x2, int y2)
	{
		_ASSERT(x1 != x2 || y1 != y2);
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
	const unsigned int format = DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_NOCLIP | DT_HIDEPREFIX;

public:
	DWORD color;
	CharStyle style;	// unuse now

	Font(int size, LPCTSTR fontname, int weight = 100, DWORD color = 0x00000000, CharStyle style = default_style)
		:color(color),
		style(style)
	{
		hf = CreateFont(size, 0, 0, 0, weight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_SWISS, fontname);
	}

	Font& bind(HDC hdc) {
		this->hdc = hdc;
		ho = SelectObject(hdc, hf);
		return *this;
	}

	void unbind() {
		SelectObject(hdc, ho);
	}
	
	//// calc expected height for specific text width
	//int calcHeight(const std::wstring& str, int width)
	//{
	//	RECT rc = { 0,0,width,0 };
	//	DrawTextW(hdc, str.c_str(), str.length(), &rc, format | DT_CALCRECT);
	//	return rc.bottom - rc.top;
	//}

	//Font& drawText(const std::wstring& str, int left, int top, int width, int height) {
	//	RECT rc = { left, top, left + width, top + height };

	//	SetBkMode(hdc, TRANSPARENT);
	//	SetTextColor(hdc, color);

	//	DrawTextW(hdc, str.c_str(), str.length(), &rc, format);
	//	return *this;
	//}

	Font& printLine(const std::wstring& str, int left, int top) {
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, color);

		TextOutW(hdc, left, top, str.c_str(), str.length());
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
		:c(c),
		color(color),
		style(style),
		width(0)
	{ }

	Character(TCHAR c)
		:c(c),
		color(MNP_FONTCOLOR),
		style(default_style),
		width(0)
	{ }

	operator TCHAR() const { return c; }
};

typedef std::list<Character> Sentence;

struct Line
{
	mutable size_t text_width;
	mutable size_t text_height;
	size_t padding_left;
	size_t padding_top;
	mutable std::shared_ptr<MemDC> mdc;
	DWORD background_color;
	Sentence sentence;

	// A child line represent it's not a real line.
	// that means there was a long text without return,
	// and word-wrap was set, so we split it into serveral lines.
	// The first line and an empty line can not be child line.
	bool child_line;

	explicit Line(bool child_line = false)
		:background_color(MNP_BGCOLOR_EDIT),
		text_width(0),
		text_height(MNP_LINEHEIGHT),
		padding_left(0),
		padding_top(0),
		child_line(child_line),
		mdc(nullptr)
	{ }

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

	// return false if no char deleted
	bool eraseChar()
	{
		if (c == l->sentence.end())
		{
			if (l == --a.end())
				return false;
			
			if (c != l->sentence.begin())
				--c;

			Article::iterator next_line = l;
			l->sentence.splice(l->sentence.end(), (++next_line)->sentence);

			if (c != l->sentence.end())
				++c;
			else
				c = l->sentence.begin();

			a.erase(next_line);
			return true;
		}
		else
		{
			c = l->sentence.erase(c);
			return true;
		}
	}

	// return false if no char deleted
	bool backspace()
	{
		if (c == l->sentence.begin())
		{
			if (l == a.begin())
				return false;
			Article::iterator previous_line = l;

			c = (--previous_line)->sentence.end();
			if (c != previous_line->sentence.begin())
				--c;

			previous_line->sentence.splice(previous_line->sentence.end(), l->sentence);

			if (c != previous_line->sentence.end())
				++c;
			else
				c = previous_line->sentence.begin();

			a.erase(l);
			l = previous_line;
			return true;
		}
		else
		{
			c = l->sentence.erase(--c);
			return true;
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
			c = l->sentence.erase(c, l->sentence.end());
			Sentence::iterator t_c = c;
			if (t_c != l->sentence.begin())
				--t_c;

			l->sentence.splice(c, to.l->sentence, to.c, to.l->sentence.end());

			if (t_c != l->sentence.end())
				++t_c;
			else
				t_c = l->sentence.begin();

			Article::iterator t_l = l;
			a.erase(++t_l, ++to.l);

			to.l = l;
			to.c = c = t_c;
		}
		else	// backward selection
		{
			to.c = to.l->sentence.erase(to.c, to.l->sentence.end());
			Sentence::iterator t_c = to.c;
			if (t_c != to.l->sentence.begin())
				--t_c;

			to.l->sentence.splice(to.c, l->sentence, c, l->sentence.end());

			if (t_c != to.l->sentence.end())
				++t_c;
			else
				t_c = to.l->sentence.begin();

			Article::iterator t_l = to.l;
			to.a.erase(++t_l, ++l);

			l = to.l;
			c = to.c = t_c;
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
			Article::iterator i = l;
			if (!(++i)->child_line)
				str.push_back(L'\n');
			while (i != to.l)
			{
				str.append(i->sentence.begin(), i->sentence.end());
				if (!(++i)->child_line)
					str.push_back(L'\n');
			}
			str += std::wstring(to.l->sentence.begin(), to.c);
			return str;
		}
		else	// backward selection
		{
			std::wstring str(to.c, to.l->sentence.end());
			Article::iterator i = to.l;
			if (!(++i)->child_line)
				str.push_back(L'\n');
			while (i != to.l)
			{
				str.append(i->sentence.begin(), i->sentence.end());
				if (!(++i)->child_line)
					str.push_back(L'\n');
			}
			str += std::wstring(l->sentence.begin(), c);
			return str;
		}
	}
	
	void insertCharacter(const Character& ch)
	{
		if (ch.c == L'\n')
		{
			Article::iterator old_l = l;
			a.insert(++l, Line());
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
	
	// rebond child lines with their parents surrounding the cursor.
	void rebond()
	{
		size_t count = std::distance(l->sentence.begin(), c);
		while (l->child_line)
		{
			_ASSERT(l != a.begin());
			--l;
			count += l->sentence.size();
		}
		Article::iterator a_iter = l;
		for (++a_iter; a_iter != article.end() && a_iter->child_line; ++a_iter)
			l->sentence.splice(l->sentence.end(), a_iter->sentence);
		l = --article.erase(++l, a_iter);
		c = l->sentence.begin;
		std::advance(c, count);
	}

	void rebond_all()
	{
		rebond();
		Article::iterator i = article.begin();
		while (i != article.end())
		{
			Article::iterator a_iter = i;
			for (++a_iter; a_iter != article.end() && a_iter->child_line; ++a_iter)
				i->sentence.splice(l->sentence.end(), a_iter->sentence);
			i = article.erase(++i, a_iter);
		}
	}

private:
	Article::const_iterator& _split_long_text(size_t max_width, Article::iterator parent_line)
	{
		_ASSERT(!parent_line->child_line);
		Article::iterator a_iter = parent_line;
		const bool cursor_in_this_line = (parent_line == l);
		bool cursor_was_found = false;

		for (size_t width = 0; ; width = 0)
		{
			Sentence::iterator s_iter = a_iter->sentence.begin();
			while (s_iter != a_iter->sentence.end() && (width += s_iter->width) < max_width)
			{
				if (cursor_in_this_line && s_iter == c)
					cursor_was_found = true;
				++s_iter;
			}
			if (s_iter != a_iter->sentence.end())
			{
				Article::iterator i = a_iter;
				a_iter = a.insert(++a_iter, Line(true));
				a_iter->sentence.splice(a_iter->sentence.end(), i->sentence, s_iter, i->sentence.end());
				if (cursor_in_this_line && !cursor_was_found)
					l = a_iter;
			}
			else
				return a_iter;
		}
	}

public:
	// split a very long string surrounding the cursor without return
	// into serveral lines. Must call rebond() before calling this.
	// return an iterator pointing to a parent line below the last child line
	Article::const_iterator& split_long_text(size_t max_width)
	{
		return _split_long_text(max_width, l);
	}

	// split all long string without return into serveral lines.
	// Must call rebond_all() before calling this.
	void split_all_long_text(size_t max_width)
	{
		for (Article::iterator a_iter = a.begin(); a_iter!=a.end(); ++a_iter)
		{
			_split_long_text(max_width, a_iter);
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
