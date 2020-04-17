/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    17.04.2020
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
#include <bits/stdc++.h>

class Timer
{
	HANDLE timer = NULL;
	mutable std::atomic_flag flag = ATOMIC_FLAG_INIT;

	static
	void __stdcall handler(PVOID f, BOOLEAN)
	{
		atomic_flag_clear(static_cast<std::atomic_flag *>(f));
	}

public:

	Timer( const DWORD freq = 0 )
	{
		if (freq > 0)
			CreateTimerQueueTimer(&timer, NULL, handler, &flag, freq, freq, WT_EXECUTEDEFAULT);
	}

	~Timer()
	{
		if (timer != NULL) {
			DeleteTimerQueueTimer(NULL, timer, NULL);
			CloseHandle(timer);
		}
	}

	bool wait() const
	{
		return atomic_flag_test_and_set(&flag);
	}
};

class Console: public Timer
{
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

public:

	const HWND   &Hwnd;
	const HANDLE &Cin;
	const HANDLE &Cout;
	const HANDLE &Cerr;

	struct Rectangle
	{
		Rectangle(): Rectangle(0, 0, 0, 0) {}
		Rectangle(int _x, int _y, int _w, int _h):
			x(_x), y(_y), width(_w), height(_h), left(_x), right(_x + _w - 1), top(_y), bottom(_y + _h - 1) {}

		const int x;
		const int y;
		const int width;
		const int height;
		const int left;
		const int right;
		const int top;
		const int bottom;
	};

	enum Color: WORD
	{
		Black       = 0,
		Blue        =                                                            FOREGROUND_BLUE,
		Green       =                                         FOREGROUND_GREEN,
		Cyan        =                                         FOREGROUND_GREEN | FOREGROUND_BLUE,
		Red         =                        FOREGROUND_RED,
		Purple      =                        FOREGROUND_RED |                    FOREGROUND_BLUE,
		Orange      =                        FOREGROUND_RED | FOREGROUND_GREEN,
		LightGrey   =                        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		Grey        = FOREGROUND_INTENSITY,
		LightBlue   = FOREGROUND_INTENSITY |                                     FOREGROUND_BLUE,
		LightGreen  = FOREGROUND_INTENSITY |                  FOREGROUND_GREEN,
		LightCyan   = FOREGROUND_INTENSITY |                  FOREGROUND_GREEN | FOREGROUND_BLUE,
		LightRed    = FOREGROUND_INTENSITY | FOREGROUND_RED,
		LightPurple = FOREGROUND_INTENSITY | FOREGROUND_RED |                    FOREGROUND_BLUE,
		Yellow      = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		White       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		Default     = LightGrey
	};

	enum Grade
	{
		NoGrade     = 0,
		SmallGrade  = 1,
		MediumGrade = 2,
		LargeGrade  = 3,
		FullGrade   = 4,
	};

	Console( LPCTSTR title = NULL, const DWORD freq = 0 ): Timer(freq), Hwnd(hwnd_), Cin(cin_), Cout(cout_), Cerr(cerr_)
	{
		hwnd_ = GetConsoleWindow();
		cin_  = GetStdHandle(STD_INPUT_HANDLE);
		cout_ = GetStdHandle(STD_OUTPUT_HANDLE);
		cerr_ = GetStdHandle(STD_ERROR_HANDLE);

		if (!hwnd_ || !cin_ || !cout_ || !cerr_)
			throw std::runtime_error("console error");

		cfi_.cbSize = sizeof(cfi_);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi_);

		DWORD mode;
		GetConsoleMode(Cin, &mode);
		mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		mode &= ~ENABLE_QUICK_EDIT_MODE;
		SetConsoleMode(Cin, mode);

		SetTitle(title);
		Clear();
	}

	~Console()
	{
		FlushConsoleInputBuffer(Cin);
		SetCurrentConsoleFontEx(Cout, FALSE, &cfi_);
		ShowCursor();
		Maximize();
	}

	void SetTitle( LPCTSTR title ) const
	{
		if (title != NULL)
			SetConsoleTitle(title);
	}

	void Minimize() const
	{
		ShowWindow(Hwnd, SW_MINIMIZE);
	}

	void Maximize() const
	{
		const COORD size = GetLargestConsoleWindowSize(Cout);
		SetSize(size.X, size.Y);
		ShowWindow(Hwnd, SW_MAXIMIZE);
	}

	void Restore() const
	{
		ShowWindow(Hwnd, SW_RESTORE);
	}

	bool Minimized() const
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & WS_MINIMIZE) != 0;
	}

	bool Maximized() const
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & WS_MAXIMIZE) != 0;
	}

	bool Windowed() const
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & (WS_MINIMIZE | WS_MAXIMIZE)) == 0;
	}

	void SetFullScreen( const bool fullscreen = true ) const
	{
		SetConsoleDisplayMode(Cout, fullscreen ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE, NULL);
		Maximize();
	}

	void GetSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = static_cast<int>(sbi.dwSize.X);
		if (height != NULL) *height = static_cast<int>(sbi.dwSize.Y);
	}

	void GetWndSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = static_cast<int>(sbi.srWindow.Right - sbi.srWindow.Left + 1);
		if (height != NULL) *height = static_cast<int>(sbi.srWindow.Bottom - sbi.srWindow.Top + 1);
	}

	void GetMaxSize( int * const width, int * const height ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = static_cast<int>(sbi.dwMaximumWindowSize.X);
		if (height != NULL) *height = static_cast<int>(sbi.dwMaximumWindowSize.Y);
	}

	void SetSize( const int width, const int height ) const
	{
		Home();
		while (!Windowed()) Restore();

		int w, h;
		GetSize(&w, &h);
		w = std::min(w, width);
		h = std::min(h, height);
		const SMALL_RECT temp = { 0, 0, static_cast<SHORT>(w - 1),     static_cast<SHORT>(h - 1) };
		const SMALL_RECT rect = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
		const COORD      size = {       static_cast<SHORT>(width),     static_cast<SHORT>(height) };
		SetConsoleWindowInfo(Cout, TRUE, &temp);
		SetConsoleScreenBufferSize(Cout, size);
		SetConsoleWindowInfo(Cout, TRUE, &rect);
	}

	void Center() const
	{
		while (!Windowed()) Restore();

		RECT rc;
		const int cx = GetSystemMetrics(SM_CXSCREEN);
		const int cy = GetSystemMetrics(SM_CYSCREEN);
		GetWindowRect(Hwnd, &rc);
		const int x = (cx - (rc.right - rc.left + 1)) / 2;
		const int y = (cy - (rc.bottom - rc.top + 1)) / 2;
		SetWindowPos(Hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	void Center( const int width, const int height ) const
	{
		SetSize(width, height);
		Center();
	}

	bool GetInput( INPUT_RECORD * const ir = NULL, const DWORD len = 1 ) const
	{
		DWORD count;
		GetNumberOfConsoleInputEvents(Cin, &count);
		if (count == 0) return false;
		if (ir == NULL) return true;
		return ReadConsoleInput(Cin, ir, len, &count) != 0;
	}

	void Clear() const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);

		const COORD home = { 0, 0 };
		const DWORD size = sbi.dwSize.X * sbi.dwSize.Y;
		DWORD count;
		FillConsoleOutputCharacter(Cout, ' ',             size, home, &count);
		FillConsoleOutputAttribute(Cout, sbi.wAttributes, size, home, &count);
		Home();
	}

	void Clear( const Color fore, const Color back = Black ) const
	{
		SetTextColor(fore, back);
		Clear();
	}

	void Home() const
	{
		SetCursorPos(0, 0);
	}

	void HideCursor() const
	{
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(Cout, &cci); cci.bVisible = FALSE;
		SetConsoleCursorInfo(Cout, &cci);
	}

	void ShowCursor() const
	{
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(Cout, &cci); cci.bVisible = TRUE;
		SetConsoleCursorInfo(Cout, &cci);
	}

	void SetCursorSize( const DWORD size ) const
	{
		const CONSOLE_CURSOR_INFO cci = { size, TRUE };
		if (size == 0) HideCursor();
		else           SetConsoleCursorInfo(Cout, &cci);
	}

	int GetFontSize() const
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi);
		return static_cast<int>(cfi.dwFontSize.Y);
	}

	void SetFont( const SHORT size, const WCHAR *name = NULL ) const
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi);
		cfi.dwFontSize = { 0, size };
		if (name != NULL) {
			cfi.FontWeight = FW_NORMAL;
			wcsncpy(cfi.FaceName, name, LF_FACESIZE);
		}
		SetCurrentConsoleFontEx(Cout, FALSE, &cfi);
	}

	void GetCursorPos( int * const x, int * const y ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (x != NULL) *x = sbi.dwCursorPosition.X;
		if (y != NULL) *y = sbi.dwCursorPosition.Y;
	}

	void SetCursorPos( const int x, const int y ) const
	{
	//	flushall();
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		SetConsoleCursorPosition(Cout, coord);
	}

	WORD MakeAttribute( const Color fore, const Color back ) const
	{
		return static_cast<WORD>(fore) | static_cast<WORD>(back * 16);
	}

	Color GetForeColor( const WORD a ) const
	{
		return static_cast<Color>(a % 16);
	}

	Color GetBackColor( const WORD a ) const
	{
		return static_cast<Color>((a / 16) % 16);
	}

	void GetTextColor( Color * const fore, Color * const back = NULL ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (fore != NULL) *fore = GetForeColor(sbi.wAttributes);
		if (back != NULL) *back = GetBackColor(sbi.wAttributes);
	}

	void SetTextColor( const Color fore, const Color back = Black ) const
	{
		SetConsoleTextAttribute(Cout, MakeAttribute(fore, back));
	}

	void SetText( const int x, const int y, const Color fore, const Color back = Black ) const
	{
		SetCursorPos(x, y);
		SetTextColor(fore, back);
	}

	char Get( const int x, const int y ) const
	{
		CHAR  c;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		ReadConsoleOutputCharacter(Cout, &c, 1, coord, &count);
		return c;
	}

	char Get( const int x, const int y, Color * const fore, Color * const back = NULL ) const
	{
		WORD  a;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		ReadConsoleOutputAttribute(Cout, &a, 1, coord, &count);
		if (fore != NULL) *fore = GetForeColor(a);
		if (back != NULL) *back = GetBackColor(a);
		return Get(x, y);
	}

	void Put( const int x, const int y, const Color fore, const Color back = Black ) const
	{
		WORD  t;
		const WORD  a = MakeAttribute(fore, back);
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		ReadConsoleOutputAttribute(Cout, &t, 1, coord, &count);
		if (a != t)
			WriteConsoleOutputAttribute(Cout, &a, 1, coord, &count);
	}

	void Put( const int x, const int y, const char c ) const
	{
		CHAR  t;
		DWORD count;
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		ReadConsoleOutputCharacter(Cout, &t, 1, coord, &count);
		if (c != t)
			WriteConsoleOutputCharacter(Cout, &c, 1, coord, &count);
	}

	void Put( const int x, const int y, const char * const s ) const
	{
		int i = x;
		const char *p = s;
		while (*p) Put(i++, y, *p++);
	}

	void Put( const int x, const int y, const char c, const Color fore, const Color back = Black ) const
	{
		Put(x, y, fore, back);
		Put(x, y, c);
	}

	void Put( const int x, const int y, const char * const s, const Color fore, const Color back = Black ) const
	{
		int i = x;
		const char *p = s;
		while (*p) Put(i++, y, *p++, fore, back);
	}

	char Get() const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		return Get(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y);
	}

	char Get( Color * const fore, Color * const back = NULL ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		return Get(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, fore, back);
	}

	void Put( const char c ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		Put(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, c);
	}

	void Put( const Color fore, const Color back = Black ) const
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		Put(sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y, fore, back);
	}

	void ColorHLine( const Rectangle &rc, const int y, const Color fore, const Color back = Black ) const
	{
		if (rc.height > 0)
			for (int x = rc.left; x <= rc.right; x++)
				Put(x, y, fore, back);
	}

	void ColorVLine( const Rectangle &rc, const int x, const Color fore, const Color back = Black ) const
	{
		if (rc.width > 0)
			for (int y = rc.top; y <= rc.bottom; y++)
				Put(x, y, fore, back);
	}

	void ColorFrame( const Rectangle &rc, const Color fore, const Color back = Black ) const
	{
		ColorHLine(rc, rc.top, fore, back);
		ColorVLine(rc, rc.left, fore, back);
		ColorVLine(rc, rc.right, fore, back);
		ColorHLine(rc, rc.bottom, fore, back);
	}

	void ColorFrame( const int x, const int y, const int width, const int height, const Color fore, const Color back = Black ) const
	{
		const Rectangle rc(x, y, width, height);
		ColorFrame(rc, fore, back);
	}

	void DrawHLine( const Rectangle &rc, const int y, const char * const box, const Bar b = NoBar ) const
	{
		if (rc.width <= 1) return;

		int x = rc.left;
		Bar p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		Put(x++, y, box[p | b | RightBar]);
		while (x < rc.right) {
			p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
			Put(x++, y, box[p | LeftBar | RightBar]);
		}
		p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		Put(x++, y, box[p | b | LeftBar]);
	}

	void DrawVLine( const Rectangle &rc, const int x, const char * const box, const Bar b = NoBar ) const
	{
		if (rc.height <= 1) return;

		int y = rc.top;
		Bar p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		Put(x, y++, box[p | b | DownBar]);
		while (y < rc.bottom) {
			p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
			Put(x, y++, box[p | UpBar | DownBar]);
		}
		p = Bar(strchr(box, Get(x, y)) - box); if (p > AllBars) p = NoBar;
		Put(x, y++, box[p | b | UpBar]);
	}

	void DrawFrame( const Rectangle &rc, const char * const box ) const
	{
		if (rc.width > 1) {
			if (rc.height > 1) {
				DrawHLine(rc, rc.top, box, DownBar);
				DrawVLine(rc, rc.left, box, RightBar);
				DrawVLine(rc, rc.right, box, LeftBar);
				DrawHLine(rc, rc.bottom, box, UpBar);
			}
			else
			if (rc.height == 1)
				DrawHLine(rc, rc.y, box);
		}
		else
		if (rc.height > 1) {
			if (rc.width == 1)
				DrawVLine(rc, rc.x, box);
		}
	}

	void DrawFrame( const int x, const int y, const int width, const int height, const char * const box ) const
	{
		const Rectangle rc(x, y, width, height);
		DrawFrame(rc, box);
	}

	void DrawSingle( const Rectangle &rc ) const
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xDA""\x20""\xC4""\xBF""\xC2""\x20""\xC0""\xB3""\xC3""\xD9""\xC1""\xB4""\xC5";
		DrawFrame(rc, box);
	}

	void DrawSingle( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		DrawSingle(rc);
	}

	void DrawDouble( const Rectangle &rc ) const
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xC9""\x20""\xCD""\xBB""\xCB""\x20""\xC8""\xBA""\xCC""\xBC""\xCA""\xB9""\xCE";
		DrawFrame(rc, box);
	}

	void DrawDouble( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		DrawDouble(rc);
	}

	void DrawBold( const Rectangle &rc ) const
	{
		const char *box = "\xDC\xDB\xDF\xFE";

		if ((rc.width <= 0) || (rc.height <= 0)) return;

		if (rc.height == 1) {
			for (int x = rc.left; x <= rc.right; x++)
				Put(x, rc.y, box[3]);
		}
		else {
			for (int x = rc.left; x <= rc.right; x++) {
				Put(x, rc.top,    box[0]);
				Put(x, rc.bottom, box[2]);
			}
			for (int y = rc.top + 1; y < rc.bottom; y++) {
				Put(rc.left,  y, box[1]);
				Put(rc.right, y, box[1]);
			}
		}
	}

	void drawbold( const Rectangle &rc ) const
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xDC""\x20""\xFE""\xDC""\xDC""\x20""\xDF""\xDB""\xDB""\xDF""\xDF""\xDB""\xDB";
//		const char *box = "\x20""\x20""\x20""\xDC""\x20""\xDC""\xDC""\xDC""\x20""\xDC""\xDB""\xDB""\xDC""\xDC""\xDB""\xDB";
		DrawFrame(rc, box);
	}

	void DrawBold( const int x, const int y, const int width, const int height ) const
	{
		const Rectangle rc(x, y, width, height);
		DrawBold(rc);
	}

	void Fill( const int x, const int y, const int width, const int height, const Color fore, const Color back = Black ) const
	{
		COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		DWORD count;
		if (width > 0)
			for (int h = height; h > 0; h--, coord.Y++)
				FillConsoleOutputAttribute(Cout, MakeAttribute(fore, back), width, coord, &count);
	}

	void Fill( const Rectangle &rc, const Color fore, const Color back = Black ) const
	{
		Fill(rc.x, rc.y, rc.width, rc.height, fore, back);
	}

	void Fill( const int x, const int y, const int width, const int height, const char c = ' ' ) const
	{
		COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		DWORD count;
		if (width > 0)
			for (int h = height; h > 0; h--, coord.Y++)
				FillConsoleOutputCharacter(Cout, c, width, coord, &count);
	}

	void Fill( const Rectangle &rc, const char c = ' ' ) const
	{
		Fill(rc.x, rc.y, rc.width, rc.height, c);
	}

	void Fill( const int x, const int y, const int width, const int height, const Grade g )
	{
		const char *bkg = "\x20\xB0\xB1\xB2\xDB";
		Fill(x, y, width, height, bkg[g]);
	}

	void Fill( const Rectangle &rc, const Grade g ) const
	{
		Fill(rc.x, rc.y, rc.width, rc.height, g);
	}
};
