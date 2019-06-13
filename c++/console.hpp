/******************************************************************************

   @file    console.hpp
   @author  Rajmund Szymanski
   @date    13.06.2019
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

	Console(const char * = nullptr);
	~Console();

	void SetWindowed();
	void SetFullScreen();
	void GetSize(int*, int*);
	void SetSize(int , int );
	void Maximize();
	void HideCursor();
	void ShowCursor();
	void SetCursorSize(int);
	void GetCursorPos(int*, int*);
	void SetCursorPos(int , int );
	int  GetFontSize();
	void SetFontSize(int);
	void GetTextColor(Color *, Color * = nullptr);
	void SetTextColor(Color, Color = Black);
	void SetTitle(const char*);
	bool GetInput(INPUT_RECORD*);
	void Clear();
	void Home();
	char Get(int, int);
	char Get(int, int, Color *, Color * = nullptr);
	char Get();
	void Put(int, int, Color, Color = Black);
	void Put(int, int, char);
	void Put(int, int, char, Color, Color = Black);
	void Put(Color, Color = Black);
	void Put(char);
	void Put(int, int, const char *);
	void Put(int, int, const char *, Color, Color = Black);
	void DrawHorizontalLine(int, int, int, const char *, Bar b = NoBar);
	void DrawVerticalLine(int, int, int, const char *, Bar b = NoBar);
	void DrawColor(int, int, int, int, Color, Color = Black);
	void Draw(int, int, int, int, const char *);
	void DrawSingle(int, int, int, int);
	void DrawDouble(int, int, int, int);
	void DrawBold(int, int, int, int);
	void DrawBold(int, int, int, int, Color, Color = Black);
	void FillColor(int, int, int, int, Color, Color = Black);
	void Fill(int, int, int, int, char = ' ');
	void Fill(int, int, int, int, int);
};

Console::Console( const char *title)
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

Console::~Console()
{
	FreeConsole();
}

void Console::SetWindowed()
{
	COORD size;
	SetConsoleDisplayMode(Cout, CONSOLE_WINDOWED_MODE, &size);
}

void Console::SetFullScreen()
{
	COORD size;
	SetConsoleDisplayMode(Cout, CONSOLE_FULLSCREEN_MODE, &size);
}

void Console::GetSize(int *width, int *height)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	if (width)  *width  = csbi.dwSize.X;
	if (height) *height = csbi.dwSize.Y;
}

void Console::SetSize(int width, int height)
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

void Console::Maximize()
{
//	WINDOWINFO wi = { sizeof(WINDOWINFO), 0 }; GetWindowInfo(Hwnd, &wi);
	CONSOLE_FONT_INFO cfi; GetCurrentConsoleFont(Cout, TRUE, &cfi);
	COORD font = GetConsoleFontSize(Cout, cfi.nFont);
	COORD size = GetLargestConsoleWindowSize(Cout);

	int x = (GetSystemMetrics(SM_CXSCREEN) - size.X * font.X) / 2 - GetSystemMetrics(SM_CXFRAME) - 2; // (int)wi.cxWindowBorders - 2;
	int y = - GetSystemMetrics(SM_CYFRAME); // (int)wi.cyWindowBorders;

	SetSize(size.X, size.Y);
	SetWindowPos(Hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}

void Console::HideCursor()
{
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(Cout, &cci); cci.bVisible = FALSE;
	SetConsoleCursorInfo(Cout, &cci);
}

void Console::ShowCursor()
{
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(Cout, &cci); cci.bVisible = TRUE;
	SetConsoleCursorInfo(Cout, &cci);
}

void Console::SetCursorSize(int size)
{
	CONSOLE_CURSOR_INFO cci = { (DWORD)size, TRUE };
	if (!size) HideCursor();
	else       SetConsoleCursorInfo(Cout, &cci);
}

void Console::GetCursorPos(int *x, int *y)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	if (x) *x = csbi.dwCursorPosition.X;
	if (y) *y = csbi.dwCursorPosition.Y;
}

void Console::SetCursorPos(int x, int y)
{
//	flushall();
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(Cout, coord);
}

int Console::GetFontSize()
{
	CONSOLE_FONT_INFOEX cfi = { sizeof(cfi), 0, { 0, 0 }, 0, 0, { 0 } };
	GetCurrentConsoleFontEx(Cout, false, &cfi);
	return (int)cfi.dwFontSize.Y;
}

void Console::SetFontSize(int size)
{
	CONSOLE_FONT_INFOEX cfi = { sizeof(cfi), 0, { 0, 0 }, 0, 0, { 0 } };
	GetCurrentConsoleFontEx(Cout, false, &cfi);
	cfi.dwFontSize.Y = (SHORT)size;
	SetCurrentConsoleFontEx(Cout, false, &cfi);
}

void Console::GetTextColor(Color *fore, Color *back)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	if (fore) *fore = (Color)((csbi.wAttributes)      & 0x0F);
	if (back) *back = (Color)((csbi.wAttributes / 16) & 0x0F);
}

void Console::SetTextColor(Color fore, Color back)
{
	SetConsoleTextAttribute(Cout, (back << 4) | fore);
}

void Console::SetTitle(const char *title)
{
	SetConsoleTitle(title);
}

bool Console::GetInput(INPUT_RECORD *input)
{
	DWORD count;
	GetNumberOfConsoleInputEvents(Cin, &count);
	return count ? ReadConsoleInput(Cin, input, 1, &count) : false;
}

void Console::Clear()
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

void Console::Home()
{
	COORD home = { 0, 0 };
	SetConsoleCursorPosition(Cout, home);
}

char Console::Get(int x, int y)
{
	CHAR  c;
	DWORD count;
	COORD coord = { (SHORT)x, (SHORT)y };
	ReadConsoleOutputCharacter(Cout, &c, 1, coord, &count);
	return c;
}

char Console::Get(int x, int y, Color *fore, Color *back)
{
	WORD  a;
	DWORD count;
	COORD coord = { (SHORT)x, (SHORT)y };
	ReadConsoleOutputAttribute(Cout, &a, 1, coord, &count);
	if (fore) *fore = (Color)(a % 16);
	if (back) *back = (Color)(a / 16);
	return Get(x, y);
}

char Console::Get()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	return Get(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
}

void Console::Put(int x, int y, Color fore, Color back)
{
	WORD  t;
	WORD  a = fore | (back * 16);
	DWORD count;
	COORD coord = { (SHORT)x, (SHORT)y };
	ReadConsoleOutputAttribute(Cout, &t, 1, coord, &count);
	if (a != t)
		WriteConsoleOutputAttribute(Cout, &a, 1, coord, &count);
}

void Console::Put(int x, int y, char c)
{
	CHAR  t;
	DWORD count;
	COORD coord = { (SHORT)x, (SHORT)y };
	ReadConsoleOutputCharacter(Cout, &t, 1, coord, &count);
	if (c != t)
		WriteConsoleOutputCharacter(Cout, &c, 1, coord, &count);
}

void Console::Put(int x, int y, char c, Color fore, Color back)
{
	Put(x, y, fore, back);
	Put(x, y, c);
}

void Console::Put(Color fore, Color back)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, fore, back);
}

void Console::Put(char c)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Cout, &csbi);
	Put(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y, c);
}

void Console::Put(int x, int y, const char *s)
{
	while (*s)
		Put(x++, y, *s++);
}

void Console::Put(int x, int y, const char *s, Color fore, Color back)
{
	while (*s)
		Put(x++, y, *s++, fore, back);
}

void Console::DrawHorizontalLine(int x, int y, int length, const char *box, Bar b)
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

void Console::DrawVerticalLine(int x, int y, int length, const char *box, Bar b)
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

void Console::DrawColor(int x, int y, int width, int height, Color fore, Color back)
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

void Console::Draw(int x, int y, int width, int height, const char *box)
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

void Console::DrawSingle(int x, int y, int width, int height)
{
	const char *box = "\x20\x20\x20\xDA\x20\xC4\xBF\xC2\x20\xC0\xB3\xC3\xD9\xC1\xB4\xC5";
	Draw(x, y, width, height, box);
}

void Console::DrawDouble(int x, int y, int width, int height)
{
	const char *box = "\x20\x20\x20\xC9\x20\xCD\xBB\xCB\x20\xC8\xBA\xCC\xBC\xCA\xB9\xCE";
	Draw(x, y, width, height, box);
}

void Console::DrawBold(int x, int y, int width, int height)
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

void Console::FillColor(int x, int y, int width, int height, Color fore, Color back)
{
	for (int j = y; j < y + height; j++)
		for (int i = x; i < x + width; i++)
			Put(i, j, fore, back);
}

void Console::Fill(int x, int y, int width, int height, char c)
{
	for (int j = y; j < y + height; j++)
		for (int i = x; i < x + width; i++)
			Put(i, j, c);
}

void Console::Fill(int x, int y, int width, int height, int fill)
{
	if (fill < 0 || fill > 100)
		return;

	const char *bkg = "\x20\xB0\xB1\xB2\xDB";
	fill = (fill + 12) / 25;

	Fill(x, y, width, height, bkg[fill]);
}

#endif // CONSOLE_HPP
