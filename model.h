#pragma once
#include <list>

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

	operator HDC() { return compatibleDC; }

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
	UINT format = DT_NOPREFIX;

public:
	DWORD color = 0x00000000;
	CharStyle style;	// unuse now

	Font(int size, LPCTSTR fontname, int weight = 100, CharStyle style = default_style) {
		hf = CreateFont(size, 0, 0, 0, weight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_SWISS, fontname);
		this->style = style;
	}

	Font& setColor(DWORD color) {
		this->color = color;
		return *this;
	}

	Font& setWordBreak() {
		format |= DT_WORDBREAK | DT_EDITCONTROL;
		return *this;
	}

	Font& bind(HDC hdc) {
		this->hdc = hdc;
		ho = SelectObject(hdc, hf);
		return *this;
	}

	void unbind() {
		SelectObject(hdc, ho);
	}

	Font& calcPrintArea(LPCTSTR str, int length, int* width, int* height) {
		RECT rc = { 0,0,0,0 };
		DrawTextW(hdc, str, length, &rc, format | DT_CALCRECT);
		*width = rc.right - rc.left;
		*height = rc.bottom - rc.top;
		return *this;
	}

	Font& print(LPCTSTR str, int length, int left, int top, int width, int height) {
		RECT rc = { left, top, left + width, top + height };

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, color);

		DrawTextW(hdc, str, length, &rc, format);
		return *this;
	}

	~Font() {
		DeleteObject(hf);
	}
};

struct Character
{
	TCHAR c;
	uint16_t width;
	DWORD color;
	CharStyle style;

	Character (TCHAR c, Font& f)
	{
		this->c = c;
		width = ;
		color = f.color;
		style = f.style;
	}
};

struct Line
{
	uint16_t width;
	uint16_t height;
	uint16_t padding_left;
	uint16_t padding_top;
	bool transparent;
	DWORD background_color;
	std::list<Character> characters;

	Line(std::wstring& s, uint16_t padding_left = 0, uint16_t padding_top = 0)
		:transparent(true),
		background_color(MNP_BGCOLOR_EDIT)
	{
		for (TCHAR c : s)
			characters.push_back(Character(c, ));
		this->padding_left = padding_left
		this->padding_top = padding_top;
		this->height = ;
	}
};

struct Position
{
	uint32_t n_line;
	uint32_t n_character;

	Position(uint32_t n_line, uint32_t n_character)
	{
		this->n_line = n_line;
		this->n_character = n_character;
	}
};