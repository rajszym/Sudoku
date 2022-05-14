/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    14.05.2022
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

using Cell = SudokuCell;

const TCHAR *title = _T("Sudoku");

constexpr FLOAT Frame     {  0.5f };
constexpr FLOAT Margin    {  4 };
constexpr FLOAT CellSize  { 64 };
constexpr FLOAT SegSize   { CellSize * 3 + Margin * 2 };
constexpr FLOAT TabSize   { SegSize  * 3 + Margin * 8 };
constexpr FLOAT MnuSize   { 12 };

const Graphics::Rect TAB(Frame + Margin * 2, Frame + CellSize + Margin * 2, TabSize, TabSize);
const Graphics::Rect MNU(TAB.right + Margin, TAB.top, SegSize,  TAB.height);
const Graphics::Rect HDR(Frame, Frame, TAB.left + MNU.right, TAB.top - TAB.left);
const Graphics::Rect FTR(HDR.left, TAB.bottom, HDR.width, CellSize / 2);
const Graphics::Rect WIN(0, 0, HDR.left + HDR.right, FTR.bottom + Frame);

const Graphics::Color Background = Graphics::Color::Moccasin;
const Graphics::Color Lighted    = Graphics::Color::OldLace;

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
	Button0Cmd,
	Button1Cmd,
	Button2Cmd,
	Button3Cmd,
	Button4Cmd,
	Button5Cmd,
	Button6Cmd,
	Button7Cmd,
	Button8Cmd,
	Button9Cmd,
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

	void    update      ( Graphics &, const TCHAR *, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameCell
{
	Graphics::Rect r;
	Cell* const cell;

	bool focused;

	static Graphics::Font *font;
	static Graphics::Font *tiny;

	bool allowed( const uint, const Assistance );

public:

	GameCell( const auto _x, const auto _y, Cell* const _c ): r{_x, _y, CellSize, CellSize}, cell{_c}, focused{false} {}

	void    update      ( Graphics &, const uint, const Assistance, Cell* const, const bool );
	void    mouseMove   ( const int, const int );
	void    mouseLeave  ();
	Command mouseLButton( const int, const int, const uint, const Assistance );
	Command mouseRButton( const int, const int );
	Cell  * getCell     ();
};

/*---------------------------------------------------------------------------*/

class GameTable: public std::vector<GameCell>
{
public:

	GameTable( Sudoku & );

	void    update      ( Graphics &, const uint, const Assistance, Cell* const, const bool );
	void    mouseMove   ( const int, const int );
	void    mouseLeave  ();
	Command mouseLButton( const int, const int, const uint, const Assistance );
	Command mouseRButton( const int, const int );
	Cell  * getCell     ();
};

/*---------------------------------------------------------------------------*/

class MenuItem: public std::vector<const TCHAR *>, public GameTimer<int, std::ratio<1, 100>>
{
	Graphics::Rect r;
	const TCHAR * const info;
	const int num;

	uint idx;
	bool focused;

	static Graphics::Font *font;

public:

	MenuItem( const int _n, const auto _y, const auto _h, const TCHAR *_i ): r{MNU.x, _y, MNU.width, _h}, info{_i}, num{_n}, idx{0}, focused{false} {}

	void    update      ( Graphics &, const int );
	void    mouseMove   ( const int, const int );
	void    mouseLeave  ();
	Command mouseLButton( const int, const int );
	const TCHAR *getInfo();
	template<typename T>
	void    setIndex    ( const T );
	uint    prev        ();
	uint    next        ();
};

/*---------------------------------------------------------------------------*/

class GameMenu: public std::vector<MenuItem>
{
public:

	GameMenu();

	void    update      ( Graphics &, const int );
	void    mouseMove   ( const int, const int );
	void    mouseLeave  ();
	Command mouseLButton( const int, const int );
	const TCHAR *getInfo();
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
	GameMenu    mnu;
	GameFooter  ftr;
	
	uint number;
	bool tracking;
	bool timer_f;
	bool light_f;

	Assistance help;

	bool set    ( uint = 0 );
	void command( Command );

public:

	Game();

	void update      ( HWND );
	void mouseMove   ( const int, const int, HWND );
	void mouseLeave  ();
	void mouseLButton( const int, const int );
	void mouseRButton( const int, const int );
	void mouseWheel  ( const int, const int, const int, HWND );
	void keyboard    ( const int );
};

/*---------------------------------------------------------------------------*/
/*                              INITIALIZATION                               */
/*---------------------------------------------------------------------------*/

Graphics::Font *GameHeader::font = nullptr;
Graphics::Font *GameHeader::tiny = nullptr;
Graphics::Font *GameCell  ::font = nullptr;
Graphics::Font *GameCell  ::tiny = nullptr;
Graphics::Font *MenuItem  ::font = nullptr;
Graphics::Font *GameFooter::font = nullptr;

/*---------------------------------------------------------------------------*/
/*                                  SUDOKU                                   */
/*---------------------------------------------------------------------------*/

auto game = Game();

/*---------------------------------------------------------------------------*/
/*                              IMPLEMENTATION                               */
/*---------------------------------------------------------------------------*/

void GameHeader::update( Graphics &gr, const TCHAR *info, const int time )
{
	if (GameHeader::font == nullptr)
		GameHeader::font = gr.font(HDR.height, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STRETCH_NORMAL, _T("Tahoma"));

	if (GameHeader::tiny == nullptr)
		GameHeader::tiny = gr.font(HDR.height / 2, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_CONDENSED, _T("Arial"));

	auto rc = Graphics::Rect::deflate(HDR, TAB.left);
	gr.draw_text(rc, GameHeader::font, Graphics::Color::White, Graphics::Alignment::Left, ::title);

	if (info != nullptr)
		gr.draw_text(rc, GameHeader::tiny, Graphics::Color::White, Graphics::Alignment::Center, info);

	if (time >= 0)
	{
		TCHAR v[16];
		_sntprintf(v, sizeof(v), _T("%6d:%02d:%02d"), time / 3600, (time / 60) % 60, time % 60);
		gr.draw_text(rc, GameHeader::font, Graphics::Color::Silver, Graphics::Alignment::Right, v);
	}
}

Command GameHeader::mouseLButton( const int _x, const int _y )
{
	if (HDR.contains(_x, _y))
		return TimerCmd;

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

void GameCell::update( Graphics &gr, const uint number, const Assistance help, Cell* const focus, const bool light )
{
	if (GameCell::font == nullptr)
		GameCell::font = gr.font(CellSize, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STRETCH_NORMAL, _T("Tahoma"));

	if (GameCell::tiny == nullptr)
		GameCell::tiny = gr.font(CellSize / 3, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STRETCH_NORMAL, _T("Tahoma"));

	if (GameCell::focused || (light && GameCell::cell->linked(focus)))
		gr.fill_rect(Graphics::Rect::deflate(GameCell::r, Margin), Lighted);

	if (GameCell::cell->num != 0)
	{
		auto f = help >= Assistance::Current && GameCell::cell->equal(number) ? Graphics::Color::Red :
		         GameCell::cell->immutable                                    ? Graphics::Color::Black :
		                                                                        Graphics::Color::Navy;

		gr.draw_char(GameCell::r, GameCell::font, f, Graphics::Alignment::Center, _T("0123456789")[GameCell::cell->num]);
	}
	else
	if ((GameCell::focused || help > Assistance::Current) && GameCell::allowed(number, help))
	{
		auto f = help >= Assistance::Sure && GameCell::cell->sure(number) ? Graphics::Color::Green :
		         help <= Assistance::Current                              ? Graphics::Color::Gray :
		                                                                    Graphics::Color::Orange;

		gr.draw_char(GameCell::r, GameCell::tiny, f, Graphics::Alignment::Center, _T("0123456789")[number]);
	}
	else
	{
		auto sure = help == Assistance::Full ? GameCell::cell->sure(number) : 0;
		if (sure != 0)
			gr.draw_char(GameCell::r, GameCell::tiny, Graphics::Color::Green, Graphics::Alignment::Center, _T("0123456789")[sure]);
	}

	gr.draw_rect(GameCell::r, Graphics::Color::Black);
}

void GameCell::mouseMove( const int _x, const int _y )
{
	GameCell::focused = GameCell::r.contains(_x, _y);
}

void GameCell::mouseLeave()
{
	GameCell::focused = false;
}

Command GameCell::mouseLButton( const int, const int, const uint number, const Assistance help )
{
	if (GameCell::focused)
	{
		if (GameCell::allowed(number, help))
			return SetCellCmd;
		else
		if (help == Assistance::Full && number == 0 && GameCell::cell->sure() != 0)
			return SetSureCmd;
		else
		if (help != Assistance::None || GameCell::cell->num != number)
			return (Command)(Button0Cmd + GameCell::cell->num);
	}

	return NoCmd;
}

Command GameCell::mouseRButton( const int, const int )
{
	if (GameCell::focused && GameCell::cell->num != 0)
		return ClearCellCmd;

	return NoCmd;
}

Cell *GameCell::getCell()
{
	if (GameCell::focused)
		return GameCell::cell;

	return nullptr;
}

bool GameCell::allowed( const uint number, const Assistance help )
{
	if (number == 0 || GameCell::cell->num != 0)
		return false;

	if (help <= Assistance::Current)
		return true;

	if (help != Assistance::Full && GameCell::cell->passable(number))
		return true;

	if (GameCell::cell->allowed(number))
		return true;

	return false;
}

/*---------------------------------------------------------------------------*/

GameTable::GameTable( Sudoku &_s )
{
	for (auto &c: _s)
	{
		auto x = TAB.x + (FLOAT)((int)c.pos % 9) * (CellSize + Margin) + (FLOAT)((int)c.pos % 9 / 3) * Margin * 3;
		auto y = TAB.y + (FLOAT)((int)c.pos / 9) * (CellSize + Margin) + (FLOAT)((int)c.pos / 9 / 3) * Margin * 3;

		GameTable::emplace_back(x, y, &c);
	}
}

void GameTable::update( Graphics &gr, const uint number, const Assistance help, Cell* const focus, const bool light )
{
	for (auto &c: *this)
		c.update(gr, number, help, focus, light);
}

void GameTable::mouseMove( const int _x, const int _y )
{
	for (auto &c: *this)
		c.mouseMove(_x, _y);
}

void GameTable::mouseLeave()
{
	for (auto &c: *this)
		c.mouseLeave();
}

Command GameTable::mouseLButton( const int _x, const int _y, const uint number, const Assistance help )
{
	for (auto &c: *this)
	{
		Command cmd = c.mouseLButton(_x, _y, number, help);
		if (cmd != NoCmd)
			return cmd;
	}

	return NoCmd;
}

Command GameTable::mouseRButton( const int _x, const int _y )
{
	for (auto &c: *this)
	{
		Command cmd = c.mouseRButton(_x, _y);
		if (cmd != NoCmd)
			return cmd;
	}

	return NoCmd;
}

Cell *GameTable::getCell()
{
	for (auto &c: *this)
	{
		Cell *cell = c.getCell();
		if (cell != nullptr)
			return cell;
	}

	return nullptr;
}

/*---------------------------------------------------------------------------*/

void MenuItem::update( Graphics &gr, const int _x )
{
	static constexpr auto h = std::round(TabSize / 1.8f / MnuSize);

	if (MenuItem::font == nullptr)
		MenuItem::font = gr.font(h, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_CONDENSED, _T("Arial"));

	if (MenuItem::focused)
		gr.fill_rect(Graphics::Rect::deflate(MenuItem::r, GameTimer::until((int)Margin * 4)), Lighted);

	if (MenuItem::size() > 1)
	{
		auto cl = !MenuItem::focused ? Lighted : ((FLOAT)_x < MNU.center || MenuItem::size() == 2) ? Graphics::Color::Black : Background;
		auto cr = !MenuItem::focused ? Lighted : ((FLOAT)_x < MNU.center && MenuItem::size() != 2) ? Background : Graphics::Color::Black;

		gr.draw_char(MenuItem::r, MenuItem::font, cl, Graphics::Alignment::Left,  _T('◄'));
		gr.draw_char(MenuItem::r, MenuItem::font, cr, Graphics::Alignment::Right, _T('►'));
	}

	gr.draw_text(MenuItem::r, MenuItem::font, Graphics::Color::Black, Graphics::Alignment::Center, MenuItem::at(MenuItem::idx));
}

void MenuItem::mouseMove( const int _x, const int _y )
{
	MenuItem::focused = MenuItem::r.contains(_x, _y);
}

void MenuItem::mouseLeave()
{
	MenuItem::focused = false;
}

Command MenuItem::mouseLButton( const int _x, const int )
{
	if (MenuItem::focused)
	{
		GameTimer::restart();

		switch (MenuItem::num)
		{
		case  0: return (FLOAT)_x < MNU.center ? PrevLevelCmd : NextLevelCmd;
		case  1: return (FLOAT)_x < MNU.center ? PrevHelpCmd  : NextHelpCmd;
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

const TCHAR *MenuItem::getInfo()
{
	if (MenuItem::focused)
		return MenuItem::info;

	return nullptr;
}

template<typename T>
void MenuItem::setIndex( const T _i )
{
	MenuItem::idx = (uint)_i;
}

uint MenuItem::prev()
{
	const uint i = MenuItem::idx;
	const uint s = static_cast<uint>(MenuItem::size());

	if (MenuItem::num == 0)
		MenuItem::idx = i == 0 ? s - 1 : i == s - 1 ? 1 : 0;
	else
		MenuItem::idx = static_cast<uint>((i + s - 1) % s);

	return MenuItem::idx;
}

uint MenuItem::next()
{
	const uint i = MenuItem::idx;
	const uint s = static_cast<uint>(MenuItem::size());

	if (MenuItem::num == 0)
		MenuItem::idx = i == s - 1 ? 0 : i == 0 ? 1 : s - 1;
	else
		MenuItem::idx = static_cast<uint>((i + 1) % s);

	return MenuItem::idx;
}

/*---------------------------------------------------------------------------*/

GameMenu::GameMenu()
{
	static constexpr auto h = std::round(TabSize / 1.2f / MnuSize);
	auto pos = []( const FLOAT i ){ return MNU.y + std::round(i * (MNU.height - h) / (MnuSize - 1)); };

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

void GameMenu::update( Graphics &gr, const int _x )
{
	for (auto &m: *this)
		m.update(gr, _x);

	gr.draw_rect(MNU, Graphics::Color::Black);
}

void GameMenu::mouseMove( const int _x, const int _y )
{
	for (auto &m: *this)
		m.mouseMove(_x, _y);
}

void GameMenu::mouseLeave()
{
	for (auto &m: *this)
		m.mouseLeave();
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

void GameFooter::update( Graphics &gr, const TCHAR *info )
{
	if (GameFooter::font == nullptr)
		GameFooter::font = gr.font(std::min(FTR.height, WIN.width / 40), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_CONDENSED, _T("Arial"));

	if (info == nullptr)
		info = _T("Sudoku game, solver and generator");
	gr.draw_text(FTR, GameFooter::font, Graphics::Color::Gray, Graphics::Alignment::Center, info);
}

/*---------------------------------------------------------------------------*/

Game::Game(): hdr{}, tab{*this}, mnu{}, ftr{}, number{0}, tracking{false}, timer_f{true}, light_f{false}, help{Assistance::None}
{
	Sudoku::generate();

	Game::mnu[0].setIndex(Sudoku::level);
	Game::mnu[1].setIndex(Game::help);
	Game::mnu[2].setIndex(Game::light_f);
}

void Game::update( HWND hWnd )
{
	static const std::array<Graphics::Color, 5> colors =
	{
		Graphics::Color::Navy,
		Graphics::Color::Green,
		Graphics::Color::Olive,
		Graphics::Color::Maroon,
		Graphics::Color::Maroon,
	};

	auto time = Game::timer_f ? GameTimer::now() : -1;
	auto info = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? _T("unsolvable") : Sudoku::rating == -1 ? _T("ambiguous") : nullptr)
	                               : (Sudoku::corrupt() ? _T("corrupt") : _T("solved"));

	RECT rc;
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(hWnd, &cursor);
	GetClientRect(hWnd, &rc);
	cursor.x = rc.right > rc.left ? (int)std::round((FLOAT)cursor.x * WIN.width  / (FLOAT)(rc.right - rc.left)) : 0;
//	cursor.y = rc.bottom > rc.top ? (int)std::round((FLOAT)cursor.y * WIN.height / (FLOAT)(rc.bottom - rc.top)) : 0;

	Graphics::begin(Background);

	Graphics::fill_rect(Graphics::Rect(TAB.x + CellSize * 3 + Margin * 3, TAB.y, Margin * 2, TAB.height), Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x + CellSize * 6 + Margin * 9, TAB.y, Margin * 2, TAB.height), Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x, TAB.y + CellSize * 3 + Margin * 3, TAB.width, Margin * 2),  Graphics::Color::DimGray);
	Graphics::fill_rect(Graphics::Rect(TAB.x, TAB.y + CellSize * 6 + Margin * 9, TAB.width, Margin * 2),  Graphics::Color::DimGray);

	Graphics::fill_rect(HDR, colors[Sudoku::level]);

	Game::hdr.update(*this, info, time);
	Game::tab.update(*this, Game::number, Game::help, Game::tab.getCell(), Game::light_f);
	Game::mnu.update(*this, cursor.x);
	Game::ftr.update(*this, mnu.getInfo());

	Graphics::end();
}

void Game::mouseMove( const int _x, const int _y, HWND hWnd )
{
	Game::tab.mouseMove(_x, _y);
	Game::mnu.mouseMove(_x, _y);

	if (!Game::tracking)
	{
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
		TrackMouseEvent(&tme);
		Game::tracking = true;
	}
}

void Game::mouseLeave()
{
	Game::tab.mouseLeave();
	Game::mnu.mouseLeave();

	Game::tracking = false;
}

void Game::mouseLButton( const int _x, const int _y )
{
	Game::command(Game::hdr.mouseLButton(_x, _y));
	Game::command(Game::tab.mouseLButton(_x, _y, Game::number, Game::help));
	Game::command(Game::mnu.mouseLButton(_x, _y));
}

void Game::mouseRButton( const int _x, const int _y )
{
	Game::command(Game::tab.mouseRButton(_x, _y));
}

void Game::mouseWheel( const int _x, const int _y, const int _d, const HWND hWnd )
{
	auto cell = Game::tab.getCell();
	POINT cursor = { _x, _y };
	ScreenToClient(hWnd, &cursor);

	if ((Game::help != Assistance::None && TAB.contains(cursor)) || (cell != nullptr && cell->empty()))
		Game::command((Command)(Button0Cmd + (_d < 0 ? (Game::number == 0 ? 1 : 1 + (Game::number + 0) % 9)
		                                             : (Game::number == 0 ? 9 : 1 + (Game::number + 7) % 9))));
}

void Game::keyboard( const int _k )
{
	switch (_k)
	{
	case '0':       Game::command(Button0Cmd);   break;
	case '1':       Game::command(Button1Cmd);   break;
	case '2':       Game::command(Button2Cmd);   break;
	case '3':       Game::command(Button3Cmd);   break;
	case '4':       Game::command(Button4Cmd);   break;
	case '5':       Game::command(Button5Cmd);   break;
	case '6':       Game::command(Button6Cmd);   break;
	case '7':       Game::command(Button7Cmd);   break;
	case '8':       Game::command(Button8Cmd);   break;
	case '9':       Game::command(Button9Cmd);   break;
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

bool Game::set( uint num )
{
	if (num == 0)
		num = Game::number;

	return num == 0                       ? false :
	       Game::help == Assistance::None ? Sudoku::set(Game::tab.getCell(), num, Force::Direct) :
	       Game::help != Assistance::Full ? Sudoku::set(Game::tab.getCell(), num, Force::Careful) :
	                                        Sudoku::set(Game::tab.getCell(), num, Force::Safe);
}

void Game::command( const Command _c )
{
	int seconds;

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
	case Button9Cmd:    Game::number = Game::number == (uint)(_c - Button0Cmd) ? 0 : (uint)(_c - Button0Cmd);
	                    break;
	case ClearCellCmd:  Game::number = Game::tab.getCell()->num;
	                    Sudoku::set(Game::tab.getCell(), 0);
	                    break;
	case SetCellCmd:    Game::set();
	                    break;
	case SetSureCmd:    Game::set(Game::tab.getCell()->sure());
	                    break;
	case PrevHelpCmd:   Game::help = (Assistance)Game::mnu[1].prev();
	                    break;
	case NextHelpCmd:   Game::help = (Assistance)Game::mnu[1].next();
	                    break;
	case PrevLevelCmd:  Sudoku::level = (Difficulty)Game::mnu[0].prev();
	                    Sudoku::generate(); Game::number = 0; GameTimer::start();
	                   	Game::mnu[0].setIndex(Sudoku::level);
	                    break;
	case NextLevelCmd:  Sudoku::level = (Difficulty)Game::mnu[0].next();
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
	case SaveCmd:       seconds = Game::timer_f ? GameTimer::now() : 0;
	                    Sudoku::save({}, seconds);
	                    break;
	case LoadCmd:       if (Sudoku::load({}, &seconds))
	                    {
	                    	Game::number = 0;
	                    	GameTimer::continue_if(Sudoku::rating >= 0, seconds);
	                    	Game::mnu[0].setIndex(Sudoku::level);
	                    }
	                    break;
	case QuitCmd:       Graphics::quit();
	                    break;
	}

	if (Sudoku::len() == 81)
	{
		Game::number = 0;
		if (Sudoku::solved())
			GameTimer::stop();
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
	const int x = w > 0 ? (int)std::round((FLOAT)GET_X_LPARAM(lParam) * WIN.width  / (FLOAT)w) : 0;
	const int y = h > 0 ? (int)std::round((FLOAT)GET_Y_LPARAM(lParam) * WIN.height / (FLOAT)h) : 0;

	switch (msg)
	{
		case WM_PAINT:       game.update(hWnd);              break;
		case WM_ERASEBKGND:  return TRUE;
		case WM_MOUSEMOVE:   game.mouseMove(x, y, hWnd);     break;
		case WM_MOUSELEAVE:  game.mouseLeave();              break;
		case WM_LBUTTONDOWN: game.mouseLButton(x, y);        break;
		case WM_RBUTTONDOWN: game.mouseRButton(x, y);        break;
		case WM_MOUSEWHEEL:  game.mouseWheel(x, y, d, hWnd); break;
		case WM_KEYDOWN:     game.keyboard(k);               break;
		case WM_DESTROY:     PostQuitMessage(0);             break;
		default:             return DefWindowProc(hWnd, msg, wParam, lParam);
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

		game.update(hWnd);
	}

	return 0;
}
