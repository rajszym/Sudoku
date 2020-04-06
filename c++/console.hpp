/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    04.04.2020
   @brief   Sudoku game

*******************************************************************************

   Copyright (c) 2018 Rajmund Szymanski. All rights reserved.

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

	Console( const char *title = nullptr)
	{
		if (AllocConsole())
		{
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

		if (title != nullptr)
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

	void GetSize(int *width, int *height)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (width  != nullptr) *width  = csbi.dwSize.X;
		if (height != nullptr) *height = csbi.dwSize.Y;
	}

	void SetSize(int width, int height)
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

	void SetCursorSize(int size)
	{
		CONSOLE_CURSOR_INFO cci = { (DWORD)size, TRUE };
		if (size == 0) HideCursor();
		else           SetConsoleCursorInfo(Cout, &cci);
	}

	void GetCursorPos(int *x, int *y)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (x != nullptr) *x = csbi.dwCursorPosition.X;
		if (y != nullptr) *y = csbi.dwCursorPosition.Y;
	}

	void SetCursorPos(int x, int y)
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

	void SetFontSize(int size)
	{
		CONSOLE_FONT_INFOEX cfi = {};
		cfi.cbSize = sizeof(cfi);
		GetCurrentConsoleFontEx(Cout, false, &cfi);
		cfi.dwFontSize.Y = (SHORT)size;
		SetCurrentConsoleFontEx(Cout, false, &cfi);
	}

	WORD MakeAttribute(Color fore, Color back)
	{
		return (WORD)fore | ((WORD)back * 16);
	}

	Color GetForeColor(WORD a)
	{
		return (Color)(a % 16);
	}

	Color GetBackColor(WORD a)
	{
		return (Color)((a / 16) % 16);
	}

	void GetTextColor(Color *fore, Color *back = nullptr)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		if (fore != nullptr) *fore = GetForeColor(csbi.wAttributes);
		if (back != nullptr) *back = GetBackColor(csbi.wAttributes);
	}

	void SetTextColor(Color fore, Color back = Black)
	{
		SetConsoleTextAttribute(Cout, MakeAttribute(fore, back));
	}

	void SetText(int x, int y, Color fore, Color back = Black)
	{
		SetCursorPos(x, y);
		SetTextColor(fore, back);
	}

	void SetTitle(const char *title)
	{
		SetConsoleTitle((LPCTSTR)title);
	}

	bool GetInput(INPUT_RECORD *lpBuffer = nullptr, unsigned nLength = 1)
	{
		DWORD cNumberOfEvents;
		GetNumberOfConsoleInputEvents(Cin, &cNumberOfEvents);
		if (cNumberOfEvents == 0) return false;
		if (lpBuffer == nullptr) return true;
		return ReadConsoleInput(Cin, lpBuffer, nLength, &cNumberOfEvents) != 0;
	}

	void Clear()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);

		COORD home = { 0, 0 };
		DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
		DWORD written;
		FillConsoleOutputCharacter(Cout, ' ',              size, home, &written);
		FillConsoleOutputAttribute(Cout, csbi.wAttributes, size, home, &written);
		Home();
	}

	void Home()
	{
		COORD home = { 0, 0 };
		SetConsoleCursorPosition(Cout, home);
	}

	char Get(int x, int y)
	{
		CHAR  c;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputCharacter(Cout, &c, 1, coord, &count);
		return c;
	}

	char Get(int x, int y, Color *fore, Color *back = nullptr)
	{
		WORD  a;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputAttribute(Cout, &a, 1, coord, &count);
		if (fore != nullptr) *fore = GetForeColor(a);
		if (back != nullptr) *back = GetBackColor(a);
		return Get(x, y);
	}

	char Get()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		return Get(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
	}

	void Put(int x, int y, Color fore, Color back = Black)
	{
		WORD  t;
		WORD  a = MakeAttribute(fore, back);
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputAttribute(Cout, &t, 1, coord, &count);
		if (a != t)
			WriteConsoleOutputAttribute(Cout, &a, 1, coord, &count);
	}

	void Put(int x, int y, char c)
	{
		CHAR  t;
		DWORD count;
		COORD coord = { (SHORT)x, (SHORT)y };
		ReadConsoleOutputCharacter(Cout, &t, 1, coord, &count);
		if (c != t)
			WriteConsoleOutputCharacter(Cout, &c, 1, coord, &count);
	}

	void Put(int x, int y, char c, Color fore, Color back = Black)
	{
		Put(x, y, fore, back);
		Put(x, y, c);
	}

	void Put(Color fore, Color back = Black)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, fore, back);
	}

	void Put(char c)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(Cout, &csbi);
		Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, c);
	}

	void Put(int x, int y, const char *s)
	{
		while (*s)
			Put(x++, y, *s++);
	}

	void Put(int x, int y, const char *s, Color fore, Color back = Black)
	{
		while (*s)
			Put(x++, y, *s++, fore, back);
	}

	void DrawHorizontalLine(int x, int y, int length, const char *box, Bar b = NoBar)
	{
		if (length < 2)
			return;

		SetCursorPos(x, y);
		int p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
		Put(box[p | b | BarRight]);
		for (int i = x + 1; i < x + length - 1; i++)
		{
			SetCursorPos(i, y);
			p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
			Put(box[p | BarLeft | BarRight]);
		}
		SetCursorPos(x + length - 1, y);
		p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
		Put(box[p | b | BarLeft]);
	}

	void DrawVerticalLine(int x, int y, int length, const char *box, Bar b = NoBar)
	{
		if (length < 2)
			return;

		SetCursorPos(x, y);
		int p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
		Put(box[p | b | BarDown]);
		for (int i = y + 1; i < y + length - 1; i++)
		{
			SetCursorPos(x, i);
			p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
			Put(box[p | BarUp | BarDown]);
		}
		SetCursorPos(x, y + length - 1);
		p = strchr(box, Get()) - box; if (p < 0 || p > 15) p = 0;
		Put(box[p | b | BarUp]);
	}

	void DrawColor(int x, int y, int width, int height, Color fore, Color back = Black)
	{
		if (width < 1 || height < 1)
			return;

		for (int i = x; i < x + width; i++)
		{
			Put(i, y, fore, back);
		}
		for (int i = y + 1; i < y + height - 1; i++)
		{
			Put(x, i, fore, back);
			Put(x + width - 1, i, fore, back);
		}
		for (int i = x; i < x + width; i++)
		{
			Put(i, y + height - 1, fore, back);
		}
	}

	void Draw(int x, int y, int width, int height, const char *box)
	{
		if (width < 1 || height < 1)
			return;

		if (height == 1)
			DrawHorizontalLine(x, y, width, box);
		else
		if (width == 1)
			DrawVerticalLine(x, y, height, box);
		else
		{
			DrawHorizontalLine(x,             y,              width,  box, BarDown);
			DrawVerticalLine  (x,             y,              height, box, BarRight);
			DrawVerticalLine  (x + width - 1, y,              height, box, BarLeft);
			DrawHorizontalLine(x,             y + height - 1, width,  box, BarUp);
		}
	}

	void DrawSingle(int x, int y, int width, int height)
	{
		const char *box = "\x20\x20\x20\xDA\x20\xC4\xBF\xC2\x20\xC0\xB3\xC3\xD9\xC1\xB4\xC5";
		Draw(x, y, width, height, box);
	}

	void DrawDouble(int x, int y, int width, int height)
	{
		const char *box = "\x20\x20\x20\xC9\x20\xCD\xBB\xCB\x20\xC8\xBA\xCC\xBC\xCA\xB9\xCE";
		Draw(x, y, width, height, box);
	}

	void DrawBold(int x, int y, int width, int height)
	{
		const char *bkg = "\xDC\xFE\xDB\xDF";

		if (width < 1 || height < 1)
			return;

		if (height == 1)
			for (int i = x; i < x + width; i++)
				Put(i, y, bkg[1]);
		else
		{
			for (int i = x; i < x + width; i++)
				Put(i, y, bkg[0]);
			for (int r = y + 1; r < y + height - 1; r++)
			{
				Put(x, r, bkg[2]);
				Put(x + width - 1, r, bkg[2]);
			}
			for (int i = x; i < x + width; i++)
				Put(i, y + height - 1, bkg[3]);
		}
	}

	void FillColor(int x, int y, int width, int height, Color fore, Color back = Black)
	{
		for (int j = y; j < y + height; j++)
			for (int i = x; i < x + width; i++)
				Put(i, j, fore, back);
	}

	void Fill(int x, int y, int width, int height, char c = ' ')
	{
		for (int j = y; j < y + height; j++)
			for (int i = x; i < x + width; i++)
				Put(i, j, c);
	}

	void Fill(int x, int y, int width, int height, int fill)
	{
		if (fill < 0 || fill > 100)
			return;

		const char *bkg = "\x20\xB0\xB1\xB2\xDB";
		fill = (fill + 12) / 25;

		Fill(x, y, width, height, bkg[fill]);
	}
};

#endif // CONSOLE_HPP
