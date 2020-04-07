/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    07.04.2020
   @brief   Sudoku game

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

#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <windows.h>
#include <iostream>
#include <cstring>

struct Console
{
	enum Color
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
	};

	enum Bar
	{
		NoBar    = 0,
		BarRight = 1,
		BarDown  = 2,
		BarLeft  = 4,
		BarUp    = 8,
	};

	HANDLE Cin;
	HANDLE Cout;
	HANDLE Cerr;
	HWND   Hwnd;

	Console( const char *title = NULL )
	{
		if (AllocConsole()) {
			AttachConsole(ATTACH_PARENT_PROCESS);
			freopen("CONIN$",  "rb", stdin );
			freopen("CONOUT$", "wb", stdout);
			freopen("CONOUT$", "wb", stderr);
		}

		Cin  = GetStdHandle(STD_INPUT_HANDLE);
		Cout = GetStdHandle(STD_OUTPUT_HANDLE);
		Cerr = GetStdHandle(STD_ERROR_HANDLE);
		Hwnd = GetConsoleWindow();

		DWORD mode;
		GetConsoleMode(Cin, &mode);
		SetConsoleMode(Cin, mode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

		if (title != NULL)
			SetTitle(title);

		Clear();
	}

	~Console()
	{
		FreeConsole();
	}

	void SetWindowed()
	{
		COORD size;
		SetConsoleDisplayMode(Cout, CONSOLE_WINDOWED_MODE, &size);
	}

	void SetFullScreen()
	{
		COORD size;
		SetConsoleDisplayMode(Cout, CONSOLE_FULLSCREEN_MODE, &size);
	}

	void GetSize( int *width, int *height )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (width  != NULL) *width  = csbi.dwSize.X;
		if (height != NULL) *height = csbi.dwSize.Y;
	}

	void SetSize( int width, int height )
	{
		int w, h;
		GetSize(&w, &h);
		if (w > width ) w = width;
		if (h > height) h = height;
		SMALL_RECT rc = { 0, 0, (SHORT)(w - 1), (SHORT)(h - 1) };
		SetConsoleWindowInfo(Cout, TRUE, &rc);

		COORD size = { (SHORT)width, (SHORT)height };
		SetConsoleScreenBufferSize(Cout, size);

		SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
		SetConsoleWindowInfo(Cout, TRUE, &rect);
	}

	void Maximize()
	{
		ShowWindow(Hwnd, SW_SHOWMAXIMIZED);
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

	int GetFontSize()
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, false, &cfi);
		return (int)cfi.dwFontSize.Y;
	}

	void SetFontSize( int size )
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, false, &cfi);
		cfi.dwFontSize.Y = (SHORT)size;
		SetCurrentConsoleFontEx(Cout, false, &cfi);
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

	void SetTitle( const char *title )
	{
		SetConsoleTitle((LPCTSTR)title);
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

	void Home()
	{
		COORD home = { 0, 0 };
		SetConsoleCursorPosition(Cout, home);
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

	char Get()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		return Get(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
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

	void Put( int x, int y, char c, Color fore, Color back = Black )
	{
		Put(x, y, fore, back);
		Put(x, y, c);
	}

	void Put( Color fore, Color back = Black )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, fore, back);
	}

	void Put( char c )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, c);
	}

	void Put( int x, int y, const char *s )
	{
		while (*s)
			Put(x++, y, *s++);
	}

	void Put( int x, int y, const char *s, Color fore, Color back = Black )
	{
		while (*s)
			Put(x++, y, *s++, fore, back);
	}

	void ColorHLine( int xl, int xr, int y, Color fore, Color back = Black )
	{
		for (int i = xl; i <= xr; i++)
			Put(i, y, fore, back);
	}

	void ColorVLine( int x, int yt, int yb, Color fore, Color back = Black )
	{
		for (int i = yt; i <= yb; i++)
			Put(x, i, fore, back);
	}

	void ColorFrame( LPRECT rc, Color fore, Color back = Black )
	{
		if ((rc->left > rc->right) || (rc->top > rc->bottom))
			return;
		ColorHLine(rc->left, rc->right, rc->top, fore, back);
		if (rc->top != rc->bottom)
			ColorHLine(rc->left, rc->right, rc->bottom, fore, back);
		ColorVLine(rc->left, rc->top, rc->bottom, fore, back);
		if (rc->left != rc->right)
			ColorVLine(rc->right, rc->top, rc->bottom, fore, back);
	}

	//[[deprecated]]
	void ColorFrame( int x, int y, int width, int height, Color fore, Color back = Black )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		ColorFrame(&rc, fore, back);
	}

	void DrawHLine( int xl, int xr, int y, const char *box, Bar b = NoBar )
	{
		int p;
		if (xl >= xr) return;
		p = strchr(box, Get(xl, y)) - box; if (p < 0 || p > 15) p = 0;
		Put(xl, y, box[p | b | BarRight]);
		for (int i = xl + 1; i < xr; i++) {
			p = strchr(box, Get(i, y)) - box; if (p < 0 || p > 15) p = 0;
			Put(i, y, box[p | BarLeft | BarRight]);
		}
		p = strchr(box, Get(xr, y)) - box; if (p < 0 || p > 15) p = 0;
		Put(xr, y, box[p | b | BarLeft]);
	}

	void DrawVLine( int x, int yt, int yb, const char *box, Bar b = NoBar )
	{
		int p;
		if (yt >= yb) return;
		p = strchr(box, Get(x, yt)) - box; if (p < 0 || p > 15) p = 0;
		Put(x, yt, box[p | b | BarDown]);
		for (int i = yt + 1; i < yb; i++) {
			p = strchr(box, Get(x, i)) - box; if (p < 0 || p > 15) p = 0;
			Put(x, i, box[p | BarUp | BarDown]);
		}
		p = strchr(box, Get(x, yb)) - box; if (p < 0 || p > 15) p = 0;
		Put(x, yb, box[p | b | BarUp]);
	}

	void DrawFrame( LPRECT rc, const char *box )
	{
		if ((rc->left > rc->right) || (rc->top > rc->bottom))
			return;
		if (rc->top == rc->bottom)
			DrawHLine(rc->left, rc->right, rc->top, box);
		else
		if (rc->left == rc->right)
			DrawVLine(rc->left, rc->top, rc->bottom, box);
		else
		{
			DrawHLine(rc->left, rc->right, rc->top, box, BarDown);
			DrawVLine(rc->left, rc->top, rc->bottom, box, BarRight);
			DrawVLine(rc->right, rc->top, rc->bottom, box, BarLeft);
			DrawHLine(rc->left, rc->right, rc->bottom, box, BarUp);
		}
	}

	//[[deprecated]]
	void DrawFrame( int x, int y, int width, int height, const char *box )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		DrawFrame(&rc, box);
	}

	void DrawSingle( LPRECT rc )
	{
		const char *box = "\x20\x20\x20\xDA\x20\xC4\xBF\xC2\x20\xC0\xB3\xC3\xD9\xC1\xB4\xC5";
		DrawFrame(rc, box);
	}

	//[[deprecated]]
	void DrawSingle( int x, int y, int width, int height )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		DrawSingle(&rc);
	}

	void DrawDouble( LPRECT rc )
	{
		const char *box = "\x20\x20\x20\xC9\x20\xCD\xBB\xCB\x20\xC8\xBA\xCC\xBC\xCA\xB9\xCE";
		DrawFrame(rc, box);
	}

	//[[deprecated]]
	void DrawDouble( int x, int y, int width, int height )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		DrawDouble(&rc);
	}

	void DrawBold( LPRECT rc )
	{
		const char *bkg = "\xDC\xFE\xDB\xDF";

		if ((rc->left > rc->right) || (rc->top > rc->bottom))
			return;

		if (rc->top == rc->bottom) {
			for (int i = rc->left; i <= rc->right; i++)
				Put(i, rc->top, bkg[1]);
		}
		else {
			for (int i = rc->left; i <= rc->right; i++) {
				Put(i, rc->top, bkg[0]);
				Put(i, rc->bottom, bkg[3]);
			}
			for (int i = rc->top + 1; i < rc->bottom; i++) {
				Put(rc->left, i, bkg[2]);
				Put(rc->right, i, bkg[2]);
			}
		}
	}

	//[[deprecated]]
	void DrawBold( int x, int y, int width, int height )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		DrawBold(&rc);
	}

	void Fill( LPRECT rc, Color fore, Color back = Black )
	{
		for (int j = rc->top; j <= rc->bottom; j++)
			for (int i = rc->left; i <= rc->right; i++)
				Put(i, j, fore, back);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, Color fore, Color back = Black )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		Fill(&rc, fore, back);
	}

	void Fill( LPRECT rc, char c = ' ' )
	{
		for (int j = rc->top; j <= rc->bottom; j++)
			for (int i = rc->left; i <= rc->right; i++)
				Put(i, j, c);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, char c = ' ' )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		Fill(&rc, c);
	}

	void Fill( LPRECT rc, int fill )
	{
		const char *bkg = "\x20\xB0\xB1\xB2\xDB";

		if (fill < 0 || fill > 100)
			return;

		Fill(rc, bkg[(fill + 12) / 25]);
	}

	//[[deprecated]]
	void Fill( int x, int y, int width, int height, int fill )
	{
		RECT rc = { x, y, x + width - 1, y + height - 1 };
		Fill(&rc, fill);
	}
};

#endif // CONSOLE_HPP
