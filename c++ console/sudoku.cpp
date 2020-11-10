/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    10.11.2020
   @brief   Sudoku game, solver and generator

*******************************************************************************

   Copyright (c) 2018 - 2020 Rajmund Szymanski. All rights reserved.

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

#include <windows.h>
#include "sudoku.hpp"
#include "console.hpp"
#include "gametimer.hpp"
#include <iostream>
#include <iomanip>
#include <codecvt>
#include <locale>
#include <tchar.h>

using Cell = SudokuCell;

const TCHAR *title = _T("Sudoku");

const Console::Rect TAB(0, 1, 25, 13);
const Console::Rect BTN(TAB.right, TAB.top,  3, TAB.height);
const Console::Rect MNU(BTN.right, BTN.top, 14, BTN.height);
const Console::Rect HDR(TAB.left, 0, MNU.right - TAB.left, TAB.top);
const Console::Rect FTR(HDR.left, TAB.bottom, HDR.width, HDR.height);
const Console::Rect WIN(0, 0, HDR.left + HDR.right, FTR.bottom);

/*---------------------------------------------------------------------------*/
/*                                GAME CLASSES                               */
/*---------------------------------------------------------------------------*/

enum Assistance
{
	None = 0,
	Current,
	Available,
	Sure,
	Full,
};

/*---------------------------------------------------------------------------*/

enum Command: int
{
	NoCmd = -1,
	Button0Cmd = 0,
	Button1Cmd = 1,
	Button2Cmd = 2,
	Button3Cmd = 3,
	Button4Cmd = 4,
	Button5Cmd = 5,
	Button6Cmd = 6,
	Button7Cmd = 7,
	Button8Cmd = 8,
	Button9Cmd = 9,
	ClearCellCmd,
	SetCellCmd,
	PrevHelpCmd,
	NextHelpCmd,
	PrevLevelCmd,
	NextLevelCmd,
	GenerateCmd,
	HighLightCmd,
	TimerCmd,
	SolveCmd,
	UndoCmd,
	ClearCmd,
	EditCmd,
	AcceptCmd,
	SaveCmd,
	LoadCmd,
	QuitCmd,
};

/*---------------------------------------------------------------------------*/

class GameHeader
{
public:

	GameHeader() {}

	void    update      ( Console &, const bool, const TCHAR *, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameCell
{
	const int x, y;
	Cell* const cell;

	bool focused;

public:

	GameCell( const int _x, const int _y, Cell* const _c ): x{_x}, y{_y}, cell{_c}, focused{false} {}

	void    update      ( Console &, const bool, Cell* const , const int, const bool, const Assistance );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	Cell  * getCell     ();
};

/*---------------------------------------------------------------------------*/

class GameTable: public std::vector<GameCell>
{
public:

	GameTable( Sudoku & );

	void    update      ( Console &, const bool, Cell* const , const int, const bool, const Assistance );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	Command mouseRButton( const int, const int );
	Cell  * getCell     ();
};

/*---------------------------------------------------------------------------*/

class Button
{
	const int x, y;
	const int num;

	bool focused;

public:

	Button( const int _y, const int _n ): x(BTN.x + 1), y{_y}, num{_n} {}

	void    update      ( Console &, const bool, Cell* const , const int, const int, const Assistance );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};


class GameButtons: public std::vector<Button>
{
public:

	GameButtons();

	void    update      ( Console &, const bool, Cell* const , const int, const int, const Assistance );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	Command mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class MenuItem: public std::vector<const TCHAR *>
{
	const int x, y;
	const TCHAR * const key;
	const TCHAR * const info;
	const int num;

	int  idx;
	bool focused;

public:

	MenuItem( const int _n, const int _y, const TCHAR *_k, const TCHAR *_i ): x{MNU.x + 1}, y{_y}, key{_k}, info{_i}, num{_n}, idx{0}, focused{false} {}

	void    update      ( Console &, const bool, const int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	const TCHAR *getInfo();
	template<typename T>
	void    setIndex    ( const T );
	int     prev        ();
	int     next        ();
};

/*---------------------------------------------------------------------------*/

class GameMenu: public std::vector<MenuItem>
{
public:

	GameMenu();

	void    update      ( Console &, const bool, const int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	const TCHAR *getInfo();
};

/*---------------------------------------------------------------------------*/

class GameFooter
{
public:

	GameFooter() {}

	void update( Console &, const bool, const TCHAR * );
};

/*---------------------------------------------------------------------------*/

class Game: public Console, public Sudoku, public GameTimer<int>
{
	GameHeader  hdr;
	GameTable   tab;
	GameButtons btn;
	GameMenu    mnu;
	GameFooter  ftr;
	
	int  number;
	bool timer_f;
	bool light_f;

	Assistance help;

	bool alive;
	int  xpos;

	void run();

public:

	Game();
	~Game();

	void operator()() { Game::run(); }

	void update      ();
	void mouseMove   ( const int, const int );
	void mouseLButton( const int, const int );
	void mouseRButton( const int, const int );
	void mouseWheel  ( const int );
	void keyboard    ( const int );
	void command     ( Command );
};

/*---------------------------------------------------------------------------*/
/*                              IMPLEMENTATION                               */
/*---------------------------------------------------------------------------*/

void GameHeader::update( Console &con, const bool init, const TCHAR *info, const int time )
{
	if (init)
	{
		con.Fill(HDR, Console::White);
		con.Put(HDR.x + 1, HDR.y, ::title);
	}

	auto s = _tcslen(info);
	con.Fill(TAB.x + 9, HDR.y, TAB.width - 10 - s, 1);
	con.Put(TAB.Right(s + 1), HDR.y, info);


	if (time >= 0)
	{
		TCHAR v[16];
		_sntprintf(v, sizeof(v), _T("%6d:%02d:%02d"), time / 3600, (time / 60) % 60, time % 60);
		con.Put(MNU.Right(_tcslen(v) + 1), HDR.y, v);
	}
	else
	{
		con.Fill(HDR.right - 12, HDR.y, 12, 1);
	}
}

Command GameHeader::mouseLButton( const int _x, const int _y )
{
	if (HDR.contains(_x, _y))
		return TimerCmd;

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

void GameCell::update( Console &con, const bool, Cell* const focus, const int number, const bool light, const Assistance help )
{
	auto l = GameCell::focused || (light && focus != nullptr && GameCell::cell->in_lst(*focus));
	auto f = GameCell::cell->empty() ? Console::LightGrey : GameCell::cell->immutable ? Console::White : Console::LightGreen;
	auto b = l ? Console::Grey : Console::Black;

	if (number != 0)
	{
		if      (help >= Assistance::Current && GameCell::cell->equal(number))   b = l ? Console::LightRed   : Console::Red;
		else if (help >= Assistance::Full    && GameCell::cell->sure(number))    b = l ? Console::LightGreen : Console::Green;
		else if (help >= Assistance::Full    && GameCell::cell->allowed(number)) b = l ? Console::Yellow     : Console::Orange;
	}

	con.Put(GameCell::x, GameCell::y, f, b);
	con.Put(GameCell::x, GameCell::y, _T("-123456789")[cell->num]);
}

void GameCell::mouseMove( const int _x, const int _y )
{
	GameCell::focused = _x == GameCell::x && _y == GameCell::y;
}

Command GameCell::mouseLButton( const int _x, const int _y )
{
	if (_x == GameCell::x && _y == GameCell::y)
	{
		if (GameCell::cell->num == 0)
			return SetCellCmd;
		else
		if (GameCell::cell->immutable)
			return static_cast<Command>(GameCell::cell->num);
		else
			return ClearCellCmd;
	}

	return NoCmd;
}

Cell *GameCell::getCell()
{
	if (GameCell::focused)
		return GameCell::cell;

	return nullptr;
}

/*---------------------------------------------------------------------------*/

GameTable::GameTable( Sudoku &_s )
{
	for (auto &c: _s)
	{
		int x = TAB.x + 2 + (c.pos % 9 + c.pos % 9 / 3) * 2;
		int y = TAB.y + 1 + (c.pos / 9 + c.pos / 9 / 3);

		GameTable::emplace_back(x, y, &c);
	}
}

void GameTable::update( Console &con, const bool init, Cell* const focus, const int number, const bool light, const Assistance help )
{
	if (init)
	{
		con.DrawSingle(TAB.x, TAB.y + (TAB.height - 1) / 3, TAB.width, (TAB.height - 1) / 3 + 1);
		con.DrawSingle(TAB.x + (TAB.width - 1) / 3, TAB.y, (TAB.width - 1) / 3 + 1, TAB.height);
		con.DrawSingle(TAB);
	}

	for (auto &c: *this)
		c.update(con, init, focus, number, light, help);
}

void GameTable::mouseMove( const int _x, const int _y )
{
	for (auto &c: *this)
		c.mouseMove(_x, _y);
}

Command GameTable::mouseLButton( const int _x, const int _y )
{
	for (auto &c: *this)
	{
		Command d = c.mouseLButton(_x, _y);
		if (d != NoCmd) return d;
	}

	return NoCmd;
}

Command GameTable::mouseRButton( const int _x, const int _y )
{
	if (TAB.contains(_x, _y))
		return Button0Cmd;

	return NoCmd;
}

Cell *GameTable::getCell()
{
	Cell *cell = nullptr;
	for (auto &c: *this)
		if (cell = c.getCell(), cell != nullptr)
			break;
	return cell;
}

/*---------------------------------------------------------------------------*/

void Button::update( Console &con, const bool init, Cell* const focus, const int number, const int count, const Assistance help )
{
	if (init)
		con.Put(BTN.x + 1, Button::y, _T("0123456789")[num]);

	auto f = number == Button::num ? Console::Black : Button::focused ? Console::White : Console::LightGrey;
	auto b = number == Button::num ? Console::White : Button::focused ? Console::Grey  : Console::Black;

	if (focus != nullptr)
	{
		if      (help >= Assistance::Sure      && focus->sure(Button::num))    (number == Button::num ? b : f) = Console::LightGreen;
		else if (help >= Assistance::Available && focus->allowed(Button::num)) (number == Button::num ? b : f) = Console::Yellow;
	}

	con.Put(Button::x, Button::y, f, b);

	if (number == Button::num && help > Assistance::None)
		con.Put(Button::x, HDR.y, count > 9 ? '?' : _T("0123456789")[count]);
}

void Button::mouseMove( const int _x, const int _y )
{
	Button::focused = _x >= BTN.left && _x < BTN.right && _y == Button::y;
}

Command Button::mouseLButton( const int _x, const int _y )
{
	if (_x >= BTN.left && _x < BTN.right && _y == Button::y)
		return static_cast<Command>(Button::num);

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameButtons::GameButtons()
{
	for (int n = 1; n <= 9; n++)
	{
		int y = BTN.y + n + (n - 1) / 3;

		GameButtons::emplace_back(y, n);
	}
}

void GameButtons::update( Console &con, const bool init, Cell* const focus, const int number, const int count, const Assistance help )
{
	if (init)
		con.DrawSingle(BTN);

	for (auto &b: *this)
		b.update(con, init, focus, number, count, help);

	if (number == 0 || help == Assistance::None)
		con.Put(BTN.x + 1, HDR.y, ' ');
}

void GameButtons::mouseMove( const int _x, const int _y )
{
	for (auto &b: *this)
		b.mouseMove(_x, _y);
}

Command GameButtons::mouseLButton( const int _x, const int _y )
{
	for (auto &b: *this)
	{
		Command d = b.mouseLButton(_x, _y);
		if (d != NoCmd) return d;
	}

	if (BTN.contains(_x, _y))
		return Button0Cmd;

	return NoCmd;
}

Command GameButtons::mouseRButton( const int _x, const int _y )
{
	if (BTN.contains(_x, _y))
		return Button0Cmd;

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

void MenuItem::update( Console &con, const bool init, const int _x )
{
	if (init)
	{
		con.Put(MNU.x + 1, MenuItem::y, key);
		con.Put(MNU.x + 4, MenuItem::y, MenuItem::at(0));
	}

	if (MenuItem::size() > 1)
	{
		con.Put(MenuItem::x, MenuItem::y, !MenuItem::focused ? MenuItem::key : _x < MNU.center ? _T("<<") : _T(">>"));
		auto s = _tcslen(MenuItem::at(MenuItem::idx));
		con.Put(MenuItem::x + 3, MenuItem::y, MenuItem::at(MenuItem::idx));
		con.Fill(MenuItem::x + 3 + s, MenuItem::y, MNU.width - 5 - s, 1);
	}

	if (MenuItem::focused)
		con.Fill(MenuItem::x, MenuItem::y, MNU.width - 2, 1, Console::White, Console::Grey);
	else
		con.Fill(MenuItem::x, MenuItem::y, MNU.width - 2, 1, Console::LightGrey);
}

void MenuItem::mouseMove( const int _x, const int _y )
{
	MenuItem::focused = _x > MNU.left && _x < MNU.right && _y == MenuItem::y;
}

Command MenuItem::mouseLButton( const int _x, const int _y )
{
	if (_x > MNU.left && _x < MNU.right && _y == MenuItem::y)
	{
		switch (MenuItem::num)
		{
		case  0: return _x < MNU.center ? PrevLevelCmd : NextLevelCmd;
		case  1: return _x < MNU.center ? PrevHelpCmd  : NextHelpCmd;
		case  2: return GenerateCmd;
		case  3: return SolveCmd;
		case  4: return UndoCmd;
		case  5: return ClearCmd;
		case  6: return EditCmd;
		case  7: return AcceptCmd;
		case  8: return SaveCmd;
		case  9: return LoadCmd;
		case 10: return QuitCmd;
		}
	}

	return NoCmd;
}

const TCHAR *MenuItem::getInfo()
{
	if (MenuItem::focused)
		return MenuItem::info;

	return nullptr;
}

template<typename T>
void MenuItem::setIndex( const T _i )
{
	MenuItem::idx = static_cast<int>(_i);
}

int MenuItem::prev()
{
	const int i = MenuItem::idx;
	const int s = MenuItem::size();

	if (MenuItem::num == 0)
		MenuItem::idx = i == 0 ? s - 1 : i == s - 1 ? 1 : 0;
	else
		MenuItem::idx = (i + s - 1) % s;

	return MenuItem::idx;
}

int MenuItem::next()
{
	const int i = MenuItem::idx;
	const int s = MenuItem::size();

	if (MenuItem::num == 0)
		MenuItem::idx = i == s - 1 ? 0 : i == 0 ? 1 : s - 1;
	else
		MenuItem::idx = (i + 1) % s;

	return MenuItem::idx;
}

/*---------------------------------------------------------------------------*/

GameMenu::GameMenu()
{
	GameMenu::emplace_back( 0, MNU.y +  1, _T("d:"), _T("Change difficulty level of the game"));
		GameMenu::back().emplace_back(_T("easy"));
		GameMenu::back().emplace_back(_T("medium"));
		GameMenu::back().emplace_back(_T("hard"));
		GameMenu::back().emplace_back(_T("expert"));
		GameMenu::back().emplace_back(_T("extreme"));
	GameMenu::emplace_back( 1, MNU.y +  2, _T("a:"), _T("Change assistance level of the game"));
		GameMenu::back().emplace_back(_T("none"));
		GameMenu::back().emplace_back(_T("current"));
		GameMenu::back().emplace_back(_T("available"));
		GameMenu::back().emplace_back(_T("sure"));
		GameMenu::back().emplace_back(_T("full"));
	GameMenu::emplace_back( 2, MNU.y +  3, _T("n:"), _T("Generate or load a new layout"));
		GameMenu::back().emplace_back(_T("new"));
	GameMenu::emplace_back( 3, MNU.y +  4, _T("s:"), _T("Solve the current layout"));
		GameMenu::back().emplace_back(_T("solve"));
	GameMenu::emplace_back( 4, MNU.y +  5, _T("u:"), _T("Undo last move / restore accepted layout"));
		GameMenu::back().emplace_back(_T("undo"));
	GameMenu::emplace_back( 5, MNU.y +  6, _T("c:"), _T("Clear the board"));
		GameMenu::back().emplace_back(_T("clear"));
	GameMenu::emplace_back( 6, MNU.y +  7, _T("e:"), _T("Start editing the current layout"));
		GameMenu::back().emplace_back(_T("edit"));
	GameMenu::emplace_back( 7, MNU.y +  8, _T("t:"), _T("Accept the layout and finish editing"));
		GameMenu::back().emplace_back(_T("accept"));
	GameMenu::emplace_back( 8, MNU.y +  9, _T("v:"), _T("Save the current layout to the file"));
		GameMenu::back().emplace_back(_T("save"));
	GameMenu::emplace_back( 9, MNU.y + 10, _T("l:"), _T("Load layout from the file"));
		GameMenu::back().emplace_back(_T("load"));
	GameMenu::emplace_back(10, MNU.y + 11, _T("q:"), _T("Quit the game"));
		GameMenu::back().emplace_back(_T("quit"));
}

void GameMenu::update( Console &con, const bool init, const int _x )
{
	if (init)
		con.DrawSingle(MNU);

	for (auto &m: *this)
		m.update(con, init, _x);
}

void GameMenu::mouseMove( const int _x, const int _y )
{
	for (auto &m: *this)
		m.mouseMove(_x, _y);
}

Command GameMenu::mouseLButton( const int _x, const int _y )
{
	for (auto &m: *this)
	{
		Command d = m.mouseLButton(_x, _y);
		if (d != NoCmd) return d;
	}

	return NoCmd;
}

const TCHAR *GameMenu::getInfo()
{
	const TCHAR *info = nullptr;
	for (auto &m: *this)
		if (info = m.getInfo(), info != nullptr)
			break;
	return info;
}

/*---------------------------------------------------------------------------*/

void GameFooter::update( Console &con, const bool init, const TCHAR *info )
{
	if (init)
		con.Fill(FTR, Console::Black, Console::Grey);

	if (info == nullptr)
		info = _T("Sudoku game, solver and generator");
	auto s = _tcslen(info);
	con.Put(FTR.x + 1, FTR.y, info);
	con.Fill(FTR.x + 1 + s, FTR.y, FTR.width - 2 - s, 1);
}

/*---------------------------------------------------------------------------*/

Game::Game(): Console(::title), hdr{}, tab{*this}, btn{}, mnu{}, ftr{}, timer_f{true}, light_f{false}, help{Assistance::None}, alive{true}, xpos{0}
{
	Console::SetFont(56, L"Consolas");
	Console::Center(WIN.width, WIN.height);
	Console::HideCursor();
	Console::Clear();

	Sudoku::generate();

	Game::mnu[0].setIndex(Sudoku::level);
	Game::mnu[1].setIndex(Game::help);
}

Game::~Game()
{
	Console::Clear();
}

void Game::run()
{
	while (Game::alive)
	{
		int x, y, s, k;
		INPUT_RECORD input;

		if (Console::GetInput(&input))
		{
			switch (input.EventType)
			{
			case MOUSE_EVENT:

				x = static_cast<int>(input.Event.MouseEvent.dwMousePosition.X);
				y = static_cast<int>(input.Event.MouseEvent.dwMousePosition.Y);

				switch(input.Event.MouseEvent.dwEventFlags)
				{
				case MOUSE_MOVED:

					Game::mouseMove(x, y);
					break;

				case 0: /* falls through */
				case DOUBLE_CLICK:

					switch (input.Event.MouseEvent.dwButtonState)
					{
					case FROM_LEFT_1ST_BUTTON_PRESSED:
						Game::mouseLButton(x, y);
						break;

					case RIGHTMOST_BUTTON_PRESSED:
						Game::mouseRButton(x, y);
						break;
					}

					break;

				case MOUSE_WHEELED:

					s = static_cast<int>(input.Event.MouseEvent.dwButtonState);

					Game::mouseWheel(s);
					break;
				}
			
				break;

			case KEY_EVENT:

				k = static_cast<int>(input.Event.KeyEvent.wVirtualKeyCode);

				if (input.Event.KeyEvent.bKeyDown)
					Game::keyboard(k);

				break;
			}
		}

		Game::update();
	}
}

void Game::update()
{
	static bool init = true;

	if (Sudoku::len() == 81)
	{
		Game::number = 0;
		if (Sudoku::solved())
			GameTimer::stop();
	}

	static constexpr std::array<Console::Color, 5> colors =
	{
		Console::Blue,
		Console::Green,
		Console::Orange,
		Console::Red,
		Console::Red
	};

	auto time = Game::timer_f ? GameTimer::now() : -1;
	auto info = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? _T("unsolvable") : Sudoku::rating == -1 ? _T("ambiguous") : _T(""))
	                               : (Sudoku::corrupt() ? _T("corrupt") : _T("solved"));

	Console::Fill(HDR, Console::White, colors[Sudoku::level]);

	SudokuCell *focus = tab.getCell();

	hdr.update(*this, init, info, time);
	tab.update(*this, init, focus, Game::number, Game::light_f, Game::help);
	btn.update(*this, init, focus, Game::number, Sudoku::count(Game::number), Game::help);
	mnu.update(*this, init, Game::xpos);
	ftr.update(*this, init, mnu.getInfo());

	init = false;
}

void Game::mouseMove( const int _x, const int _y )
{
	Game::xpos = _x;

	tab.mouseMove(_x, _y);
	btn.mouseMove(_x, _y);
	mnu.mouseMove(_x, _y);
}

void Game::mouseLButton( const int _x, const int _y )
{
	Game::command(hdr.mouseLButton(_x, _y));
	Game::command(tab.mouseLButton(_x, _y));
	Game::command(btn.mouseLButton(_x, _y));
	Game::command(mnu.mouseLButton(_x, _y));
}

void Game::mouseRButton( const int _x, const int _y )
{
	Game::command(tab.mouseRButton(_x, _y));
	Game::command(btn.mouseRButton(_x, _y));
}

void Game::mouseWheel( const int _d )
{
	Game::command(static_cast<Command>(_d < 0 ? (Game::number == 0 ? 1 : 1 + (Game::number + 0) % 9)
	                                          : (Game::number == 0 ? 9 : 1 + (Game::number + 7) % 9)));
}

void Game::keyboard( const int _k )
{
	switch (_k)
	{
	case '0':       /* falls through */
	case '1':       /* falls through */
	case '2':       /* falls through */
	case '3':       /* falls through */
	case '4':       /* falls through */
	case '5':       /* falls through */
	case '6':       /* falls through */
	case '7':       /* falls through */
	case '8':       /* falls through */
	case '9':       Game::command(static_cast<Command>(_k - '0')); break;
	case VK_LEFT:   Game::command(PrevHelpCmd);  break;
	case VK_RIGHT:  /* falls through */
	case 'A':       Game::command(NextHelpCmd);  break;
	case VK_NEXT:   Game::command(PrevLevelCmd); break; // PAGE DOWN
	case VK_PRIOR:  /* falls through */                 // PAGE UP
	case 'D':       Game::command(NextLevelCmd); break;
	case VK_TAB:    /* falls through */
	case 'N':       Game::command(GenerateCmd);  break;
	case 'H':       Game::command(HighLightCmd); break;
	case VK_RETURN: /* falls through */
	case 'S':       Game::command(SolveCmd);     break;
	case VK_BACK:   /* falls through */
	case 'U':       Game::command(UndoCmd);      break;
	case VK_DELETE: /* falls through */
	case 'C':       Game::command(ClearCmd);     break;
	case VK_HOME:   /* falls through */
	case 'E':       Game::command(EditCmd);      break;
	case VK_END:    /* falls through */
	case 'T':       Game::command(AcceptCmd);    break;
	case VK_INSERT: /* falls through */
	case 'V':       Game::command(SaveCmd);      break;
	case 'L':       Game::command(LoadCmd);      break;
	case VK_ESCAPE: /* falls through */
	case 'Q':       Game::command(QuitCmd);      break;
	}
}

void Game::command( const Command _c )
{
	switch (_c)
	{
	case NoCmd:         break;
	case Button0Cmd:    Game::number = 0;
	                    break;
	case Button1Cmd:    /* falls through */
	case Button2Cmd:    /* falls through */
	case Button3Cmd:    /* falls through */
	case Button4Cmd:    /* falls through */
	case Button5Cmd:    /* falls through */
	case Button6Cmd:    /* falls through */
	case Button7Cmd:    /* falls through */
	case Button8Cmd:    /* falls through */
	case Button9Cmd:    if (Sudoku::len() < 81) Game::number = static_cast<int>(_c);
	                    break;
	case ClearCellCmd:  Game::number = tab.getCell()->num;
	                    Sudoku::set(*tab.getCell(), 0);
	                    break;
	case SetCellCmd:    if (Game::number == 0 && Game::help >= Assistance::Full)
	                    Sudoku::set(*tab.getCell(),  tab.getCell()->sure());
	                    else
	                    Sudoku::set(*tab.getCell(),  Game::number, Game::help <= Assistance::Current);
	                    break;
	case PrevHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].prev());
	                    break;
	case NextHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].next());
	                    break;
	case PrevLevelCmd:  Sudoku::level = static_cast<Difficulty>(Game::mnu[0].prev());
	                    Sudoku::generate(); Game::number = 0; GameTimer::start();
                    	Game::mnu[0].setIndex(Sudoku::level);
	                    break;
	case NextLevelCmd:  Sudoku::level = static_cast<Difficulty>(Game::mnu[0].next());
	                    /* falls through */
	case GenerateCmd:   Sudoku::generate(); Game::number = 0; GameTimer::start();
                    	Game::mnu[0].setIndex(Sudoku::level);
	                    break;
	case HighLightCmd:  Game::light_f = !Game::light_f; Game::mnu[2].setIndex(Game::light_f);
	                    break;
	case TimerCmd:      Game::timer_f = !Game::timer_f;
	                    break;
	case SolveCmd:      Sudoku::solve();    Game::number = 0;
	                    if (Sudoku::len() < 81) Sudoku::rating = -2;
	                    break;
	case UndoCmd:       Sudoku::undo();
	                    break;
	case ClearCmd:      Sudoku::clear();    Game::number = 0; GameTimer::reset();
	                    break;
	case EditCmd:       Sudoku::discard();                    GameTimer::reset();
	                    break;
	case AcceptCmd:     Sudoku::accept();
	                    break;
	case SaveCmd:       Sudoku::save();
	                    break;
	case LoadCmd:       if (Sudoku::load()) Game::number = 0, GameTimer::start();
	                    break;
	case QuitCmd:       Game::alive = false;
	                    break;
	}
}

static inline
std::string cstring( TCHAR *str )
{
#if defined(UNICODE)
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str);
#else
	return std::string(str);
#endif
}

int _tmain( int argc, TCHAR **argv )
{
	std::setlocale(LC_ALL, "");

	int   cnt = 0;
	TCHAR cmd = _T('g');
	auto file = cstring(*argv) + ".board";

	if (--argc > 0 && (++argv, **argv == _T('/') || **argv == _T('-')))
		cmd = *++*argv;

	switch (std::toupper(cmd))
	{
		case _T('G'): // game
		{
			auto sudoku = Game();
			LONG style = GetWindowLong(sudoku.Console::Hwnd, GWL_STYLE);
		//	SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX));
			SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));
			sudoku();
			SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style);
			break;
		}

		case _T('F'): // find
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer<int>();
			auto data   = std::vector<uint32_t>();

			if (--argc > 0)
				file = cstring(*++argv);

			std::wcerr << ::title << " find" << std::endl;

			GetAsyncKeyState(VK_ESCAPE);
			while (!GetAsyncKeyState(VK_ESCAPE))
			{
				sudoku.generate();
				if (sudoku.level > Difficulty::Medium && sudoku.len() > 17 && std::islower(cmd))
					sudoku.raise();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					std::cout << sudoku << std::endl;
					sudoku.append(file);
				}
			}

			std::wcerr << ::title << " find: " << data.size() << " boards found, " << timer.now() << 's' << std::endl;
			break;
		}

		case _T('T'): // test
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer<int>();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, cstring(*++argv));
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::wcerr << ::title << " test: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::wcerr << ' ' << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(false))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? Sudoku::select_rating : Sudoku::select_weight);

			for (auto &tab: coll)
				std::cout << tab << std::endl;

			std::wcerr << ::title << " test: " << data.size() << " boards found, " << timer.now() << 's' << std::endl;
			break;
		}

		case _T('S'): // sort
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer<int>();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, cstring(*++argv));
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::wcerr << ::title << " sort: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::wcerr << ' ' << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(true))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? Sudoku::select_rating : Sudoku::select_length);

			for (auto &tab: coll)
				std::cout << tab << std::endl;

			std::wcerr << ::title << " sort: " << data.size() << " boards found, " << timer.now() << 's' << std::endl;
			break;
		}

		case _T('R'): // raise
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer<int>();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();


			while (--argc > 0)
				Sudoku::load(lst, cstring(*++argv));
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::wcerr << ::title << " raise: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::wcerr << ' ' << ++cnt << '\r';
				sudoku.init(i);
				sudoku.raise();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), Sudoku::select_rating);

			for (auto &tab: coll)
				std::cout << tab << std::endl;

			std::wcerr << ::title << " raise: " << data.size() << " boards found, " << timer.now() << 's' << std::endl;
			break;
		}

		case _T('?'): /* falls through */
		case _T('H'): // help
		{
			std::wcerr << "\n"
			             "Sudoku game, solver and generator\n"
			             "\n"
			             "Copyright (c) 2018 - 2020 Rajmund Szymanski. All rights reserved.\n"
			             "This software is distributed under the MIT License.\n"
			             "You are free to modify and redistribute it.\n"
			             "\n"
			             "Usage:\n"
			             "sudoku -g        - game\n"
			             "sudoku -f [file] - find and show extreme only\n"
			             "sudoku -F [file] - find and show all\n"
			             "sudoku -t [file] - test and sort by rating\n"
			             "sudoku -T [file] - test and sort by weight\n"
			             "sudoku -s [file] - sort by rating / length\n"
			             "sudoku -S [file] - sort by length / rating\n"
			             "sudoku -r [file] - raise and show extreme only\n"
			             "sudoku -R [file] - raise and show all\n"
			             "sudoku -h        - this usage help\n"
			             "sudoku -?        - this usage help\n"
			          << std::endl;
			break;
		}

		default:
		{
			std::wcerr << ::title << ": unknown command" << std::endl;
			break;
		}
	}

	return 0;
}
