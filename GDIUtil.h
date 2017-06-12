#pragma once
#include <windows.h>

class GDIUtil
{
public :

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

	MemDC(HDC hdc, int width, int height):hdc(hdc) {
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

class Font {
	DWORD clr = 0x00000000;
	HFONT hf;
	HGDIOBJ ho;
	HDC hdc;
	UINT format = DT_NOPREFIX;

public:

	Font(int size, LPCTSTR fontname, int weight = 100, BOOL itatic = FALSE) {
		hf = CreateFont(size, 0, 0, 0, weight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_SWISS, fontname);
	}

	Font& setColor(DWORD color) {
		clr = color;
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
		SetTextColor(hdc, clr);
		
		DrawTextW(hdc, str, length, &rc, format);
		return *this;
	}

	~Font() {
		DeleteObject(hf);
	}
};