/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    01.11.2020
   @brief   console class

*******************************************************************************

   Copyright (c) 2018-2020 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

******************************************************************************/

#pragma once

#include <windows.h>
#include <algorithm>

class Console
{
public:

	enum: char
	{
		LightShade     = '\xB0',
		MediumShade    = '\xB1',
		DarkShade      = '\xB2',
		FullBlock      = '\xDB',
		LowerHalfBlock = '\xDC',
		UpperHalfBlock = '\xDF',
		BlackSquare    = '\xFE',
	};

	enum Color: WORD
	{
		Black       = 0x0,
		Blue        = 0x1,
		Green       = 0x2,
		Cyan        = 0x3,
		Red         = 0x4,
		Purple      = 0x5,
		Orange      = 0x6,
		LightGrey   = 0x7,
		Grey        = 0x8,
		LightBlue   = 0x9,
		LightGreen  = 0xA,
		LightCyan   = 0xB,
		LightRed    = 0xC,
		LightPurple = 0xD,
		Yellow      = 0xE,
		White       = 0xF,
		Default     = LightGrey,
	};

	struct Rectangle
	{
		const int x, y, width, height;
		const int left, top, right, bottom;

		Rectangle(int _x, int _y, int _w, int _h):
			x(_x), y(_y), width(_w), height(_h),
			left(_x), top(_y), right(_x + _w - 1), bottom(_y + _h - 1) {}

		bool contains( const int _x, const int _y ) const
		{
			return _x >= left && _x <= right && _y >= top && _y <= bottom;
		}

		int Center( const int _w ) const
		{
			return x + (width - _w) / 2;
		}

		int Right( const int _w ) const
		{
			return x + width - _w;
		}

		int Middle( const int _h ) const
		{
			return y + (height - _h) / 2;
		}

		int Bottom( const int _h ) const
		{
			return y + height - _h;
		}
	};

private:

	HWND   hwnd_;
	HANDLE cin_;
	HANDLE cout_;
	HANDLE cerr_;

	CONSOLE_FONT_INFOEX cfi_;

	enum Bar: unsigned
	{
		NoBar    = 0,
		RightBar = 1,
		DownBar  = 2,
		LeftBar  = 4,
		UpBar    = 8,
		AllBars  = RightBar | DownBar | LeftBar | UpBar
	};

	bool Open( LPCTSTR title )
	{
		hwnd_ = GetConsoleWindow();
		if (!hwnd_)
			return false;

		cin_  = GetStdHandle(STD_INPUT_HANDLE);
		cout_ = GetStdHandle(STD_OUTPUT_HANDLE);
		cerr_ = GetStdHandle(STD_ERROR_HANDLE);
		if (!cin_ || !cout_ || !cerr_)
			return false;

		if (!FlushConsoleInputBuffer(cin_))
			return false;

		cfi_.cbSize = sizeof(cfi_);
		if (!GetCurrentConsoleFontEx(cout_, FALSE, &cfi_))
			return false;

		DWORD mode;
		if (!GetConsoleMode(cin_, &mode))
			return false;

		mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		mode &= ~ENABLE_QUICK_EDIT_MODE;

		return SetConsoleMode(cin_, mode) &&
		       SetTitle(title);
	}

	template <typename T>
	void Reset( T * const a, T * const b ) const
	{
		if (a) *a = static_cast<T>(0);
		if (b) *b = static_cast<T>(0);
	}

	WORD MakeAttribute( const Color fore, const Color back ) const
	{
		return static_cast<WORD>((back * 16) | fore);
	}

	Color GetForeColor( const WORD a ) const
	{
		return static_cast<Color>(a % 16);
	}

	Color GetBackColor( const WORD a ) const
	{
		return static_cast<Color>((a / 16) % 16);
	}

	bool ColorHLine( const Rectangle &rc, const int y, const Color fore, const Color back = Black ) const
	{
		if (y < rc.top || y > rc.bottom || rc.width < 1)
			return false;

		for (int x = rc.left; x <= rc.right; x++)
			if (!Put(x, y, fore, back))
				return false;

		return true;
	}

	bool ColorVLine( const Rectangle &rc, const int x, const Color fore, const Color back = Black ) const
	{
		if (x < rc.left || x > rc.right || rc.height < 1)
			return false;

		for (int y = rc.top; y <= rc.bottom; y++)
			if (!Put(x, y, fore, back))
				return false;

		return true;
	}

	bool DrawHLine( const Rectangle &rc, const int y, const char * const box, const Bar b = NoBar ) const
	{
		if (y < rc.top || y > rc.bottom || rc.width <= 1)
			return false;

		int x = rc.left;
		Bar p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		if (!Put(x++, y, box[p | b | RightBar]))
			return false;

		while (x < rc.right) {
			p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
			if (!Put(x++, y, box[p | LeftBar | RightBar]))
				return false;
		}

		p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		if (!Put(x++, y, box[p | b | LeftBar]))
			return false;

		return true;
	}

	bool DrawVLine( const Rectangle &rc, const int x, const char * const box, const Bar b = NoBar ) const
	{
		if (x < rc.left || x > rc.right || rc.height <= 1)
			return false;

		int y = rc.top;
		Bar p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		if (!Put(x, y++, box[p | b | DownBar]))
			return false;

		while (y < rc.bottom) {
			p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
			if (!Put(x, y++, box[p | UpBar | DownBar]))
				return false;
		}

		p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		if (!Put(x, y++, box[p | b | UpBar]))
			return false;

		return true;
	}

	bool DrawFrame( const Rectangle &rc, const char * const box ) const
	{
		if (rc.width < 1 || rc.height < 1)
			return false;

		if (rc.width > 1) {
			if (rc.height > 1)
				return DrawHLine(rc, rc.top, box, DownBar) &&
				       DrawVLine(rc, rc.left, box, RightBar) &&
				       DrawVLine(rc, rc.right, box, LeftBar) &&
				       DrawHLine(rc, rc.bottom, box, UpBar);
			else
				return DrawHLine(rc, rc.y, box);
		}
		else {
			if (rc.height > 1)
				return DrawVLine(rc, rc.x, box);
			else
				return false;
		}
	}

	bool DrawFrame( const int x, const int y, const int width, const int height, const char * const box ) const
	{
		const Rectangle rc(x, y, width, height);
		return DrawFrame(rc, box);
	}

public:

	const HWND   &Hwnd;
	const HANDLE &Cin;
	const HANDLE &Cout;
	const HANDLE &Cerr;

	Console( LPCTSTR title = NULL ): Hwnd(hwnd_), Cin(cin_), Cout(cout_), Cerr(cerr_)
	{
		if (!Open(title)) {
			cerr_ = cout_ = cin_ = NULL;
			hwnd_ = NULL;
 		}
	}

	~Console()
	{
		if (hwnd_) {
			FlushConsoleInputBuffer(cin_);
			SetCurrentConsoleFontEx(cout_, FALSE, &cfi_);
			ShowCursor();
			Maximize();
		}
	}

	bool operator!() const
	{
		return !hwnd_;
	}

	bool SetTitle( LPCTSTR title ) const
	{
		return !title || SetConsoleTitle(title);
	}

	bool Minimize() const
	{
		return ShowWindow(Hwnd, SW_MINIMIZE);
	}

	bool Maximize() const
	{
		const COORD size = GetLargestConsoleWindowSize(Cout);
		if (size.X == 0 && size.Y == 0)
			return false;

		return SetSize(size.X, size.Y) &&
		       ShowWindow(Hwnd, SW_MAXIMIZE);
	}

	bool Restore() const
	{
		return ShowWindow(Hwnd, SW_RESTORE);
	}

	bool Minimized() const
	{
		LONG result;
		result = GetWindowLong(Hwnd, GWL_STYLE);
		return (result & WS_MINIMIZE) != 0L;
	}

	bool Maximized() const
	{
		LONG result;
		result = GetWindowLong(Hwnd, GWL_STYLE);
		return (result & WS_MAXIMIZE) != 0L;
	}

	bool Windowed() const
	{
		LONG result;
		result = GetWindowLong(Hwnd, GWL_STYLE);
		return result != 0L && (result & (WS_MINIMIZE | WS_MAXIMIZE)) == 0L;
	}

	bool SetFullScreen( const bool fullscreen = true ) const
	{
		return SetConsoleDisplayMode(Cout, fullscreen ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE, NULL) &&
		       Maximize();
	}

	bool GetSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(width, height), false;

		if (width)  *width  = static_cast<int>(sbi.dwSize.X);
		if (height) *height = static_cast<int>(sbi.dwSize.Y);

		return true;
	}

	bool GetWndSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(width, height), false;

		if (width)  *width  = static_cast<int>(sbi.srWindow.Right - sbi.srWindow.Left + 1);
		if (height) *height = static_cast<int>(sbi.srWindow.Bottom - sbi.srWindow.Top + 1);

		return true;
	}

	bool GetMaxSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(width, height), false;

		if (width)  *width  = static_cast<int>(sbi.dwMaximumWindowSize.X);
		if (height) *height = static_cast<int>(sbi.dwMaximumWindowSize.Y);

		return true;
	}

	bool SetSize( const int width, const int height ) const
	{
		while (!Windowed())
			if (!Restore())
				return false;

		int w, h;
		if (!GetSize(&w, &h))
			return false;

		w = std::min(w, width);
		h = std::min(h, height);
		const SMALL_RECT temp = { 0, 0, static_cast<SHORT>(w - 1),     static_cast<SHORT>(h - 1) };
		const COORD      size = {       static_cast<SHORT>(width),     static_cast<SHORT>(height) };
		const SMALL_RECT rect = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };

		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		if (!GetCurrentConsoleFontEx(Cout, FALSE, &cfi))
			return false;

		const int fx = GetSystemMetrics(SM_CXFRAME);
		const int fy = GetSystemMetrics(SM_CYFRAME);
		const int cy = GetSystemMetrics(SM_CYCAPTION);
		if (fx == 0 || fy == 0 || cy == 0)
			return false;

		const int wx = width  * cfi.dwFontSize.X + fx * 2;
		const int hx = height * cfi.dwFontSize.Y + fy * 2 + cy;

		return SetConsoleWindowInfo(Cout, TRUE, &temp) &&
		       SetConsoleScreenBufferSize(Cout, size) &&
		       SetConsoleWindowInfo(Cout, TRUE, &rect) &&
		       SetWindowPos(Hwnd, HWND_TOP, 0, 0, wx, hx, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW) &&
		       Home();
	}

	bool Center() const
	{
		while (!Windowed())
			if (!Restore())
				return false;

		const int cx = GetSystemMetrics(SM_CXSCREEN);
		const int cy = GetSystemMetrics(SM_CYSCREEN);
		if (cx == 0 || cy == 0)
			return false;

		RECT rc;
		if (!GetWindowRect(Hwnd, &rc))
			return false;

		const int x = (cx - (rc.right - rc.left + 1)) / 2;
		const int y = (cy - (rc.bottom - rc.top + 1)) / 2;

		return SetWindowPos(Hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	bool Center( const int width, const int height ) const
	{
		return SetSize(width, height) &&
		       Center();
	}

	bool CenterUp() const
	{
		while (!Windowed())
			if (!Restore())
				return false;

		const int cx = GetSystemMetrics(SM_CXSCREEN);
		if (cx == 0)
			return false;

		RECT rc;
		if (!GetWindowRect(Hwnd, &rc))
			return false;

		const int x = (cx - (rc.right - rc.left + 1)) / 2;

		return SetWindowPos(Hwnd, HWND_TOP, x, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	bool CenterUp( const int width, const int height ) const
	{
		return SetSize(width, height) &&
		       CenterUp();
	}

	bool GetInput( INPUT_RECORD * const input = NULL, const DWORD len = 1 ) const
	{
		DWORD count;
		if (!GetNumberOfConsoleInputEvents(Cin, &count) || count == 0)
			return false;

		return !input || ReadConsoleInput(Cin, input, len, &count);
	}

	bool Clear( const Color fore = Default, const Color back = Black ) const
	{
		WORD a = MakeAttribute(fore, back);
		if (!SetConsoleTextAttribute(Cout, a))
			return false;

		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return false;

		const COORD home = { 0, 0 };
		const DWORD size = sbi.dwSize.X * sbi.dwSize.Y;
		DWORD count;

		return FillConsoleOutputCharacter(Cout, ' ', size, home, &count) &&
		       FillConsoleOutputAttribute(Cout,  a , size, home, &count) &&
		       Home();
	}

	bool Home() const
	{
		return SetCursorPos(0, 0);
	}

	bool HideCursor() const
	{
		CONSOLE_CURSOR_INFO cci;
		if (!GetConsoleCursorInfo(Cout, &cci))
			return false;

		cci.bVisible = FALSE;
		return SetConsoleCursorInfo(Cout, &cci);
	}

	bool ShowCursor() const
	{
		CONSOLE_CURSOR_INFO cci;
		if (!GetConsoleCursorInfo(Cout, &cci))
			return false;

		cci.bVisible = TRUE;
		return SetConsoleCursorInfo(Cout, &cci);
	}

	bool SetCursorSize( const DWORD size ) const
	{
		const CONSOLE_CURSOR_INFO cci = { size, true };
		if (size == 0)
			return HideCursor();
		else
			return SetConsoleCursorInfo(Cout, &cci);
	}

	bool GetCursorPos( int * const x, int * const y ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(x, y), false;

		if (x) *x = sbi.dwCursorPosition.X;
		if (y) *y = sbi.dwCursorPosition.Y;

		return true;
	}

	bool SetCursorPos( const int x, const int y ) const
	{
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		return SetConsoleCursorPosition(Cout, coord);
	}

	int GetFontSize() const
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		if (!GetCurrentConsoleFontEx(Cout, FALSE, &cfi))
			return 0;

		return static_cast<int>(cfi.dwFontSize.Y);
	}

	bool SetFont( const SHORT size, const WCHAR *name = NULL ) const
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		if (!GetCurrentConsoleFontEx(Cout, FALSE, &cfi))
			return false;

		cfi.dwFontSize = { 0, size };
		if (name) {
			cfi.FontWeight = FW_NORMAL;
			wcsncpy(cfi.FaceName, name, LF_FACESIZE);
		}

		return SetCurrentConsoleFontEx(Cout, FALSE, &cfi);
	}

	bool GetTextColor( Color * const fore, Color * const back = NULL ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(fore, back), false;

		if (fore) *fore = GetForeColor(sbi.wAttributes);
		if (back) *back = GetBackColor(sbi.wAttributes);

		return true;
	}

	bool SetTextColor( const Color fore, const Color back = Black ) const
	{
		return SetConsoleTextAttribute(Cout, MakeAttribute(fore, back));
	}

	bool SetText( const int x, const int y, const Color fore, const Color back = Black ) const
	{
		return SetCursorPos(x, y) &&
		       SetTextColor(fore, back);
	}

	char Get( const int x, const int y ) const
	{
		CHAR  c;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		if (!ReadConsoleOutputCharacter(Cout, &c, 1, coord, &count))
			return 0;

		return c;
	}

	char Get( const int x, const int y, Color * const fore, Color * const back = NULL ) const
	{
		WORD  a;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		if (!ReadConsoleOutputAttribute(Cout, &a, 1, coord, &count))
			return Reset(fore, back), 0;

		if (fore) *fore = GetForeColor(a);
		if (back) *back = GetBackColor(a);

		return Get(x, y);
	}

	bool Put( const int x, const int y, const Color fore, const Color back = Black ) const
	{
		WORD  t;
		const WORD  a = MakeAttribute(fore, back);
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		if (!ReadConsoleOutputAttribute(Cout, &t, 1, coord, &count))
			return false;

		return a == t || WriteConsoleOutputAttribute(Cout, &a, 1, coord, &count);
	}

	bool Put( const int x, const int y, const char c ) const
	{
		CHAR  t;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		if (!ReadConsoleOutputCharacter(Cout, &t, 1, coord, &count))
			return false;

		return c == t || WriteConsoleOutputCharacter(Cout, &c, 1, coord, &count);
	}

	bool Put( const int x, const int y, const char * const s ) const
	{
		int i = x;
		const char *p = s;
		while (*p)
			if (!Put(i++, y, *p++))
				return false;

		return true;
	}

	bool Put( const int x, const int y, const char c, const Color fore, const Color back = Black ) const
	{
		return Put(x, y, fore, back) &&
		       Put(x, y, c);
	}

	bool Put( const int x, const int y, const char * const s, const Color fore, const Color back = Black ) const
	{
		int i = x;
		const char *p = s;
		while (*p)
			if (!Put(i++, y, *p++, fore, back))
				return false;

		return true;
	}

	char Get() const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return 0;

		return Get(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y);
	}

	char Get( Color * const fore, Color * const back = NULL ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return Reset(fore, back), 0;

		return Get(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, fore, back);
	}

	bool Put( const char c ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return false;

		return Put(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, c);
	}

	bool Put( const Color fore, const Color back = Black ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		if (!GetConsoleScreenBufferInfo(Cout, &sbi))
			return false;

		return Put(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, fore, back);
	}

	bool ColorFrame( const Rectangle &rc, const Color fore, const Color back = Black ) const
	{
		return ColorHLine(rc, rc.top, fore, back) &&
		       ColorVLine(rc, rc.left, fore, back) &&
		       ColorVLine(rc, rc.right, fore, back) &&
		       ColorHLine(rc, rc.bottom, fore, back);
	}

	bool ColorFrame( const int x, const int y, const int width, const int height, const Color fore, const Color back = Black ) const
	{
		const Rectangle rc(x, y, width, height);
		return ColorFrame(rc, fore, back);
	}

	bool DrawSingle( const Rectangle &rc ) const
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xDA""\x20""\xC4""\xBF""\xC2""\x20""\xC0""\xB3""\xC3""\xD9""\xC1""\xB4""\xC5";
		return DrawFrame(rc, box);
	}

	bool DrawSingle( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		return DrawSingle(rc);
	}

	bool DrawDouble( const Rectangle &rc ) const
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xC9""\x20""\xCD""\xBB""\xCB""\x20""\xC8""\xBA""\xCC""\xBC""\xCA""\xB9""\xCE";
		return DrawFrame(rc, box);
	}

	bool DrawDouble( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		return DrawDouble(rc);
	}

	bool DrawBold( const Rectangle &rc ) const
	{
		const char *box = "\xDC\xDB\xDF\xFE";

		if (rc.width <= 0 || rc.height <= 0)
			return false;

		if (rc.height == 1) {
			for (int x = rc.left; x <= rc.right; x++)
				if (!Put(x, rc.y, box[3]))
					return false;
		}
		else {
			for (int x = rc.left; x <= rc.right; x++)
				if (!Put(x, rc.top,    box[0]) ||
				    !Put(x, rc.bottom, box[2]))
					return false;
			for (int y = rc.top + 1; y < rc.bottom; y++)
				if (!Put(rc.left,  y, box[1]) ||
				    !Put(rc.right, y, box[1]))
					return false;
		}

		return true;
	}

	bool DrawBold( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		return DrawBold(rc);
	}

	bool Fill( const int x, const int y, const int width, const int height, const Color fore, const Color back = Black ) const
	{
		if (width <= 0 || height <= 0)
			return false;

		COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		DWORD count;
		for (int h = height; h > 0; h--, coord.Y++)
			if (!FillConsoleOutputAttribute(Cout, MakeAttribute(fore, back), width, coord, &count))
				return false;

		return true;
	}

	bool Fill( const Rectangle &rc, const Color fore, const Color back = Black ) const
	{
		return Fill(rc.x, rc.y, rc.width, rc.height, fore, back);
	}

	bool Fill( const int x, const int y, const int width, const int height, const char c = ' ' ) const
	{
		if (width <= 0 || height <= 0)
			return false;

		COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		DWORD count;
		for (int h = height; h > 0; h--, coord.Y++)
			if (!FillConsoleOutputCharacter(Cout, c, width, coord, &count))
				return false;

		return true;
	}

	bool Fill( const Rectangle &rc, const char c = ' ' ) const
	{
		return Fill(rc.x, rc.y, rc.width, rc.height, c);
	}
};
