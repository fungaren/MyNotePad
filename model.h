#pragma once
#include <list>
#include <tuple>
#include <memory>

struct Character
{
	char32_t c;
	int width;
	COLOR color;
	CharStyle style;

	Character(char32_t c, COLOR color, CharStyle style)
		:c(c),
		color(color),
		style(style),
		width(0)
	{ }

	Character(char32_t c)
		:c(c),
		color(MNP_FONTCOLOR),
		style(default_style),
		width(0)
	{ }

	operator char32_t() const { return c; }


	typedef std::list<Character> Sentence;

	static inline std::wstring to_wstring(Sentence::const_iterator from, Sentence::const_iterator to)
	{
		std::wstring s;
		for (Sentence::const_iterator c = from; c != to; ++c)
			if (c->c > 0xFFFF)
			{
				s += (wchar_t)(c->c >> 16);
				s += (wchar_t)c->c;
			}
			else
				s += (wchar_t)c->c;
		return s;
	}

	static inline std::u32string to_u32string(Sentence::const_iterator from, Sentence::const_iterator to)
	{
		return std::u32string(from, to);
	}
};

typedef std::list<Character> Sentence;

struct Line
{
	mutable size_t text_width;	// the width of this line, will be recalculated after modified
	mutable size_t text_height;
	size_t padding_left;
	size_t padding_top;
	mutable std::shared_ptr<MemDC> mdc;
	COLOR background_color;
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
		return Character::to_wstring(sentence.begin(),sentence.end());
	}

	operator std::u32string() const
	{
		return Character::to_u32string(sentence.begin(), sentence.end());
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
		else if (--c == l->sentence.begin() && l->child_line)
		{
			--l;
			toLastChar();
		}
	}

	void move_right()
	{
		// the last char in this line, go to the beginning of next line
		if (c == l->sentence.end())
		{
			if (l == --a.end())
				return;	// end of all, nothing to do
			else if ((++l)->child_line)
				c = ++(l->sentence.begin());
			else
				toFirstChar();
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
			{
				++c;
				if (next_line->child_line)
					c = l->sentence.erase(c);
			}
			else
				c = l->sentence.begin();// an empty line can not be a child line

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
				return Character::to_wstring(to.c, c);
			else if (dist_x < 0)
				return Character::to_wstring(c, to.c);
			else
				return L"";
		}
		else if (dist_y < 0)	// forward selection
		{
			std::wstring str = Character::to_wstring(c, l->sentence.end());
			Article::iterator i = l;
			if (!(++i)->child_line)
				str.push_back(L'\n');
			while (i != to.l)
			{
				str.append(i->sentence.begin(), i->sentence.end());
				if (!(++i)->child_line)
					str.push_back(L'\n');
			}
			str += Character::to_wstring(to.l->sentence.begin(), to.c);
			return str;
		}
		else	// backward selection
		{
			std::wstring str = Character::to_wstring(to.c, to.l->sentence.end());
			Article::iterator i = to.l;
			if (!(++i)->child_line)
				str.push_back(L'\n');
			while (i != l)
			{
				str.append(i->sentence.begin(), i->sentence.end());
				if (!(++i)->child_line)
					str.push_back(L'\n');
			}
			str += Character::to_wstring(l->sentence.begin(), c);
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
				// cursor on the middle of the line,
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
	
	// count characters before cursor in the "real line"
	// and set cursor to the beginning of parent line.
	size_t count_char()
	{
		size_t count = std::distance(l->sentence.begin(), c);
		while (l->child_line)
		{
			_ASSERT(l != a.begin());
			--l;
			count += l->sentence.size();
		}
		c = l->sentence.begin();
		return count;
	}

	// rebond child lines with their parent surrounding the cursor,
	// and set cursor to the corresponding position in parent line.
	void rebond()
	{
		size_t count = count_char();
		Article::iterator a_iter = l;
		for (++a_iter; a_iter != a.end() && a_iter->child_line; ++a_iter)
			l->sentence.splice(l->sentence.end(), a_iter->sentence);

		l = --a.erase(++l, a_iter);
		std::advance(c, count);
	}

	void rebond_all()
	{
		rebond();
		Article::iterator i = a.begin();
		while (i != a.end())
		{
			Article::iterator a_iter = i;
			for (++a_iter; a_iter != a.end() && a_iter->child_line; ++a_iter)
				i->sentence.splice(i->sentence.end(), a_iter->sentence);
			i = a.erase(++i, a_iter);
		}
	}

private:
	Article::iterator _split_long_text(size_t max_width, Article::iterator parent_line)
	{
		if (parent_line->child_line)
			return ++parent_line; // another cursor called split_long_text() before
		Article::iterator a_iter = parent_line;
		// cursor in this line
		if (parent_line == l)
		{
			if (l->sentence.end() == c)
			{
				for (size_t width = 0; ; width = 0)
				{
					Sentence::iterator s_iter = a_iter->sentence.begin();
					while (s_iter != a_iter->sentence.end() && (width += s_iter->width) < max_width)
					{
						++s_iter;
					}
					if (s_iter != a_iter->sentence.end())
					{
						Article::iterator i = a_iter;
						a_iter = a.insert(++a_iter, Line(true));
						a_iter->sentence.splice(a_iter->sentence.end(), i->sentence, s_iter, i->sentence.end());
						l = a_iter;
						c = a_iter->sentence.end();
					}
					else
						return ++a_iter;
				}
			}
			else
			{
				bool cursor_was_found = false;

				for (size_t width = 0; ; width = 0)
				{
					Sentence::iterator s_iter = a_iter->sentence.begin();
					while (s_iter != a_iter->sentence.end() && (width += s_iter->width) < max_width)
					{
						if (!cursor_was_found && s_iter == c)
						{
							cursor_was_found = true;
							// if cursor at the beginning of child line, move to the end of
							// previous line (can be either child line or parent line)
							if (a_iter->child_line && c == a_iter->sentence.begin())
							{
								l = a_iter;
								c = (--l)->sentence.end();
							}
						}
						++s_iter;
					}
					if (s_iter != a_iter->sentence.end())
					{
						Article::iterator i = a_iter;
						a_iter = a.insert(++a_iter, Line(true));
						a_iter->sentence.splice(a_iter->sentence.end(), i->sentence, s_iter, i->sentence.end());
						if (!cursor_was_found)
							l = a_iter;
					}
					else
						return ++a_iter;
				}
			}
		}
		else // cursor is not in this line
		{
			for (size_t width = 0; ; width = 0)
			{
				Sentence::iterator s_iter = a_iter->sentence.begin();
				while (s_iter != a_iter->sentence.end() && (width += s_iter->width) < max_width)
				{
					++s_iter;
				}
				if (s_iter != a_iter->sentence.end())
				{
					Article::iterator i = a_iter;
					a_iter = a.insert(++a_iter, Line(true));
					a_iter->sentence.splice(a_iter->sentence.end(), i->sentence, s_iter, i->sentence.end());
				}
				else
					return ++a_iter;
			}
		}
	}

public:
	// Split a very long string surrounding the cursor without return
	// into serveral lines. Must call rebond() before calling this.
	// Return an iterator pointing to a parent line below the last child line.
	Article::const_iterator split_long_text(size_t max_width)
	{
		return _split_long_text(max_width, l);
	}

	// Split all long string without return into serveral lines.
	// Must call rebond_all() before calling this.
	void split_all_long_text(size_t max_width)
	{
		for (Article::iterator a_iter = a.begin(); a_iter!=a.end();)
		{
			a_iter = _split_long_text(max_width, a_iter);
		}
	}

	// On warp-line mode, if something selected while the
	// edit area size changed, we have to rebond_all()
	// and split_all_long_text(), and make sure the
	// two cursors in the correct states.
	void reform(size_t max_width, Cursor& right)
	{
		size_t count = count_char();
		Article::iterator parent_line = l;

		right.rebond_all();
		right.split_all_long_text(max_width);

		l = parent_line;
		c = parent_line->sentence.begin();

		for (size_t i = 0; i < count; ++i)
			move_right();
	}

	// if something selected while exiting warp-line mode,
	// we have to rebond_all() and make sure the
	// two cursors in the correct states.
	void recover(Cursor& right)
	{
		size_t count = count_char();
		Article::iterator parent_line = l;

		right.rebond_all();

		l = parent_line;
		c = parent_line->sentence.begin();

		std::advance(c, count);
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
