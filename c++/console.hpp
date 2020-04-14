/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    14.04.2020
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
	std::atomic_flag flag = ATOMIC_FLAG_INIT;

	static
	void __stdcall handler(PVOID f, BOOLEAN)
	{
		atomic_flag_clear(static_cast<std::atomic_flag *>(f));
	}

public:

	Timer( DWORD ms = 0 )
	{
		if (ms) {
			CreateTimerQueueTimer(&timer, NULL, handler, &flag, ms, ms, WT_EXECUTEDEFAULT);
		}
	}

	~Timer()
	{
		if (timer) {
			DeleteTimerQueueTimer(NULL, timer, NULL);
			CloseHandle(timer);
		}
	}

	bool wait()
	{
		return atomic_flag_test_and_set(&flag);
	}
};

class Console : public Timer
{
	CONSOLE_FONT_INFOEX cfi_;

	enum Bar : unsigned
	{
		NoBar    = 0,
		RightBar = 1,
		DownBar  = 2,
		LeftBar  = 4,
		UpBar    = 8,
		AllBars  = RightBar | DownBar | LeftBar | UpBar
	};

public:

	HANDLE Cin;
	HANDLE Cout;
	HANDLE Cerr;
	HWND   Hwnd;

	struct Rectangle
	{
		Rectangle() : Rectangle(0, 0, 0, 0) {}
		Rectangle(int _x, int _y, int _w, int _h) :
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

	enum Color : unsigned
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

	enum Grade : unsigned
	{
		NoGrade     = 0,
		SmallGrade  = 1,
		MediumGrade = 2,
		LargeGrade  = 3,
		FullGrade   = 4,
	};

	Console( const char *title = NULL, unsigned freq = 0 ) : Timer(freq)
	{
		Cin  = GetStdHandle(STD_INPUT_HANDLE);
		Cout = GetStdHandle(STD_OUTPUT_HANDLE);
		Cerr = GetStdHandle(STD_ERROR_HANDLE);
		Hwnd = GetConsoleWindow();

		if (!Cin || !Cout || !Cerr || !Hwnd)
			throw std::runtime_error("console error");

		cfi_.cbSize = sizeof(CONSOLE_FONT_INFOEX);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi_);

		DWORD mode;
		GetConsoleMode(Cin, &mode);
		SetConsoleMode(Cin, mode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

		if (title != NULL)
			SetTitle(title);

		Clear();
	}

	~Console()
	{
		FlushConsoleInputBuffer(Cin);
		SetCurrentConsoleFontEx(Cout, FALSE, &cfi_);
		Maximize();
		ShowCursor();
		FreeConsole();
	}

	void SetTitle( const char *title )
	{
		SetConsoleTitle(title);
	}

	void Minimize()
	{
		ShowWindow(Hwnd, SW_MINIMIZE);
	}

	void Maximize()
	{
		COORD size = GetLargestConsoleWindowSize(Cout);
		SetSize(size.X, size.Y);
		ShowWindow(Hwnd, SW_MAXIMIZE);
	}

	void Restore()
	{
		ShowWindow(Hwnd, SW_RESTORE);
	}

	bool IsMinimized()
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & WS_MINIMIZE) != 0;
	}

	bool IsMaximized()
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & WS_MAXIMIZE) != 0;
	}

	bool IsRestored()
	{
		return (GetWindowLong(Hwnd, GWL_STYLE) & (WS_MINIMIZE | WS_MAXIMIZE)) == 0;
	}

	void SetWindowed()
	{
		if (SetConsoleDisplayMode(Cout, CONSOLE_WINDOWED_MODE, NULL))
			Maximize();
	}

	void SetFullScreen()
	{
		if (SetConsoleDisplayMode(Cout, CONSOLE_FULLSCREEN_MODE, NULL))
			Maximize();
	}

	void Center()
	{
		RECT rc;
		GetWindowRect(Hwnd, &rc);
		int cx = rc.right - rc.left;
		int cy = rc.bottom - rc.top;
		int x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
		int y = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;
		SetWindowPos(Hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
//		MoveWindow(Hwnd, x, y, cx, cy, FALSE);
	}

	void Center( int width, int height )
	{
		SetSize(width, height);
		Center();
	}

	void GetSize( int *width, int *height )
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = sbi.dwSize.X;
		if (height != NULL) *height = sbi.dwSize.Y;
	}

	void GetWndSize( int *width, int *height )
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = sbi.srWindow.Right - sbi.srWindow.Left + 1;
		if (height != NULL) *height = sbi.srWindow.Bottom - sbi.srWindow.Top + 1;
	}

	void GetMaxSize( int *width, int *height )
	{
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		GetConsoleScreenBufferInfo(Cout, &sbi);
		if (width  != NULL) *width  = sbi.dwMaximumWindowSize.X;
		if (height != NULL) *height = sbi.dwMaximumWindowSize.Y;
	}

	void SetSize( int width, int height )
	{
		LockWindowUpdate(Hwnd);
		while (!IsRestored()) Restore();

		int w, h;
		GetSize(&w, &h);
		w = std::min(w, width);
		h = std::min(h, height);
		SMALL_RECT temp = { 0, 0, (SHORT)(w - 1), (SHORT)(h - 1) };
		SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
		COORD      size = { (SHORT)width, (SHORT)height };
		SetConsoleWindowInfo(Cout, TRUE, &temp);
		SetConsoleScreenBufferSize(Cout, size);
		SetConsoleWindowInfo(Cout, TRUE, &rect);

		Clear();
		LockWindowUpdate(NULL);
	}

	bool GetInput( INPUT_RECORD *rec = NULL, unsigned len = 1 )
	{
		DWORD count;
		GetNumberOfConsoleInputEvents(Cin, &count);
		if (count == 0) return false;
		if (rec == NULL) return true;
		return ReadConsoleInput(Cin, rec, len, &count) != 0;
	}

	void Clear()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);

		COORD home = { 0, 0 };
		DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
		DWORD count;
		FillConsoleOutputCharacter(Cout, ' ',              size, home, &count);
		FillConsoleOutputAttribute(Cout, csbi.wAttributes, size, home, &count);
		Home();
	}

	void Clear( Color fore, Color back = Black )
	{
		SetTextColor(fore, back);
		Clear();
	}

	void Home()
	{
		SetCursorPos(0, 0);
	}

	void HideCursor()
	{
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(Cout, &cci); cci.bVisible = FALSE;
		SetConsoleCursorInfo(Cout, &cci);
	}

	void ShowCursor()
	{
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(Cout, &cci); cci.bVisible = TRUE;
		SetConsoleCursorInfo(Cout, &cci);
	}

	void SetCursorSize( int size )
	{
		CONSOLE_CURSOR_INFO cci = { (DWORD)size, TRUE };
		if (size == 0) HideCursor();
		else           SetConsoleCursorInfo(Cout, &cci);
	}

	int GetFontSize()
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi);
		return (int)cfi.dwFontSize.Y;
	}

	void SetFontSize( int size )
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, FALSE, &cfi);
		cfi.dwFontSize.X = 0;
		cfi.dwFontSize.Y = (SHORT)size;
		SetCurrentConsoleFontEx(Cout, FALSE, &cfi);
	}

	void GetCursorPos( int *x, int *y )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (x != NULL) *x = csbi.dwCursorPosition.X;
		if (y != NULL) *y = csbi.dwCursorPosition.Y;
	}

	void SetCursorPos( int x, int y )
	{
	//	flushall();
		COORD coord = { (SHORT)x, (SHORT)y };
		SetConsoleCursorPosition(Cout, coord);
	}

	WORD MakeAttribute( Color fore, Color back )
	{
		return (WORD)fore | ((WORD)back * 16);
	}

	Color GetForeColor( WORD a )
	{
		return (Color)(a % 16);
	}

	Color GetBackColor( WORD a )
	{
		return (Color)((a / 16) % 16);
	}

	void GetTextColor( Color *fore, Color *back = NULL )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (fore != NULL) *fore = GetForeColor(csbi.wAttributes);
		if (back != NULL) *back = GetBackColor(csbi.wAttributes);
	}

	void SetTextColor( Color fore, Color back = Black )
	{
		SetConsoleTextAttribute(Cout, MakeAttribute(fore, back));
	}

	void SetText( int x, int y, Color fore, Color back = Black )
	{
		SetCursorPos(x, y);
		SetTextColor(fore, back);
	}

	char Get( int x, int y )
	{
		CHAR  c;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputCharacter(Cout, &c, 1, coord, &count);
		return c;
	}

	char Get( int x, int y, Color *fore, Color *back = NULL )
	{
		WORD  a;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputAttribute(Cout, &a, 1, coord, &count);
		if (fore != NULL) *fore = GetForeColor(a);
		if (back != NULL) *back = GetBackColor(a);
		return Get(x, y);
	}

	void Put( int x, int y, Color fore, Color back = Black )
	{
		WORD  t;
		WORD  a = MakeAttribute(fore, back);
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputAttribute(Cout, &t, 1, coord, &count);
		if (a != t)
			WriteConsoleOutputAttribute(Cout, &a, 1, coord, &count);
	}

	void Put( int x, int y, char c )
	{
		CHAR  t;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputCharacter(Cout, &t, 1, coord, &count);
		if (c != t)
			WriteConsoleOutputCharacter(Cout, &c, 1, coord, &count);
	}

	void Put( int x, int y, const char *s )
	{
		while (*s)
			Put(x++, y, *s++);
	}

	void Put( int x, int y, char c, Color fore, Color back = Black )
	{
		Put(x, y, fore, back);
		Put(x, y, c);
	}

	void Put( int x, int y, const char *s, Color fore, Color back = Black )
	{
		while (*s)
			Put(x++, y, *s++, fore, back);
	}

	char Get()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		return Get(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
	}

	char Get( Color *fore, Color *back = NULL )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		return Get(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, fore, back);
	}

	void Put( char c )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, c);
	}

	void Put( Color fore, Color back = Black )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, fore, back);
	}

	void ColorHLine( Rectangle &rc, int y, Color fore, Color back = Black )
	{
		if (rc.height <= 0) return;

		for (int x = rc.left; x <= rc.right; x++)
			Put(x, y, fore, back);
	}

	void ColorVLine( Rectangle &rc, int x, Color fore, Color back = Black )
	{
		if (rc.width <= 0) return;

		for (int y = rc.top; y <= rc.bottom; y++)
			Put(x, y, fore, back);
	}

	void ColorFrame( Rectangle &rc, Color fore, Color back = Black )
	{
		ColorHLine(rc, rc.top, fore, back);
		ColorVLine(rc, rc.left, fore, back);
		ColorVLine(rc, rc.right, fore, back);
		ColorHLine(rc, rc.bottom, fore, back);
	}

	//[[deprecated]]
	void ColorFrame( int x, int y, int width, int height, Color fore, Color back = Black )
	{
		Rectangle rc(x, y, width, height);
		ColorFrame(rc, fore, back);
	}

	void DrawHLine( Rectangle &rc, int y, const char *box, Bar b = NoBar )
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

	void DrawVLine( Rectangle &rc, int x, const char *box, Bar b = NoBar )
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

	void DrawFrame( Rectangle &rc, const char *box )
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

	//[[deprecated]]
	void DrawFrame( int x, int y, int width, int height, const char *box )
	{
		Rectangle rc(x, y, width, height);
		DrawFrame(rc, box);
	}

	void DrawSingle( Rectangle &rc )
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xDA""\x20""\xC4""\xBF""\xC2""\x20""\xC0""\xB3""\xC3""\xD9""\xC1""\xB4""\xC5";
		DrawFrame(rc, box);
	}

	//[[deprecated]]
	void DrawSingle( int x, int y, int width, int height )
	{
		Rectangle rc(x, y, width, height);
		DrawSingle(rc);
	}

	void DrawDouble( Rectangle &rc )
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xC9""\x20""\xCD""\xBB""\xCB""\x20""\xC8""\xBA""\xCC""\xBC""\xCA""\xB9""\xCE";
		DrawFrame(rc, box);
	}

	//[[deprecated]]
	void DrawDouble( int x, int y, int width, int height )
	{
		Rectangle rc(x, y, width, height);
		DrawDouble(rc);
	}

	void DrawBold( Rectangle &rc )
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

	void drawbold( Rectangle &rc )
	{
 		//                 ----  ---R  ---D  --DR  -L--  -L-R  -L-D  -LDR  U---  U--R  U-D-  U-DR  UL--  UL-R  ULD-  ULDR
		const char *box = "\x20""\x20""\x20""\xDC""\x20""\xFE""\xDC""\xDC""\x20""\xDF""\xDB""\xDB""\xDF""\xDF""\xDB""\xDB";
//		const char *box = "\x20""\x20""\x20""\xDC""\x20""\xDC""\xDC""\xDC""\x20""\xDC""\xDB""\xDB""\xDC""\xDC""\xDB""\xDB";
		DrawFrame(rc, box);
	}

	//[[deprecated]]
	void DrawBold( int x, int y, int width, int height )
	{
		Rectangle rc(x, y, width, height);
		DrawBold(rc);
	}

	void Fill( Rectangle &rc, Color fore, Color back = Black )
	{
		for (int y = rc.top; y <= rc.bottom; y++)
			for (int x = rc.left; x <= rc.right; x++)
				Put(x, y, fore, back);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, Color fore, Color back = Black )
	{
		Rectangle rc(x, y, width, height);
		Fill(rc, fore, back);
	}

	void Fill( Rectangle &rc, char c = ' ' )
	{
		for (int y = rc.top; y <= rc.bottom; y++)
			for (int x = rc.left; x <= rc.right; x++)
				Put(x, y, c);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, char c = ' ' )
	{
		Rectangle rc(x, y, width, height);
		Fill(rc, c);
	}

	void Fill( Rectangle &rc, Grade g )
	{
		const char *bkg = "\x20\xB0\xB1\xB2\xDB";
		Fill(rc, bkg[g]);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, Grade g )
	{
		Rectangle rc(x, y, width, height);
		Fill(rc, g);
	}
};
