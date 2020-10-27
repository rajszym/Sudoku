/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    27.10.2020
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
#include "directx.hpp"

const char *title = "Sudoku";

constexpr int CellSize  { 64 };
constexpr int BigMargin { CellSize / 4 };
constexpr int LowMargin { CellSize / 16 };
constexpr int TabSize   { CellSize * 9 + LowMargin * 6 + BigMargin * 2 };
constexpr int BarWidth  { CellSize };
constexpr int MnuHeight { (TabSize - LowMargin * 10) / 11 };
constexpr int MnuWidth  { MnuHeight * 4 };
constexpr int HdrWidth  { TabSize + BarWidth + MnuWidth + BigMargin * 4 };
constexpr int FtrHeight { CellSize / 2 };

const DirectX::Rectangle HDR(LowMargin, LowMargin, HdrWidth, CellSize);
const DirectX::Rectangle TAB(HDR.left, HDR.bottom + BigMargin, TabSize, TabSize);
const DirectX::Rectangle BTN(TAB.right + BigMargin * 2, TAB.top, BarWidth, TabSize);
const DirectX::Rectangle MNU(BTN.right + BigMargin * 2, TAB.top, MnuWidth, TabSize);
const DirectX::Rectangle FTR(HDR.left, TAB.bottom + BigMargin, HDR.width, FtrHeight);
const DirectX::Rectangle WIN(0, 0, HDR.width + LowMargin * 2, HDR.height + TAB.height + FTR.height + BigMargin * 2 + LowMargin * 2);

constexpr D3DCOLOR Background = DirectX::Moccasin;
constexpr D3DCOLOR Lighted    = DirectX::OldLace;
constexpr D3DCOLOR Arrow      = DirectX::Peru;

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

	static LPD3DXFONT font;

	GameHeader() {}

	void update( DirectX &, const char *, int );
};

/*---------------------------------------------------------------------------*/

class GameCell
{
	using Cell = SudokuCell;

	const int x;
	const int y;
	const DirectX::Rectangle r;

	Cell &cell;

public:

	static GameCell *focus;

	GameCell( const int _x, const int _y, Cell &_c ): x{_x}, y{_y}, r{x, y, CellSize, CellSize}, cell{_c} {}

	Cell  & get         ()        { return cell; }
	bool    allowed     ( int n ) { return GameCell::cell.allowed(n); }
	int     sure        ( int n ) { return GameCell::cell.sure(n); }

	void    update      ( DirectX & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameTable: public std::vector<GameCell>
{
public:

	static LPD3DXFONT font;

	GameTable( Sudoku & );

	void    update      ( DirectX & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class Button
{
	const int y;
	const DirectX::Rectangle r;
	const int num;

public:

	static Button *focus;
	static int     cur;

	Button( const int _y, const int _n ): y{_y}, r{BTN.x, y, BTN.width, CellSize}, num{_n} {}

	void    update      ( DirectX &, int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};


class GameButtons: public std::vector<Button>
{
public:

	static LPD3DXFONT font;

	GameButtons();

	void    update      ( DirectX &, int );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
	void    mouseRButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class MenuItem: public std::vector<const char *>
{
	const int  y;
	const int  idx;
	const DirectX::Rectangle r;

public:

	const char *info;

	static MenuItem *focus;
	static bool      back;

	MenuItem( const int x, const int _y, const char *_i ): y{_y}, idx{x}, r{MNU.x, y, MnuWidth, MnuHeight}, info{_i} {}

	void    next        ( bool );
	void    update      ( DirectX & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameMenu: public std::vector<MenuItem>
{
public:

	static LPD3DXFONT font;

	GameMenu();

	void    update      ( DirectX & );
	void    mouseMove   ( const int, const int );
	Command mouseLButton( const int, const int );
};

/*---------------------------------------------------------------------------*/

class GameFooter
{
public:

	static LPD3DXFONT font;

	GameFooter() {}

	void update( DirectX & );
};

/*---------------------------------------------------------------------------*/

class Game: public DirectX, public Sudoku
{
	GameHeader  hdr;
	GameTable   tab;
	GameButtons btn;
	GameMenu    mnu;
	GameFooter  ftr;
	
	bool solved;

public:

	static Assistance help;
	static Difficulty level;

	Game(): DirectX(), Sudoku{}, hdr{}, tab{*this}, btn{}, mnu{}, ftr{}, solved{false} { Sudoku::generate(); }

	void update      ();
	void mouseMove   ( const int, const int );
	void mouseLButton( const int, const int );
	void mouseRButton( const int, const int );
	void mouseWheel  ( const int, const int, const int );
	void keyboard    ( const int );
	void command     ( Command );
};

/*---------------------------------------------------------------------------*/
/*                              INITIALIZATION                               */
/*---------------------------------------------------------------------------*/

LPD3DXFONT  GameHeader ::font  = NULL;
LPD3DXFONT  GameTable  ::font  = NULL;
LPD3DXFONT  GameButtons::font  = NULL;
LPD3DXFONT  GameMenu   ::font  = NULL;
LPD3DXFONT  GameFooter ::font  = NULL;

GameCell   *GameCell   ::focus = nullptr;

Button     *Button     ::focus = nullptr;
int         Button     ::cur   = 0;

MenuItem   *MenuItem   ::focus = nullptr;
bool        MenuItem   ::back  = false;

Assistance  Game       ::help  = Assistance::None;
Difficulty  Game       ::level = Difficulty::Easy;

/*---------------------------------------------------------------------------*/
/*                                  SUDOKU                                   */
/*---------------------------------------------------------------------------*/

auto sudoku = Game();

/*---------------------------------------------------------------------------*/
/*                              IMPLEMENTATION                               */
/*---------------------------------------------------------------------------*/

void GameHeader::update( DirectX &dx, const char *info, int time )
{
	if (GameHeader::font == NULL)
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = HDR.height - LowMargin * 2;
		desc.Width           = 0;
		desc.Weight          = FW_NORMAL;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_OUTLINE_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = VARIABLE_PITCH;
		strcpy(desc.FaceName, "Tahoma");

		GameHeader::font = dx.font(&desc);
	}

	static const DirectX::Color banner_color[] =
	{
		DirectX::Blue,
		DirectX::Green,
		DirectX::Orange,
		DirectX::Crimson,
		DirectX::Crimson,
	};

	auto f = banner_color[Game::level];
	dx.rect(HDR, DirectX::Black);
	dx.text(HDR, LowMargin, GameHeader::font, f, DT_LEFT, ::title);

	if (info != nullptr)
		dx.text(HDR, GameHeader::font, DirectX::Red, DT_CENTER, info);

	char v[16];
	snprintf(v, sizeof(v), "%6d:%02d:%02d", time / 3600, (time / 60) % 60, time % 60);
	dx.text(HDR, LowMargin, GameHeader::font, f, DT_RIGHT, v);
}

/*---------------------------------------------------------------------------*/

void GameCell::update( DirectX &dx )
{
	auto h = Game::help;
	auto n = Button::cur;
	auto f = cell.empty() ? DirectX::Grey : cell.immutable ? DirectX::Black : DirectX::Green;

	if (n != 0)
	{
		if      (h >= Assistance::Current && GameCell::cell.equal(n))   f = DirectX::Red;
		else if (h >= Assistance::Full    && GameCell::cell.sure(n))    f = DirectX::Green;
		else if (h >= Assistance::Full    && GameCell::cell.allowed(n)) f = DirectX::Orange;
	}

	if (GameCell::focus == this)
		dx.fill(GameCell::r, LowMargin, Lighted);
	dx.rect(GameCell::r, DirectX::Black);

	dx.put(GameCell::r, GameTable::font, f, DT_CENTER, "-123456789"[cell.num]);
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
			if (Button::cur == 0 && Game::help >= Assistance::Full)
				return static_cast<Command>(GameCell::cell.sure());
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
		int x = TAB.x + (c.pos % 9) * (CellSize + LowMargin) + (c.pos % 9 / 3) * (BigMargin - LowMargin);
		int y = TAB.y + (c.pos / 9) * (CellSize + LowMargin) + (c.pos / 9 / 3) * (BigMargin - LowMargin);;

		GameTable::emplace_back(x, y, c);
	}
}

void GameTable::update( DirectX &dx )
{
	if (GameTable::font == NULL)
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = CellSize;
		desc.Width           = 0;
		desc.Weight          = FW_BOLD;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_OUTLINE_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = VARIABLE_PITCH;
		strcpy(desc.FaceName, "Tahoma");

		GameTable::font = dx.font(&desc);
	}

	for (auto &c: *this)
		c.update(dx);

	dx.rect(TAB.x + CellSize * 3 + LowMargin * 2 + BigMargin * 1 / 2 - 1, TAB.y, 2, TAB.height, DirectX::Black);
	dx.rect(TAB.x + CellSize * 6 + LowMargin * 4 + BigMargin * 3 / 2 - 1, TAB.y, 2, TAB.height, DirectX::Black);
	dx.rect(TAB.x, TAB.y + CellSize * 3 + LowMargin * 2 + BigMargin * 1 / 2 - 1, TAB.width, 2,  DirectX::Black);
	dx.rect(TAB.x, TAB.y + CellSize * 6 + LowMargin * 4 + BigMargin * 3 / 2 - 1, TAB.width, 2,  DirectX::Black);
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

void Button::update( DirectX &dx, int count )
{
	auto h = Game::help;
	auto f = DirectX::DimGrey;

	if (Button::focus == this)
		dx.fill(Button::r, LowMargin, Lighted);
	else
	if (Button::cur == Button::num)
		dx.fill(Button::r, DirectX::White);

	dx.rect(Button::r, DirectX::Black);
	if (Button::cur == Button::num)
	{
		dx.rect(Button::r, 1, DirectX::DimGrey);
		dx.rect(Button::r, 2, DirectX::Grey);
		dx.rect(Button::r, 2, DirectX::LightGrey);
	}

	if (GameCell::focus != nullptr)
	{
		if      (h >= Assistance::Sure      && GameCell::focus->sure(Button::num))    f = DirectX::Green;
		else if (h >= Assistance::Available && GameCell::focus->allowed(Button::num)) f = DirectX::Orange;
	}

	dx.put(Button::r, GameButtons::font, f, DT_CENTER, '0' + Button::num);

	if (Button::cur == Button::num && Game::help > Assistance::None)
	{
		RECT rc { BTN.right, Button::r.bottom - BigMargin * 2, MNU.left, Button::r.bottom };
		dx.put(rc, GameMenu::font, DirectX::Grey, DT_CENTER, count > 9 ? '?' : '0' + count);
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
		return static_cast<Command>(Button::num);

	return NoCmd;
}

/*---------------------------------------------------------------------------*/

GameButtons::GameButtons()
{
	for (int n = 1; n <= 9; n++)
	{
		int y = BTN.y + (n - 1) * (CellSize + LowMargin) + ((n - 1) / 3) * (BigMargin - LowMargin);

		GameButtons::emplace_back(y, n);
	}
}

void GameButtons::update( DirectX &dx, int count )
{
	if (GameButtons::font == NULL)
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = CellSize;
		desc.Width           = 0;
		desc.Weight          = FW_BOLD;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_OUTLINE_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = VARIABLE_PITCH;
		strcpy(desc.FaceName, "Tahoma");

		GameButtons::font = dx.font(&desc);
	}

	for (auto &b: *this)
		b.update(dx, count);
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

void MenuItem::next( bool prev )
{
	const int max = MenuItem::size() - 1;

	if (max > 0)
	{
		if (MenuItem::idx == 0)
		{
			int i = static_cast<int>(Game::level);
			if (prev) i = i == 0 ? max : i == max ? 1 : 0;
			else      i = i == max ? 0 : i == 0 ? 1 : max;
			Game::level = static_cast<Difficulty>(i);
		}
		else
		if (MenuItem::idx == 1)
		{
			int i = static_cast<int>(Game::help);
			if (prev) i = (i + max) % (max + 1);
			else      i = (i + 1)   % (max + 1);
			Game::help = static_cast<Assistance>(i);
		}
	}
}

void MenuItem::update( DirectX &dx )
{
	int i = MenuItem::idx == 0 ? static_cast<int>(Game::level) : MenuItem::idx == 1 ? static_cast<int>(Game::help) : 0;

	if (MenuItem::focus == this)
	{
		dx.fill(MenuItem::r, Lighted);
		if (MenuItem::size() > 1)
		{
			dx.left (MenuItem::r, MenuItem::r.height / 8, MenuItem::back ? Arrow : Background);
			dx.right(MenuItem::r, MenuItem::r.height / 8, MenuItem::back ? Background : Arrow);
		}
	}

	dx.text(MenuItem::r, GameMenu::font, DirectX::Black, DT_CENTER, MenuItem::at(i));
}

void MenuItem::mouseMove( const int _x, const int _y )
{
	if (MenuItem::r.contains(_x, _y))
	{
		MenuItem::focus = this;
		MenuItem::back = _x < MNU.Center(0);
	}
}

Command MenuItem::mouseLButton( const int _x, const int _y )
{
	if (MenuItem::r.contains(_x, _y))
	{
		switch (MenuItem::idx)
		{
		case  0: return MenuItem::back ? PrevLevelCmd : NextLevelCmd;
		case  1: return MenuItem::back ? PrevHelpCmd  : NextHelpCmd;
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

/*---------------------------------------------------------------------------*/

GameMenu::GameMenu()
{
	GameMenu::emplace_back( 0, MNU.y + (TabSize - MnuHeight) *  0/10, "Change difficulty level of the game (keyboard shortcuts: D, PgUp, PgDn)");
		GameMenu::back().emplace_back("easy");
		GameMenu::back().emplace_back("medium");
		GameMenu::back().emplace_back("hard");
		GameMenu::back().emplace_back("expert");
		GameMenu::back().emplace_back("extreme");
	GameMenu::emplace_back( 1, MNU.y + (TabSize - MnuHeight) *  1/10, "Change the assistance level of the game (keyboard shortcuts: A, Left, Right)");
		GameMenu::back().emplace_back("none");
		GameMenu::back().emplace_back("current");
		GameMenu::back().emplace_back("available");
		GameMenu::back().emplace_back("sure");
		GameMenu::back().emplace_back("full");
	GameMenu::emplace_back( 2, MNU.y + (TabSize - MnuHeight) *  2/10, "Generate or load a new layout (keyboard shortcuts: N, Tab)");
		GameMenu::back().emplace_back("new");
	GameMenu::emplace_back( 3, MNU.y + (TabSize - MnuHeight) *  3/10, "Solve the current layout (keyboard shortcuts: S, Enter)");
		GameMenu::back().emplace_back("solve");
	GameMenu::emplace_back( 4, MNU.y + (TabSize - MnuHeight) *  4/10, "Undo last move or restore the accepted layout (keyboard shortcuts: U, Backspace)");
		GameMenu::back().emplace_back("undo");
	GameMenu::emplace_back( 5, MNU.y + (TabSize - MnuHeight) *  5/10, "Clear the board (keyboard shortcuts: C, Delete)");
		GameMenu::back().emplace_back("clear");
	GameMenu::emplace_back( 6, MNU.y + (TabSize - MnuHeight) *  6/10, "Start editing the current layout (keyboard shortcuts: E, Home)");
		GameMenu::back().emplace_back("edit");
	GameMenu::emplace_back( 7, MNU.y + (TabSize - MnuHeight) *  7/10, "Accept the current layout and finish editing (keyboard shortcuts: T, End)");
		GameMenu::back().emplace_back("accept");
	GameMenu::emplace_back( 8, MNU.y + (TabSize - MnuHeight) *  8/10, "Save the current layout to the file (keyboard shortcuts: V, Insert)");
		GameMenu::back().emplace_back("save");
	GameMenu::emplace_back( 9, MNU.y + (TabSize - MnuHeight) *  9/10, "Load layout from the file (keyboard shortcut: L)");
		GameMenu::back().emplace_back("load");
	GameMenu::emplace_back(10, MNU.y + (TabSize - MnuHeight) * 10/10, "Quit the game (keyboard shortcuts: Q, Esc)");
		GameMenu::back().emplace_back("quit");
}

void GameMenu::update( DirectX &dx )
{
	if (GameMenu::font == NULL)
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = MnuHeight - LowMargin * 2;
		desc.Width           = 0;
		desc.Weight          = FW_NORMAL;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_OUTLINE_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = VARIABLE_PITCH;
		strcpy(desc.FaceName, "Tahoma");

		GameMenu::font = dx.font(&desc);
	}

	for (auto &m: *this)
		m.update(dx);

	dx.rect(MNU, DirectX::Black);
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

void GameFooter::update( DirectX &dx )
{
	if (GameFooter::font == NULL)
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = FTR.height - LowMargin * 2;
		desc.Width           = 0;
		desc.Weight          = FW_NORMAL;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_OUTLINE_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = VARIABLE_PITCH;
		strcpy(desc.FaceName, "Tahoma");

		GameFooter::font = dx.font(&desc);
	}

	dx.rect(FTR, DirectX::Grey);

	if (MenuItem::focus != nullptr)
		dx.text(FTR, GameFooter::font, DirectX::Grey, DT_CENTER, MenuItem::focus->info);
}

/*---------------------------------------------------------------------------*/

void Game::update()
{
	Game::solved = Sudoku::solved();
	Game::level  = Sudoku::level;

	if (Game::solved)
	{
		Button::cur = 0;
		Sudoku::Timer::stop();
	}

	auto time  = Sudoku::Timer::get();
	auto count = Sudoku::count(Button::cur);
	auto info  = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? "UNSOLVABLE" : Sudoku::rating == -1 ? "AMBIGUOUS" : nullptr)
	                                : (Sudoku::corrupt() ? "CORRUPT" : "SOLVED");

	DirectX::begin(Background);

	hdr.update(*this, info, time);
	tab.update(*this);
	btn.update(*this, count);
	mnu.update(*this);
	ftr.update(*this);

	DirectX::end();
}

void Game::mouseMove( const int _x, const int _y )
{
	tab.mouseMove(_x, _y);
	btn.mouseMove(_x, _y);
	mnu.mouseMove(_x, _y);
}

void Game::mouseLButton( const int _x, const int _y )
{
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
	Game::command(static_cast<Command>(_d < 0 ? (Button::cur == 0 ? 1 : 1 + (Button::cur + 0) % 9)
	                                          : (Button::cur == 0 ? 9 : 1 + (Button::cur + 7) % 9)));
}

void Game::keyboard( const int _k )
{
	bool prev = false;

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
	case VK_LEFT:   prev = true; /* falls through */
	case VK_RIGHT:               /* falls through */
	case 'A':       Game::command(prev ? PrevHelpCmd : NextHelpCmd); break;
	case VK_NEXT:   prev = true; /* falls through */ // PAGE DOWN
	case VK_PRIOR:               /* falls through */ // PAGE UP
	case 'D':       Game::command(prev ? PrevLevelCmd : NextLevelCmd); /* falls through */
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
	bool prev = false;

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
	case Button9Cmd:    if (!Game::solved) Button::cur = static_cast<int>(_c);
	                    break;
	case ClearCellCmd:  Button::cur = GameCell::focus->get().num;
	                    Sudoku::set(GameCell::focus->get(), 0);
	                    break;
	case SetCellCmd:    Sudoku::set(GameCell::focus->get(), Button::cur, Game::help <= Assistance::Current);
	                    break;
	case PrevHelpCmd:   prev = true; /* falls through */
	case NextHelpCmd:   Game::mnu[1].next(prev);
	                    break;
	case PrevLevelCmd:  prev = true; /* falls through */
	case NextLevelCmd:  Game::mnu[0].next(prev); Sudoku::level = Game::level; /* falls through */
	case GenerateCmd:   Sudoku::generate(); Button::cur = 0; Sudoku::Timer::start();
	                    break;
	case SolveCmd:      Sudoku::solve();    Button::cur = 0;
	                    if (!Sudoku::solved()) Sudoku::rating = -2;
	                    break;
	case UndoCmd:       Sudoku::undo();
	                    break;
	case ClearCmd:      Sudoku::clear();    Button::cur = 0; Sudoku::Timer::reset();
	                    break;
	case EditCmd:       Sudoku::discard();  Sudoku::Timer::reset();
	                    break;
	case AcceptCmd:     Sudoku::accept();
	                    break;
	case SaveCmd:       Sudoku::save();
	                    break;
	case LoadCmd:       if (Sudoku::load()) Button::cur = 0, Sudoku::Timer::start();
	                    break;
	case QuitCmd:       DirectX::quit();
	                    break;
	}
}

//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int k = GET_KEYSTATE_WPARAM(wParam);
	int d = GET_WHEEL_DELTA_WPARAM(wParam);
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	if (w > 0) x = (x * WIN.width  + w / 2) / w;
	if (h > 0) y = (y * WIN.height + h / 2) / h;

	switch (msg)
	{
		case WM_CREATE:      if (!sudoku.DirectX::init(hWnd)) DestroyWindow(hWnd); break;
		case WM_MOUSEMOVE:   sudoku.Game::mouseMove(x, y);                         break;
		case WM_MOUSEWHEEL:  sudoku.Game::mouseWheel(x, y, d);                     break;
		case WM_LBUTTONDOWN: sudoku.Game::mouseLButton(x, y);                      break;
		case WM_RBUTTONDOWN: sudoku.Game::mouseRButton(x, y);                      break;
		case WM_KEYDOWN:     sudoku.Game::keyboard(k);                             break;
		case WM_DESTROY:     PostQuitMessage(0);                                   break;
		default:      return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return FALSE;
}

//----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
	WNDCLASSEX wc = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
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

		sudoku.Game::update();
	}

	return 0;
}
