/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    04.11.2020
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
#include <cstring>

const char *title = "Sudoku";

const Console::Rectangle WIN(0, 0, 42, 15);
const Console::Rectangle HDR(WIN.left, WIN.top, WIN.width, 1);
const Console::Rectangle TAB(HDR.left, HDR.bottom + 1, 25, 13);
const Console::Rectangle BTN(TAB.right + 1, TAB.top,  3, TAB.height);
const Console::Rectangle MNU(BTN.right + 1, TAB.top, 14, TAB.height);
const Console::Rectangle FTR(WIN.left, WIN.bottom, WIN.width, 1);

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
	SetSureCmd,
	PrevHelpCmd,
	NextHelpCmd,
	PrevLevelCmd,
	NextLevelCmd,
	GenerateCmd,
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

	void    update      ( Console &, bool, const char *, int, int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameCell
{
	using Cell = SudokuCell;

	const int x;
	const int y;

	Cell &cell;

public:

	static GameCell *focus;

	GameCell( const int _x, const int _y, Cell &_c ): x{_x}, y{_y}, cell{_c} {}

	Cell  & get         ()        { return cell; }
	bool    allowed     ( int n ) { return GameCell::cell.allowed(n); }
	int     sure        ( int n ) { return GameCell::cell.sure(n); }

	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameTable: public std::vector<GameCell>
{
public:

	GameTable( Sudoku & );

	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class Button
{
	const int y;
	const int num;

public:

	static Button *focus;
	static int     cur;

	Button( const int _y, const int _n ): y{_y}, num{_n} {}

	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};


class GameButtons: public std::vector<Button>
{
public:

	GameButtons();

	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class MenuItem: public std::vector<const char *>
{
	const int   y;
	const char *key;

public:

	const char *info;

	static MenuItem *focus;
	static bool      back;

	MenuItem( const int _y, const char *_k, const char *_i ): y{_y}, key{_k}, info{_i} {}

	int     index       ();
	int     prev        ();
	int     next        ();
	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameMenu: public std::vector<MenuItem>
{
public:

	GameMenu();

	void    update      ( Console &, bool );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameFooter
{
public:

	GameFooter() {}

	void update( Console &, bool );
};

/*---------------------------------------------------------------------------*/

class Game: public Console, public Sudoku, public GameTimer
{
	GameHeader  hdr;
	GameTable   tab;
	GameButtons btn;
	GameMenu    mnu;
	GameFooter  ftr;
	
	bool solved;
	bool alive;

	void run();

public:

	static Assistance help;
	static Difficulty level;
	static bool       timer_f;

	Game( Difficulty );
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
/*                              INITIALIZATION                               */
/*---------------------------------------------------------------------------*/

GameCell   *GameCell  ::focus = nullptr;

Button     *Button    ::focus = nullptr;
int         Button    ::cur   = 0;

MenuItem   *MenuItem  ::focus = nullptr;
bool        MenuItem  ::back  = false;

Assistance  Game      ::help  = Assistance::None;
Difficulty  Game      ::level = Difficulty::Easy;
bool        Game      ::timer_f = true;

/*---------------------------------------------------------------------------*/
/*                              IMPLEMENTATION                               */
/*---------------------------------------------------------------------------*/

void GameHeader::update( Console &con, bool init, const char *info, int count, int time )
{
	if (init)
	{
		con.Fill(HDR, Console::White);
		con.Put(HDR.x + 1, HDR.y, ::title);
	}

	static const Console::Color banner_color[] =
	{
		Console::Blue,
		Console::Green,
		Console::Orange,
		Console::Red,
		Console::Red
	};

	con.Fill(HDR, Console::White, banner_color[Game::level]);

	auto s = std::strlen(info);
	con.Fill(TAB.x + 9, HDR.y, TAB.width - 10 - s, 1);
	con.Put(TAB.Right(s + 1), HDR.y, info);

	auto n = Button::cur;
	auto c = Button::cur == 0 ? 0 : count;
	con.Put(BTN.x + 1, HDR.y, n == 0 || Game::help == Assistance::None ? ' ' : c > 9 ? '?' : '0' + c);

	if (Game::timer_f)
	{
		char v[16];
		std::snprintf(v, sizeof(v), "%6d:%02d:%02d", time / 3600, (time / 60) % 60, time % 60);
		con.Put(MNU.Right(std::strlen(v) + 1), HDR.y, v);
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

void GameCell::update( Console &con, bool )
{
	auto h = Game::help;
	auto n = Button::cur;
	auto f = cell.empty() ? Console::LightGrey : cell.immutable ? Console::White : Console::LightGreen;
	auto b = GameCell::focus != this ? Console::Black : Console::Grey;

	if (n != 0)
	{
		if      (h >= Assistance::Current && GameCell::cell.equal(n))   b = GameCell::focus != this ? Console::Red    : Console::LightRed;
		else if (h >= Assistance::Full    && GameCell::cell.sure(n))    b = GameCell::focus != this ? Console::Green  : Console::LightGreen;
		else if (h >= Assistance::Full    && GameCell::cell.allowed(n)) b = GameCell::focus != this ? Console::Orange : Console::Yellow;
	}

	con.Put(GameCell::x, GameCell::y, f, b);
	con.Put(GameCell::x, GameCell::y, "-123456789"[cell.num]);
}

void GameCell::mouseMove( const int _x, const int _y )
{
	if (_x == GameCell::x && _y == GameCell::y)
		GameCell::focus = this;
}

Command GameCell::mouseLButton( const int _x, const int _y )
{
	if (_x == GameCell::x && _y == GameCell::y)
	{
		if (GameCell::cell.num == 0)
		{
			if (Button::cur == 0 && Game::help >= Assistance::Full)
				return SetSureCmd;
			else
				return SetCellCmd;
		}
		else
		{
			if (GameCell::cell.immutable)
				return static_cast<Command>(GameCell::cell.num);
			else
				return ClearCellCmd;
		}
	}

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameTable::GameTable( Sudoku &_s )
{
	for (auto &c: _s)
	{
		int x = TAB.x + 2 + (c.pos % 9 + c.pos % 9 / 3) * 2;
		int y = TAB.y + 1 + (c.pos / 9 + c.pos / 9 / 3);

		GameTable::emplace_back(x, y, c);
	}
}

void GameTable::update( Console &con, bool init )
{
	if (init)
	{
		con.DrawSingle(TAB);
		con.DrawSingle(TAB.x, TAB.y + (TAB.height - 1) / 3, TAB.width, (TAB.height - 1) / 3 + 1);
		con.DrawSingle(TAB.x + (TAB.width - 1) / 3, TAB.y, (TAB.width - 1) / 3 + 1, TAB.height);
	}

	for (auto &c: *this)
		c.update(con, init);
}

void GameTable::mouseMove( const int _x, const int _y )
{
	GameCell::focus = nullptr;

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

void GameTable::mouseRButton( const int _x, const int _y )
{
	if (TAB.contains(_x, _y))
		Button::cur = 0;
}

/*---------------------------------------------------------------------------*/

void Button::update( Console &con, bool init )
{
	if (init)
		con.Put(BTN.x + 1, Button::y, '0' + num);

	auto h = Game::help;
	auto f = Button::cur == Button::num ? Console::Black : Button::focus != this ? Console::LightGrey : Console::White;
	auto b = Button::cur == Button::num ? Console::White : Button::focus != this ? Console::Black     : Console::Grey;

	if (GameCell::focus != nullptr)
	{
		if      (h >= Assistance::Sure      && GameCell::focus->sure(Button::num))    (Button::cur == Button::num ? b : f) = Console::LightGreen;
		else if (h >= Assistance::Available && GameCell::focus->allowed(Button::num)) (Button::cur == Button::num ? b : f) = Console::Yellow;
	}

	con.Put(BTN.x + 1, Button::y, f, b);
}

void Button::mouseMove( const int _x, const int _y )
{
	if (_x >= BTN.left && _x <= BTN.right && _y == Button::y)
		Button::focus = this;
}

Command Button::mouseLButton( const int _x, const int _y )
{
	if (_x >= BTN.left && _x <= BTN.right && _y == Button::y)
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

void GameButtons::update( Console &con, bool init )
{
	if (init)
		con.DrawSingle(BTN);

	for (auto &b: *this)
		b.update(con, init);
}

void GameButtons::mouseMove( const int _x, const int _y )
{
	Button::focus = nullptr;

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

void GameButtons::mouseRButton( const int _x, const int _y )
{
	if (BTN.contains(_x, _y))
		Button::cur = 0;
}

/*---------------------------------------------------------------------------*/

int MenuItem::index()
{
	switch (MenuItem::y - MNU.y)
	{
	case 1: return static_cast<int>(Game::level);
	case 2: return static_cast<int>(Game::help);
	}

	return 0;
}

int MenuItem::prev()
{
	const int max = MenuItem::size() - 1;

	switch (MenuItem::y - MNU.y)
	{
	case 1: return static_cast<int>(Game::level) == 0 ? max : static_cast<int>(Game::level) == max ? 1 : 0;
	case 2: return (static_cast<int>(Game::help) + max) % (max + 1);
	}

	return 0;
}

int MenuItem::next()
{
	const int max = MenuItem::size() - 1;

	switch (MenuItem::y - MNU.y)
	{
	case 1: return static_cast<int>(Game::level) == max ? 0 : static_cast<int>(Game::level) == 0 ? 1 : max;
	case 2: return (static_cast<int>(Game::help) + 1) % (max + 1);
	}

	return 0;
}

void MenuItem::update( Console &con, bool init )
{
	if (init)
	{
		con.Put(MNU.x + 1, MenuItem::y, key);
		con.Put(MNU.x + 4, MenuItem::y, MenuItem::at(0));
	}

	if (MenuItem::size() > 1)
	{
		con.Put(MNU.x + 1, MenuItem::y, MenuItem::focus != this ? MenuItem::key : MenuItem::back ? "<<" : ">>");
		auto i = MenuItem::index();
		auto s = std::strlen(MenuItem::at(i));
		con.Put(MNU.x + 4, MenuItem::y, MenuItem::at(i));
		con.Fill(MNU.x + 4 + s, MenuItem::y, MNU.width - 5 - s, 1);
	}

	if (MenuItem::focus == this)
		con.Fill(MNU.x + 1, MenuItem::y, MNU.width - 2, 1, Console::White, Console::Grey);
	else
		con.Fill(MNU.x + 1, MenuItem::y, MNU.width - 2, 1, Console::LightGrey);
}

void MenuItem::mouseMove( const int _x, const int _y )
{
	if (_x > MNU.left && _x < MNU.right && _y == MenuItem::y)
	{
		MenuItem::focus = this;
		MenuItem::back = _x < MNU.Center(0);
	}
}

Command MenuItem::mouseLButton( const int _x, const int _y )
{
	if (_x > MNU.left && _x < MNU.right && _y == MenuItem::y)
	{
		switch (_y)
		{
		case  2: return MenuItem::back ? PrevLevelCmd : NextLevelCmd;
		case  3: return MenuItem::back ? PrevHelpCmd  : NextHelpCmd;
		case  4: return GenerateCmd;
		case  5: return SolveCmd;
		case  6: return UndoCmd;
		case  7: return ClearCmd;
		case  8: return EditCmd;
		case  9: return AcceptCmd;
		case 10: return SaveCmd;
		case 11: return LoadCmd;
		case 12: return QuitCmd;
		}
	}

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameMenu::GameMenu()
{
	GameMenu::emplace_back(MNU.y +  1, "d:", "change difficulty level of the game");
		GameMenu::back().emplace_back("easy");
		GameMenu::back().emplace_back("medium");
		GameMenu::back().emplace_back("hard");
		GameMenu::back().emplace_back("expert");
		GameMenu::back().emplace_back("extreme");
	GameMenu::emplace_back(MNU.y +  2, "a:", "change assistance level of the game");
		GameMenu::back().emplace_back("none");
		GameMenu::back().emplace_back("current");
		GameMenu::back().emplace_back("available");
		GameMenu::back().emplace_back("sure");
		GameMenu::back().emplace_back("full");
	GameMenu::emplace_back(MNU.y +  3, "n:", "generate or load a new layout");
		GameMenu::back().emplace_back("new");
	GameMenu::emplace_back(MNU.y +  4, "s:", "solve the current layout");
		GameMenu::back().emplace_back("solve");
	GameMenu::emplace_back(MNU.y +  5, "u:", "undo last move / restore accepted layout");
		GameMenu::back().emplace_back("undo");
	GameMenu::emplace_back(MNU.y +  6, "c:", "clear the board");
		GameMenu::back().emplace_back("clear");
	GameMenu::emplace_back(MNU.y +  7, "e:", "start editing the current layout");
		GameMenu::back().emplace_back("edit");
	GameMenu::emplace_back(MNU.y +  8, "t:", "accept the layout and finish editing");
		GameMenu::back().emplace_back("accept");
	GameMenu::emplace_back(MNU.y +  9, "v:", "save the current layout to the file");
		GameMenu::back().emplace_back("save");
	GameMenu::emplace_back(MNU.y + 10, "l:", "load layout from the file");
		GameMenu::back().emplace_back("load");
	GameMenu::emplace_back(MNU.y + 11, "q:", "quit the game");
		GameMenu::back().emplace_back("quit");
}

void GameMenu::update( Console &con, bool init )
{
	if (init)
		con.DrawSingle(MNU);

	for (auto &m: *this)
		m.update(con, init);
}

void GameMenu::mouseMove( const int _x, const int _y )
{
	MenuItem::focus = nullptr;

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

/*---------------------------------------------------------------------------*/

void GameFooter::update( Console &con, bool init )
{
	if (init)
		con.Fill(FTR, Console::Black, Console::Grey);

	auto i = MenuItem::focus != nullptr ? MenuItem::focus->info : "Sudoku game, solver and generator";
	auto s = std::strlen(i);
	con.Put(FTR.x + 1, FTR.y, i);
	con.Fill(FTR.x + 1 + s, FTR.y, FTR.width - 2 - s, 1);
}

/*---------------------------------------------------------------------------*/

Game::Game( Difficulty _d ): Console(::title), Sudoku{_d}, hdr{}, tab{*this}, btn{}, mnu{}, ftr{}, solved{false}, alive{true}
{
	Console::SetFont(56, L"Consolas");
	Console::Center(WIN.width, WIN.height);
	Console::HideCursor();
	Console::Clear();
}

Game::~Game()
{
	Console::Clear();
}

void Game::run()
{
	Sudoku::generate();

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

	Game::solved = Sudoku::solved();
	Game::level  = Sudoku::level;

	if (Game::solved)
	{
		Button::cur = 0;
		GameTimer::stop();
	}

	auto time  = GameTimer::counter();
	auto count = Sudoku::count(Button::cur);
	auto info  = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? "unsolvable :(" : Sudoku::rating == -1 ? "ambiguous :/" : "")
	                                : (Sudoku::corrupt() ? "corrupt  :(" : "solved :)");

	hdr.update(*this, init, info, count, time);
	tab.update(*this, init);
	btn.update(*this, init);
	mnu.update(*this, init);
	ftr.update(*this, init);

	init = false;
}

void Game::mouseMove( const int _x, const int _y )
{
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
	tab.mouseRButton(_x, _y);
	btn.mouseRButton(_x, _y);
}

void Game::mouseWheel( const int _d )
{
	Game::command(static_cast<Command>(_d < 0 ? (Button::cur == 0 ? 1 : 1 + (Button::cur + 0) % 9)
	                                          : (Button::cur == 0 ? 9 : 1 + (Button::cur + 7) % 9)));
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
	case Button0Cmd:    /* falls through */
	case Button1Cmd:    /* falls through */
	case Button2Cmd:    /* falls through */
	case Button3Cmd:    /* falls through */
	case Button4Cmd:    /* falls through */
	case Button5Cmd:    /* falls through */
	case Button6Cmd:    /* falls through */
	case Button7Cmd:    /* falls through */
	case Button8Cmd:    /* falls through */
	case Button9Cmd:    if (Sudoku::len() < 81) Button::cur = static_cast<int>(_c);
	                    break;
	case ClearCellCmd:  Button::cur = GameCell::focus->get().num;
	                    Sudoku::set(GameCell::focus->get(), 0);
	                    break;
	case SetCellCmd:    Sudoku::set(GameCell::focus->get(), Button::cur, Game::help <= Assistance::Current);
	                    break;
	case SetSureCmd:    Sudoku::set(GameCell::focus->get(), GameCell::focus->get().sure());
	                    break;
	case PrevHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].prev());
	                    break;
	case NextHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].next());
	                    break;
	case PrevLevelCmd:  Sudoku::level = Game::level = static_cast<Difficulty>(Game::mnu[0].prev());
	                    Sudoku::generate(); Button::cur = 0; GameTimer::start();
	                    break;
	case NextLevelCmd:  Sudoku::level = Game::level = static_cast<Difficulty>(Game::mnu[0].next());
	                    /* falls through */
	case GenerateCmd:   Sudoku::generate(); Button::cur = 0; GameTimer::start();
	                    break;
	case TimerCmd:      Game::timer_f = !Game::timer_f;
	                    break;
	case SolveCmd:      Sudoku::solve();    Button::cur = 0;
	                    if (Sudoku::len() < 81) Sudoku::rating = -2;
	                    break;
	case UndoCmd:       Sudoku::undo();
	                    break;
	case ClearCmd:      Sudoku::clear();    Button::cur = 0; GameTimer::reset();
	                    break;
	case EditCmd:       Sudoku::discard();                   GameTimer::reset();
	                    break;
	case AcceptCmd:     Sudoku::accept();
	                    break;
	case SaveCmd:       Sudoku::save();
	                    break;
	case LoadCmd:       if (Sudoku::load()) Button::cur = 0, GameTimer::start();
	                    break;
	case QuitCmd:       Game::alive = false;
	                    break;
	}
}

int main( int argc, char **argv )
{
	int  cnt  = 0;
	char cmd  = 'g';
	auto file = std::string(*argv) + ".board";

	if (--argc > 0 && (**++argv == '/' || **argv == '-'))
		cmd = *++*argv;

	switch (std::toupper(cmd))
	{
		case 'G': // game
		{
			auto sudoku = Game(std::islower(cmd) ? Difficulty::Easy : Difficulty::Medium);
			LONG style = GetWindowLong(sudoku.Console::Hwnd, GWL_STYLE);
		//	SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX));
			SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));
			sudoku();
			SetWindowLong(sudoku.Console::Hwnd, GWL_STYLE, style);
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer();
			auto data   = std::vector<uint32_t>();

			if (--argc > 0)
				file = *++argv;

			std::cerr << ::title << " find" << std::endl;

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

			std::cerr << ::title << " find: " << data.size() << " boards found, " << timer.counter() << 's' << std::endl;
			break;
		}

		case 'T': // test
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " test: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::cerr << ' ' << ++cnt << '\r';
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

			std::cerr << ::title << " test: " << data.size() << " boards found, " << timer.counter() << 's' << std::endl;
			break;
		}

		case 'S': // sort
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " sort: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::cerr << ' ' << ++cnt << '\r';
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

			std::cerr << ::title << " sort: " << data.size() << " boards found, " << timer.counter() << 's' << std::endl;
			break;
		}

		case 'R': // raise
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto timer  = GameTimer();
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();


			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " raise: " << lst.size() << " boards loaded" << std::endl;

			for (std::string &i: lst)
			{
				std::cerr << ' ' << ++cnt << '\r';
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

			std::cerr << ::title << " raise: " << data.size() << " boards found, " << timer.counter() << 's' << std::endl;
			break;
		}

		case '?': /* falls through */
		case 'H': // help
		{
			std::cerr << "\n"
			             "Sudoku game, solver and generator\n"
			             "\n"
			             "Copyright (c) 2018 - 2020 Rajmund Szymanski. All rights reserved.\n"
			             "This software is distributed under the MIT License.\n"
			             "You are free to modify and redistribute it.\n"
			             "\n"
			             "Usage:\n"
			             "sudoku -g        - game (easy level by default)\n"
			             "sudoku -G        - game (medium / hard / expert)\n"
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
			std::cerr << ::title << ": unknown command" << std::endl;
			break;
		}
	}
}
