#pragma once
#include <cstdint>
#include <string>

struct COLOR {
	DWORD c;
	COLOR(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) :
		c(a<<24|b<<16|g<<8|r) { }
	COLOR(DWORD c) :c(c) {}
	operator DWORD() {return c;}
};

class GDIUtil
{
public:
	static void fill(HDC hdc, COLOR color, int left, int top, int width, int height) {
		if (width == 0 || height == 0)
			return;
		RECT rc = { left, top, left + width, top + height };
		SetBkColor(hdc, color);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	static void line(HDC hdc, COLOR color, int x1, int y1, int x2, int y2)
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
	COLOR color;
	CharStyle style;

	Font(int size, LPCTSTR fontname, COLOR color = 0x00000000, CharStyle style = default_style)
		:color(color),
		style(style)
	{
		hf = CreateFont(size, 0, 0, 0, style.bold ? 700 : 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
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

	// Print a line without '\n'
	Font& printLine(const std::wstring& str, int left, int top) {
		_ASSERT(str.find(L'\n') == std::string::npos);

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, color);

		std::wstring temp;
		for (wchar_t c : str)
			if (c != L'\t')
				temp += c;
			else
				temp += L"    ";
	
		TextOutW(hdc, left, top, temp.c_str(), temp.length());
		return *this;
	}

	~Font() {
		DeleteObject(hf);
	}
};
