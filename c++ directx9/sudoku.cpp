/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    07.11.2020
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
#include <windowsx.h>
#include "sudoku.hpp"
#include "graphics.hpp"
#include "gametimer.hpp"
#include <tchar.h>

const TCHAR *title = _T("Sudoku");

constexpr auto Frame     {  0 };
constexpr auto Margin    {  4 };
constexpr auto CellSize  { 64 };
constexpr auto SegSize   { CellSize * 3 + Margin * 2 };
constexpr auto TabSize   { SegSize  * 3 + Margin * 8 };
constexpr auto MnuSize   { 12 };

const Graphics::Rect TAB(Frame + Margin * 2, Frame + CellSize, TabSize, TabSize);
const Graphics::Rect BTN(TAB.right + Margin * 8, TAB.top, CellSize, TAB.height);
const Graphics::Rect MNU(BTN.right + Margin * 8, TAB.top, SegSize,  TAB.height);
const Graphics::Rect HDR(TAB.left, Frame, MNU.right - TAB.left, TAB.top - Frame);
const Graphics::Rect FTR(TAB.left, TAB.bottom, HDR.width, CellSize / 2);
const Graphics::Rect WIN(0, 0, HDR.left + HDR.right, FTR.bottom + Frame);

constexpr Graphics::Color Background = Graphics::Color::Moccasin;
constexpr Graphics::Color Lighted    = Graphics::Color::OldLace;

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
	static Graphics::Font *font;
	static Graphics::Font *tiny;

public:

	GameHeader() {}

	void    update      ( Graphics &, const TCHAR *, int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameCell
{
	const Graphics::Rect r;

	using Cell = SudokuCell;
	Cell &cell;

	static Graphics::Font *font;

public:

	static GameCell *focus;

	GameCell( const auto _x, const auto _y, Cell &_c ): r{_x, _y, CellSize, CellSize}, cell{_c} {}

	Cell  & get         ()        { return cell; }
	bool    allowed     ( int n ) { return GameCell::cell.allowed(n); }
	int     sure        ( int n ) { return GameCell::cell.sure(n); }

	void    update      ( Graphics & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameTable: public std::vector<GameCell>
{
public:

	GameTable( Sudoku & );

	void    update      ( Graphics & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class Button: public GameTimer<int, std::ratio<1, 50>>
{
	const Graphics::Rect r;
	const int num;

	static Graphics::Font *font;
	static Graphics::Font *tiny;

public:

	static Button *focus;

	Button( const auto _y, const int _n ): r{BTN.x, _y, BTN.width, CellSize}, num{_n} {}

	void    update      ( Graphics &, int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};


class GameButtons: public std::vector<Button>
{
public:

	GameButtons();

	void    update      ( Graphics &, int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class MenuItem: public std::vector<const TCHAR *>, public GameTimer<int, std::ratio<1, 100>>
{
	const Graphics::Rect r;
	const int idx;
	const TCHAR *info;

	static Graphics::Font *font;

public:

	static MenuItem *focus;
	static bool back;

	MenuItem( const int _n, const auto _y, const auto _h, const TCHAR *_i ): r{MNU.x, _y, MNU.width, _h}, idx{_n}, info{_i} {}

	int          index       ();
	int          prev        ();
	int          next        ();
	void         update      ( Graphics & );
	const TCHAR *mouseMove   ( const int, const int );
	Command      mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameMenu: public std::vector<MenuItem>
{
public:

	GameMenu();

	void         update      ( Graphics & );
	const TCHAR *mouseMove   ( const int, const int );
	Command      mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameFooter
{
	static Graphics::Font *font;

public:

	GameFooter() {}

	void update( Graphics &, const TCHAR * );
};

/*---------------------------------------------------------------------------*/

class Game: public Graphics, public Sudoku, public GameTimer<int>
{
	GameHeader  hdr;
	GameTable   tab;
	GameButtons btn;
	GameMenu    mnu;
	GameFooter  ftr;
	
	bool tracking;
	const TCHAR *about;

public:

	static Difficulty level;
	static Assistance help;

	static int  number;
	static bool light_f;
	static bool timer_f;

	Game();

	void update      ();
	void mouseMove   ( const int, const int, HWND );
	void mouseLeave  ();
	void mouseLButton( const int, const int );
	void mouseRButton( const int, const int );
	void mouseWheel  ( const int, const int, const int );
	void keyboard    ( const int );
	void command     ( Command );
};

/*---------------------------------------------------------------------------*/
/*                              INITIALIZATION                               */
/*---------------------------------------------------------------------------*/

Graphics::Font *GameHeader::font = nullptr;
Graphics::Font *GameHeader::tiny = nullptr;
Graphics::Font *GameCell  ::font = nullptr;
Graphics::Font *Button    ::font = nullptr;
Graphics::Font *Button    ::tiny = nullptr;
Graphics::Font *MenuItem  ::font = nullptr;
Graphics::Font *GameFooter::font = nullptr;

GameCell       *GameCell  ::focus = nullptr;
Button         *Button    ::focus = nullptr;
MenuItem       *MenuItem  ::focus = nullptr;

bool            MenuItem  ::back   = false;

Difficulty      Game      ::level  = Difficulty::Easy;
Assistance      Game      ::help   = Assistance::None;

int             Game      ::number  = 0;
bool            Game      ::light_f = false;
bool            Game      ::timer_f = true;

/*---------------------------------------------------------------------------*/
/*                                  SUDOKU                                   */
/*---------------------------------------------------------------------------*/

auto game = Game();

/*---------------------------------------------------------------------------*/
/*                              IMPLEMENTATION                               */
/*---------------------------------------------------------------------------*/

void GameHeader::update( Graphics &gr, const TCHAR *info, int time )
{
	if (GameHeader::font == nullptr)
		GameHeader::font = gr.font(HDR.height, FW_MEDIUM, FIXED_PITCH | FF_DECORATIVE, _T("Tahoma"));

	if (GameHeader::tiny == nullptr)
		GameHeader::tiny = gr.font(HDR.height / 2,  FW_NORMAL, VARIABLE_PITCH, _T("Arial"));

	gr.draw_text(HDR, GameHeader::font, Graphics::Color::Black, Graphics::Alignment::Left, ::title);

	if (info != nullptr)
		gr.draw_text(HDR, GameHeader::tiny, Graphics::Color::Red, Graphics::Alignment::Center, info);

	if (Game::timer_f)
	{
		TCHAR v[16];
		_sntprintf(v, sizeof(v), _T("%6d:%02d:%02d"), time / 3600, (time / 60) % 60, time % 60);
		gr.draw_text(HDR, GameHeader::font, Graphics::Color::DimGray, Graphics::Alignment::Right, v);
	}
}

Command GameHeader::mouseLButton( const int _x, const int _y )
{
	if (HDR.contains(_x, _y))
		return TimerCmd;

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

void GameCell::update( Graphics &gr )
{
	if (GameCell::font == nullptr)
		GameCell::font = gr.font(CellSize, FW_BLACK, FIXED_PITCH | FF_DECORATIVE, _T("Tahoma"));

	auto h = Game::help;
	auto n = Game::number;
	auto f = GameCell::cell.empty() ? (GameCell::focus == this ? Lighted : Background)
	                                : (GameCell::cell.immutable ? Graphics::Color::Black : Graphics::Color::Green);

	if (n != 0)
	{
		if      (h >= Assistance::Current && GameCell::cell.equal(n))   f = Graphics::Color::Red;
		else if (h >= Assistance::Full    && GameCell::cell.sure(n))    f = Graphics::Color::Green;
		else if (h >= Assistance::Full    && GameCell::cell.allowed(n)) f = Graphics::Color::Orange;
	}

	if (GameCell::focus == this || (Game::light_f && GameCell::focus != nullptr && GameCell::cell.in_lst(GameCell::focus->cell)))
		gr.fill_rect(GameCell::r, Margin, Lighted);

	if (!GameCell::cell.empty())
		gr.draw_char(GameCell::r, GameCell::font, f, Graphics::Alignment::Center, _T(" 123456789")[GameCell::cell.num]);
	else
	if (f != Background)
		gr.fill_rect(GameCell::r, CellSize / 3, f);

	gr.draw_rect(GameCell::r, Graphics::Color::Black);
}

void GameCell::mouseMove( const int _x, const int _y )
{
	if (GameCell::r.contains(_x, _y))
		GameCell::focus = this;
}

Command GameCell::mouseLButton( const int _x, const int _y )
{
	if (GameCell::r.contains(_x, _y))
	{
		if (GameCell::cell.num == 0)
		{
			if (Game::number == 0 && Game::help >= Assistance::Full)
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
		auto x = TAB.x + (c.pos % 9) * (CellSize + Margin) + (c.pos % 9 / 3) * Margin * 3;
		auto y = TAB.y + (c.pos / 9) * (CellSize + Margin) + (c.pos / 9 / 3) * Margin * 3;

		GameTable::emplace_back(x, y, c);
	}
}

void GameTable::update( Graphics &gr )
{
	for (auto &c: *this)
		c.update(gr);
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
		Game::number = 0;
}

/*---------------------------------------------------------------------------*/

void Button::update( Graphics &gr, int count )
{
	if (Button::font == nullptr)
		Button::font = gr.font(CellSize, FW_BLACK, FIXED_PITCH | FF_DECORATIVE, _T("Tahoma"));

	if (Button::tiny == nullptr)
		Button::tiny = gr.font(CellSize / 2, FW_NORMAL, FIXED_PITCH | FF_DECORATIVE, _T("Arial"));

	auto h = Game::help;
	auto f = Graphics::Color::DimGray;

	if (Button::focus == this)
		gr.fill_rect(Button::r, Margin, Lighted);
	else
	if (Game::number == Button::num)
		gr.fill_rect(Button::r, Graphics::Color::White);

	static constexpr std::array<Graphics::Color, 5> colors =
	{
		Graphics::Color::Black,
		Graphics::Color::DimGray,
		Graphics::Color::Gray,
		Graphics::Color::DarkGray,
		Graphics::Color::LightGray,
	};

	if (Game::number == Button::num)
		for (int i = GameTimer::from(4); i >= 0; i--)
			gr.draw_rect(Button::r, i, colors[i]);
	else
		gr.draw_rect(Button::r, Graphics::Color::Black);

	if (GameCell::focus != nullptr)
	{
		if      (h >= Assistance::Sure      && GameCell::focus->sure(Button::num))    f = Graphics::Color::Green;
		else if (h >= Assistance::Available && GameCell::focus->allowed(Button::num)) f = Graphics::Color::Orange;
	}

	gr.draw_char(Button::r, Button::font, f, Graphics::Alignment::Center, _T("0123456789")[Button::num]);

	if (Game::number == Button::num && Game::help > Assistance::None)
	{
		auto rc = Graphics::Rect(BTN.right, Button::r.top, MNU.left - BTN.right, Button::r.height);
		gr.draw_char(rc, Button::tiny, Graphics::Color::White, Graphics::Alignment::Bottom, count > 9 ? _T('?') : _T("0123456789")[count]);
	}
}

void Button::mouseMove( const int _x, const int _y )
{
	if (Button::r.contains(_x, _y))
		Button::focus = this;
}

Command Button::mouseLButton( const int _x, const int _y )
{
	if (Button::r.contains(_x, _y))
	{
		GameTimer::restart();

		return static_cast<Command>(Button::num);
	}

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameButtons::GameButtons()
{
	for (int n = 1; n <= 9; n++)
	{
		auto y = BTN.y + std::round((n - 1) * (BTN.height - CellSize) / 8);

		GameButtons::emplace_back(y, n);
	}
}

void GameButtons::update( Graphics &gr, int count )
{
	for (auto &b: *this)
		b.update(gr, count);
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
		Game::number = 0;
}

/*---------------------------------------------------------------------------*/

int MenuItem::index()
{
	switch (MenuItem::idx)
	{
	case 0: return static_cast<int>(Game::level);
	case 1: return static_cast<int>(Game::help);
	case 2: return Game::light_f ? 1 : 0;
	}

	return 0;
}

int MenuItem::prev()
{
	const int max = MenuItem::size() - 1;

	switch (MenuItem::idx)
	{
	case 0: return static_cast<int>(Game::level) == 0 ? max : static_cast<int>(Game::level) == max ? 1 : 0;
	case 1: return (static_cast<int>(Game::help) + max) % (max + 1);
	case 2: return Game::light_f ? 0 : 1;
	}

	return 0;
}

int MenuItem::next()
{
	const int max = MenuItem::size() - 1;

	switch (MenuItem::idx)
	{
	case 0: return static_cast<int>(Game::level) == max ? 0 : static_cast<int>(Game::level) == 0 ? 1 : max;
	case 1: return (static_cast<int>(Game::help) + 1) % (max + 1);
	case 2: return Game::light_f ? 0 : 1;
	}

	return 0;
}

void MenuItem::update( Graphics &gr )
{
	static constexpr auto h = std::round(TabSize / 1.8f / MnuSize);

	if (MenuItem::font == nullptr)
		MenuItem::font = gr.font(h, FW_NORMAL, VARIABLE_PITCH, _T("Arial"));

	if (MenuItem::focus == this)
		gr.fill_rect(MenuItem::r, GameTimer::until(Margin * 4), Lighted);

	if (MenuItem::size() > 1)
	{
		auto cl = MenuItem::focus != this ? Lighted : MenuItem::back ? Graphics::Color::Black : Background;
		auto cr = MenuItem::focus != this ? Lighted : MenuItem::back ? Background : Graphics::Color::Black;

#if defined(UNICODE)
		gr.draw_char(MenuItem::r, MenuItem::font, cl, Graphics::Alignment::Left,  _T('◄'));
		gr.draw_char(MenuItem::r, MenuItem::font, cr, Graphics::Alignment::Right, _T('►'));
#else
		gr.draw_left (MenuItem::r, std::round(TabSize / 1.8 / MnuSize), cl);
		gr.draw_right(MenuItem::r, std::round(TabSize / 1.8 / MnuSize), cr);
#endif
	}

	gr.draw_text(MenuItem::r, MenuItem::font, Graphics::Color::Black, Graphics::Alignment::Center, MenuItem::at(MenuItem::index()));
}

const TCHAR *MenuItem::mouseMove( const int _x, const int _y )
{
	if (MenuItem::r.contains(_x, _y))
	{
		MenuItem::focus = this;
		MenuItem::back = _x < MNU.center;
		return MenuItem::info;
	}

	return nullptr;
}

Command MenuItem::mouseLButton( const int _x, const int _y )
{
	if (MenuItem::r.contains(_x, _y))
	{
		GameTimer::restart();

		switch (MenuItem::idx)
		{
		case  0: return MenuItem::back ? PrevLevelCmd : NextLevelCmd;
		case  1: return MenuItem::back ? PrevHelpCmd  : NextHelpCmd;
		case  2: return HighLightCmd;
		case  3: return GenerateCmd;
		case  4: return SolveCmd;
		case  5: return UndoCmd;
		case  6: return ClearCmd;
		case  7: return EditCmd;
		case  8: return AcceptCmd;
		case  9: return SaveCmd;
		case 10: return LoadCmd;
		case 11: return QuitCmd;
		}
	}

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameMenu::GameMenu()
{
	static constexpr auto h = std::round(TabSize / 1.2f / MnuSize);
	auto pos = []( const auto i ){ return MNU.y + std::round(i * (MNU.height - h) / (MnuSize - 1)); };

	GameMenu::emplace_back( 0, pos( 0), h, _T("Change the difficulty level: easy, medium / hard / expert, extreme (keyboard shortcuts: D, PgUp, PgDn)"));
		GameMenu::back().emplace_back(_T("easy"));
		GameMenu::back().emplace_back(_T("medium"));
		GameMenu::back().emplace_back(_T("hard"));
		GameMenu::back().emplace_back(_T("expert"));
		GameMenu::back().emplace_back(_T("extreme"));
	GameMenu::emplace_back( 1, pos( 1), h, _T("Change the assistance level: none, current, available, sure, full (keyboard shortcuts: A, Left, Right)"));
		GameMenu::back().emplace_back(_T("none"));
		GameMenu::back().emplace_back(_T("current"));
		GameMenu::back().emplace_back(_T("available"));
		GameMenu::back().emplace_back(_T("sure"));
		GameMenu::back().emplace_back(_T("full"));
	GameMenu::emplace_back( 2, pos( 2), h, _T("Display the highlight of the entire group of cells: on / off (keyboard shortcut: H)"));
		GameMenu::back().emplace_back(_T("highlight off"));
		GameMenu::back().emplace_back(_T("highlight on"));
	GameMenu::emplace_back( 3, pos( 3), h, _T("Generate or load a new layout (keyboard shortcuts: N, Tab)"));
		GameMenu::back().emplace_back(_T("new"));
	GameMenu::emplace_back( 4, pos( 4), h, _T("Solve the current layout (keyboard shortcuts: S, Enter)"));
		GameMenu::back().emplace_back(_T("solve"));
	GameMenu::emplace_back( 5, pos( 5), h, _T("Undo last move or restore the accepted layout (keyboard shortcuts: U, Backspace)"));
		GameMenu::back().emplace_back(_T("undo"));
	GameMenu::emplace_back( 6, pos( 6), h, _T("Clear the board (keyboard shortcuts: C, Delete)"));
		GameMenu::back().emplace_back(_T("clear"));
	GameMenu::emplace_back( 7, pos( 7), h, _T("Start editing the current layout (keyboard shortcuts: E, Home)"));
		GameMenu::back().emplace_back(_T("edit"));
	GameMenu::emplace_back( 8, pos( 8), h, _T("Accept the current layout and finish editing (keyboard shortcuts: T, End)"));
		GameMenu::back().emplace_back(_T("accept"));
	GameMenu::emplace_back( 9, pos( 9), h, _T("Save the current layout to the file (keyboard shortcuts: V, Insert)"));
		GameMenu::back().emplace_back(_T("save"));
	GameMenu::emplace_back(10, pos(10), h, _T("Load layout from the file (keyboard shortcut: L)"));
		GameMenu::back().emplace_back(_T("load"));
	GameMenu::emplace_back(11, pos(11), h, _T("Quit the game (keyboard shortcuts: Q, Esc)"));
		GameMenu::back().emplace_back(_T("quit"));
}

void GameMenu::update( Graphics &gr )
{
	for (auto &m: *this)
		m.update(gr);

	gr.draw_rect(MNU, Graphics::Color::Black);
}

const TCHAR *GameMenu::mouseMove( const int _x, const int _y )
{
	MenuItem::focus = nullptr;

	const TCHAR *info = nullptr;
	for (auto &m: *this)
		if (info = m.mouseMove(_x, _y), info != nullptr)
			break;

	return info;
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

void GameFooter::update( Graphics &gr, const TCHAR *about )
{
	if (GameFooter::font == nullptr)
		GameFooter::font = gr.font(std::min(FTR.height, WIN.width / 40), FW_NORMAL, VARIABLE_PITCH, _T("Arial"));

	if (about == nullptr)
		about = _T("Sudoku game, solver and generator");
	gr.draw_text(FTR, GameFooter::font, Graphics::Color::Gray, Graphics::Alignment::Center, about);
}

/*---------------------------------------------------------------------------*/

Game::Game(): hdr{}, tab{*this}, btn{}, mnu{}, ftr{}, tracking{false}, about{nullptr}
{
	Sudoku::generate();
}

void Game::update()
{
	Game::level = Sudoku::level;

	if (Sudoku::len() == 81)
	{
		Game::number = 0;
		if (Sudoku::solved())
			GameTimer::stop();
	}

	auto time  = GameTimer::now();
	auto count = Sudoku::count(Game::number);
	auto info  = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? _T("UNSOLVABLE") : Sudoku::rating == -1 ? _T("AMBIGUOUS") : nullptr)
	                                : (Sudoku::corrupt() ? _T("CORRUPT") : _T("SOLVED"));

	Graphics::begin(Background);

	static constexpr std::array<Graphics::Color, 5> colors =
	{
		Graphics::Color::Blue,
		Graphics::Color::Green,
		Graphics::Color::Orange,
		Graphics::Color::Crimson,
		Graphics::Color::Crimson,
	};

	Graphics::fill_rect(Graphics::Rect(TAB.x + CellSize * 3 + Margin * 3, TAB.y, Margin * 2, TAB.height), Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x + CellSize * 6 + Margin * 9, TAB.y, Margin * 2, TAB.height), Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x, TAB.y + CellSize * 3 + Margin * 3, TAB.width, Margin * 2),  Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x, TAB.y + CellSize * 6 + Margin * 9, TAB.width, Margin * 2),  Graphics::Color::DimGray);

	Graphics::fill_rect(Graphics::Rect(TAB.right + Margin, TAB.y, Margin * 6, TAB.height), colors[Game::level]);
	Graphics::fill_rect(Graphics::Rect(BTN.right + Margin, TAB.y, Margin * 6, TAB.height), colors[Game::level]);

	hdr.update(*this, info, time);
	tab.update(*this);
	btn.update(*this, count);
	mnu.update(*this);
	ftr.update(*this, Game::about);

	Graphics::end();
}

void Game::mouseMove( const int _x, const int _y, HWND hWnd )
{
	tab.mouseMove(_x, _y);
	btn.mouseMove(_x, _y);
	Game::about = mnu.mouseMove(_x, _y);

	if (!Game::tracking)
	{
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
		TrackMouseEvent(&tme);
		Game::tracking = true;
	}
}

void Game::mouseLeave()
{
	GameCell::focus = nullptr;
	Button  ::focus = nullptr;
	MenuItem::focus = nullptr;

	Game::tracking = false;
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

void Game::mouseWheel( const int, const int, const int _d )
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
	case Button0Cmd:    /* falls through */
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
	case ClearCellCmd:  Game::number = GameCell::focus->get().num;
	                    Sudoku::set(GameCell::focus->get(), 0);
	                    break;
	case SetCellCmd:    Sudoku::set(GameCell::focus->get(),  Game::number, Game::help <= Assistance::Current);
	                    break;
	case SetSureCmd:    Sudoku::set(GameCell::focus->get(),  GameCell::focus->get().sure());
	                    break;
	case PrevHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].prev());
	                    break;
	case NextHelpCmd:   Game::help = static_cast<Assistance>(Game::mnu[1].next());
	                    break;
	case PrevLevelCmd:  Sudoku::level = Game::level = static_cast<Difficulty>(Game::mnu[0].prev());
	                    Sudoku::generate(); Game::number = 0; GameTimer::start();
	                    break;
	case NextLevelCmd:  Sudoku::level = Game::level = static_cast<Difficulty>(Game::mnu[0].next());
	                    /* falls through */
	case GenerateCmd:   Sudoku::generate(); Game::number = 0; GameTimer::start();
	                    break;
	case HighLightCmd:  Game::light_f = !Game::light_f;
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
	case QuitCmd:       Graphics::quit();
	                    break;
	}
}

//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	const int w = rc.right - rc.left;
	const int h = rc.bottom - rc.top;
	const int k = GET_KEYSTATE_WPARAM(wParam);
	const int d = GET_WHEEL_DELTA_WPARAM(wParam);
	const int x = w > 0 ? std::round(static_cast<FLOAT>(GET_X_LPARAM(lParam)) * WIN.width  / w) : 0;
	const int y = h > 0 ? std::round(static_cast<FLOAT>(GET_Y_LPARAM(lParam)) * WIN.height / h) : 0;

	switch (msg)
	{
		case WM_MOUSEMOVE:   game.mouseMove(x, y, hWnd); break;
		case WM_MOUSELEAVE:  game.mouseLeave();          break;
		case WM_LBUTTONDOWN: game.mouseLButton(x, y);    break;
		case WM_RBUTTONDOWN: game.mouseRButton(x, y);    break;
		case WM_MOUSEWHEEL:  game.mouseWheel(x, y, d);   break;
		case WM_KEYDOWN:     game.keyboard(k);           break;
		case WM_DESTROY:     PostQuitMessage(0);                 break;
		default:      return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return FALSE;
}

//----------------------------------------------------------------------------
int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow )
{
	WNDCLASSEX wc = {};
	wc.cbSize        = sizeof(wc);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = ::title;
	RegisterClassEx(&wc);

	RECT rc = WIN;
	DWORD style = WS_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&rc, style, FALSE, 0);
	SIZE s = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	SIZE w = { rc.right - rc.left, rc.bottom - rc.top };

	HWND hWnd = CreateWindowEx(0, ::title, ::title, style,
	                          (s.cx - w.cx) / 2, (s.cy - w.cy) / 2, w.cx, w.cy,
	                          GetDesktopWindow(), NULL, hInstance, NULL);

	if (!game.Graphics::init(hWnd))
		return 0;

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	for (;;)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

			continue;
		}

		game.update();
	}

	return 0;
}
