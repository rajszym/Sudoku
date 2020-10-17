/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    17.10.2020
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

#include "sudoku.hpp"
#include "console.hpp"
#include <cstring>
#include <windows.h>

const char *title = "Sudoku";

const Console::Rectangle WIN(0, 0, 42, 15);
const Console::Rectangle BNR(WIN.left, WIN.top, WIN.width, 1);
const Console::Rectangle TAB(BNR.left, BNR.bottom + 1, 25, 13);
const Console::Rectangle BTN(TAB.right + 1, TAB.top,  3, TAB.height);
const Console::Rectangle MNU(BTN.right + 1, TAB.top, 14, TAB.height);
const Console::Rectangle NFO(WIN.left, WIN.bottom, WIN.width, 1);

std::optional<Console> con;

enum Assistance
{
	None = 0,
	Current,
	Available,
	Sure,
	Full,
};

struct Button
{
	static int     button;
	static Button *focus;

	int num;
	int pos;

	Button( const int n ): num{n}, pos{n+(n-1)/3} {}

	void draw();
	void update(Assistance, Cell *);
};

int     Button::button{0};
Button *Button::focus {nullptr};

void Button::draw()
{
	::con->Put(BTN.x + 1, BTN.y + Button::pos, '0' + Button::num);
}

void Button::update(Assistance h, Cell *c)
{
	auto fore = Button::button == Button::num ? Console::Black : Button::focus == this ? Console::White : Console::LightGrey;
	auto back = Button::button == Button::num ? Console::White : Button::focus == this ? Console::Grey  : Console::Black;

	if (c != nullptr)
	{
		if      (h >= Assistance::Sure      && c->sure(Button::num))    (Button::button == Button::num ? back : fore) = Console::LightGreen;
		else if (h >= Assistance::Available && c->allowed(Button::num)) (Button::button == Button::num ? back : fore) = Console::Yellow;
	}

	::con->Put(BTN.x + 1, BTN.y + Button::pos, fore, back);
}

struct Menu: std::vector<const char *>
{
	static Menu *focus;
	static bool  back;

	const  char *key;
	const  char *info;

	int pos;
	int idx;

	Menu( const int p, const char *k, const char *i, const std::vector<const char *> v, const int x = 0 ): std::vector<const char *>{v}, key{k}, info{i}, pos{p}, idx{x} {}

	int  next( bool );
	void draw( int = 0 );
	void update();
};

Menu *Menu::focus{nullptr};
bool  Menu::back {false};

int Menu::next( bool prev )
{
	const int max = Menu::size() - 1;

	if (Menu::pos == 1)
	{
		if (prev) Menu::idx = Menu::idx == 0 ? max : Menu::idx == max ? 1 : 0;
		else      Menu::idx = Menu::idx == max ? 0 : Menu::idx == 0 ? 1 : max;
	}
	else
	if (max > 0)
	{
		if (prev) Menu::idx = (Menu::idx + max) % (max + 1);
		else      Menu::idx = (Menu::idx + 1)   % (max + 1);
	}

	return Menu::idx;
}

void Menu::draw( int i )
{
	if (Menu::size() > static_cast<size_t>(i))
	{
		unsigned n = std::strlen(Menu::data()[Menu::idx = i]);
		::con->Put (MNU.x + 4,     MNU.y + Menu::pos, Menu::data()[Menu::idx]);
		::con->Fill(MNU.x + 4 + n, MNU.y + Menu::pos, MNU.width - 5 - n, 1);
	}
}

void Menu::update()
{
	if (Menu::focus == this)
	{
		if (Menu::size() > 1)
			::con->Put (MNU.x + 1, MNU.y + Menu::pos, Menu::back ? "<<" : ">>");
		::con->Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::White, Console::Grey);
	}
	else
	{
		::con->Put (MNU.x + 1, MNU.y + Menu::pos, Menu::key);
		::con->Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::LightGrey);
	}
}

struct Game: public Sudoku
{
	static Cell *focus;
	const  char *title;

	Assistance   help;

	std::vector<Button> btn;
	std::vector<Menu>   mnu;

	Game( const char *, Difficulty = Difficulty::Easy );
	~Game();

	void draw_cell    ( Cell & );
	void update_cell  ( Cell & );
	void update_info  ();
	void update_menu  ();
	void draw         ();
	void update       ();
	void game         ();
};

Cell *Game::focus = nullptr;

Game::Game( const char *t, Difficulty l ): Sudoku{l}, title{t}, help{Assistance::None}
{
	::con->SetFont(56, L"Consolas");
	::con->Center(WIN.width, WIN.height);
	::con->HideCursor();
	::con->Clear();

	Game::btn = { { 1 }, { 2 }, { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 } };

	Game::mnu = { {  1, "d:", "change difficulty level of the game",      { "easy", "medium", "hard", "expert", "extreme"  }, Sudoku::level },
	              {  2, "a:", "change assistance level of the game",      { "none", "current", "available", "sure", "full" }, Game::help },
	              {  3, "n:", "generate or load a new layout",            { "new"    } },
	              {  4, "s:", "solve the current layout",                 { "solve"  } },
	              {  5, "u:", "undo last move / restore accepted layout", { "undo"   } },
	              {  6, "c:", "clear the board",                          { "clear"  } },
	              {  7, "e:", "start editing the current layout",         { "edit"   } },
	              {  8, "t:", "accept the layout and finish editing",     { "accept" } },
	              {  9, "v:", "save the current layout to the file",      { "save"   } },
	              { 10, "l:", "load layout from the file",                { "load"   } },
	              { 11, "q:", "quit the game",                            { "quit"   } },
	            };
}

Game::~Game()
{
	::con->Clear();
}

void Game::draw_cell( Cell &cell )
{
	int x = TAB.x + 2 + (cell.pos % 9 + cell.pos % 9 / 3) * 2;
	int y = TAB.y + 1 + (cell.pos / 9 + cell.pos / 9 / 3);

	::con->Put(x, y, "-123456789"[cell.num]);
}

void Game::update_cell( Cell &cell )
{
	const int n = Button::button; // n = Button::button == 0 && Button::focus != nullptr ? Button::focus->num : Button::button;
	const Assistance h = Game::help;

	const int x = TAB.x + 2 + (cell.pos % 9 + cell.pos % 9 / 3) * 2;
	const int y = TAB.y + 1 + (cell.pos / 9 + cell.pos / 9 / 3);

	auto fore = cell.empty() ? Console::LightGrey : cell.immutable ? Console::White : Console::LightGreen;
	auto back = Game::focus != &cell ? Console::Black : Console::Grey;

	if (n != 0)
	{
		if      (h >= Assistance::Current && cell.equal(n))   back = Game::focus != &cell ? Console::Red    : Console::LightRed;
		else if (h >= Assistance::Full    && cell.sure(n))    back = Game::focus != &cell ? Console::Green  : Console::LightGreen;
		else if (h >= Assistance::Full    && cell.allowed(n)) back = Game::focus != &cell ? Console::Orange : Console::Yellow;
	}

	::con->Put(x, y, fore, back);
}

void Game::update_info()
{
	char txt[16];
	const char *nfo;
	const int n = Button::button; // n = Button::button == 0 && Button::focus != nullptr ? Button::focus->num : Button::button;
	size_t cnt = n == 0 ? 0 : static_cast<size_t>(Sudoku::count(n));
	::con->Put(BTN.x + 1, BNR.y, n == 0 || Game::help == Assistance::None ? ' ' : cnt > 9 ? '?' : '0' + cnt);

	nfo = Sudoku::len() < 81 ? (Sudoku::rating == -2 ? "unsolvable" : Sudoku::rating == -1 ? "ambiguous" : "")
	                         : (Sudoku::corrupt() ? "corrupt" : "solved");
	cnt = std::strlen(nfo);
	::con->Fill(TAB.x + 9, BNR.y, TAB.width - 10 - cnt, 1); ::con->Put(TAB.Right(cnt + 1), BNR.y, nfo);

	cnt = static_cast<size_t>(Sudoku::Timer::get());
	std::snprintf(txt, sizeof(txt), "%zu:%02zu:%02zu", cnt / 3600, (cnt / 60) % 60, cnt % 60);
	cnt = std::strlen(txt);
	::con->Fill(MNU.x + 1, BNR.y, MNU.width - 2 - cnt, 1); ::con->Put(MNU.Right(cnt + 1), BNR.y, txt);

	nfo = (Menu::focus == nullptr) ? "" : Menu::focus->info;
	cnt = std::strlen(nfo);
	::con->Put(NFO.x + 1, NFO.y, nfo); ::con->Fill(NFO.x + 1 + cnt, NFO.y, NFO.width - 2 - cnt, 1);
}

void Game::update_menu()
{
	Game::mnu[0].draw(static_cast<int>(Sudoku::level));
	Game::mnu[1].draw(static_cast<int>(Game::help));
}

void Game::draw()
{
	for (Cell &c: *this)
		Game::draw_cell(c);
}

void Game::update()
{
	static const Console::Color banner_color[] =
	{
		Console::Blue,
		Console::Green,
		Console::Orange,
		Console::Red,
		Console::Red
	};

	::con->Fill(BNR, Console::White, banner_color[Sudoku::level]);
	Game::update_info();

	for (Cell &c: *this)
		Game::update_cell(c);

	for (Button &b: Game::btn)
		b.update(Game::help, Game::focus);

	for (Menu &m: Game::mnu)
		m.update();
}

void Game::game()
{
	::con->Fill(BNR, Console::White);
	::con->Put(BNR.x + 1, BNR.y, Game::title);
	::con->Fill(NFO, Console::Black, Console::Grey);

	::con->DrawSingle(TAB);
	::con->DrawSingle(TAB.x, TAB.y + (TAB.height - 1) / 3, TAB.width, (TAB.height - 1) / 3 + 1);
	::con->DrawSingle(TAB.x + (TAB.width - 1) / 3, TAB.y, (TAB.width - 1) / 3 + 1, TAB.height);
	::con->DrawSingle(BTN);
	::con->DrawSingle(MNU);

	for (Button b: Game::btn)
		b.draw();

	for (Menu m: Game::mnu)
		m.draw(m.idx);

	Sudoku::generate();
	Game::draw();
	Game::update();

	for (;;)
	{
		int x, y, p;
		INPUT_RECORD input;

		while (::con->GetInput(&input))
		{
			switch (input.EventType)
			{
			case MOUSE_EVENT:

				x = (int) input.Event.MouseEvent.dwMousePosition.X;
				y = (int) input.Event.MouseEvent.dwMousePosition.Y;

				switch(input.Event.MouseEvent.dwEventFlags)
				{

				case 0:
				case DOUBLE_CLICK:

					if (x > TAB.left && x < TAB.right && y > TAB.top && y < TAB.bottom)
					{
						switch (input.Event.MouseEvent.dwButtonState)
						{
						case RIGHTMOST_BUTTON_PRESSED:
							Button::button = 0;
							break;
						}

						x -= TAB.x; y -= TAB.y;
						if (x % 2 != 0 || x % 8 == 0 || y % 4 == 0)
							break;

						x = (x / 2) - (x / 8) - 1;
						y = y - (y / 4) - 1;
						p = y * 9 + x;
						Cell &c = Sudoku::data()[p];

						switch (input.Event.MouseEvent.dwButtonState)
						{
						case FROM_LEFT_1ST_BUTTON_PRESSED:
							if (c.num == 0)
							{
								if (Button::button == 0 && Game::help >= Assistance::Full)
									Button::button = c.sure();
								else
								if (Sudoku::set(c, Button::button, Game::help <= Assistance::Current))
									Game::draw_cell(c);
							}
							else if (!Sudoku::solved() || !c.immutable)
							{
								Button::button = c.num;
								if (Sudoku::set(c, 0))
									Game::draw_cell(c);
							}
							break;
						}
					}
					else
					if (x >= BTN.left && x <= BTN.right && y > BTN.top && y < BTN.bottom && !Sudoku::solved())
					{
						x -= BTN.x; y -= BTN.y;

						switch (input.Event.MouseEvent.dwButtonState)
						{
						case FROM_LEFT_1ST_BUTTON_PRESSED:
							Button::button = (y % 4 == 0) ? 0 : y - y / 4;
							break;
						case RIGHTMOST_BUTTON_PRESSED:
							Button::button = 0;
							break;
						}
					}
					else
					if (x > MNU.left && x < MNU.right && y > MNU.top && y < MNU.bottom)
					{
						x -= MNU.x; y -= MNU.y;

						if (input.Event.MouseEvent.dwButtonState != FROM_LEFT_1ST_BUTTON_PRESSED)
							break;

						switch (y)
						{
						case  2: Game::help =        static_cast<Assistance>(Game::mnu[1].next(Menu::back)); break;
						case  1: Sudoku::level =     static_cast<Difficulty>(Game::mnu[0].next(Menu::back)); /* falls through */
						case  3: Sudoku::generate(); Game::draw(); Button::button = 0; Sudoku::Timer::start(); break;
						case  4: Sudoku::solve();    Game::draw(); Button::button = 0; break;
						case  5: Sudoku::undo();     Game::draw(); break;
						case  6: Sudoku::clear();    Game::draw(); Button::button = 0; Sudoku::Timer::reset(); break;
						case  7: Sudoku::discard();  Sudoku::Timer::reset(); break;
						case  8: Sudoku::accept();   break;
						case  9: Sudoku::save();     break;
						case 10: if (Sudoku::load()) Game::draw(), Button::button = 0, Sudoku::Timer::start(); break;
						case 11: return;
						}

						Game::update_menu();
					}
					break;

				case MOUSE_MOVED:

					Game::focus   = nullptr;
					Button::focus = nullptr;
					Menu::focus   = nullptr;

					if (x > TAB.left && x < TAB.right && y > TAB.top && y < TAB.bottom)
					{
						x -= TAB.x; y -= TAB.y;
						if (x % 2 != 0 || x % 8 == 0 || y % 4 == 0)
							break;

						x = (x / 2) - (x / 8) - 1;
						y = y - (y / 4) - 1;
						p = y * 9 + x;
						Game::focus = &Game::data()[p];
					}
					else
					if (x >= BTN.left && x <= BTN.right && y > BTN.top && y < BTN.bottom && !solved())
					{
						x -= BTN.x; y -= BTN.y;
						if (y % 4 == 0)
							break;
						y = y - (y / 4) - 1;
						Button::focus = &Game::btn[y];
					}
					else
					if (x > MNU.left && x < MNU.right && y > MNU.top && y < MNU.bottom)
					{
						x -= MNU.x; y -= MNU.y;
						Menu::focus = &Game::mnu[y - 1];
						Menu::back = x < MNU.width / 2;
					}
					break;

				case MOUSE_WHEELED:

					if (x >= TAB.left && x <= BTN.right && y >= BTN.top && y <= BTN.bottom && !solved())
					{
						if ((int) input.Event.MouseEvent.dwButtonState < 0)
							Button::button = (Button::button == 0) ? 1 : 1 + (Button::button + 0) % 9;
						else
							Button::button = (Button::button == 0) ? 9 : 1 + (Button::button + 7) % 9;
					}
					break;
				}
			
				break;

			case KEY_EVENT:

				if (input.Event.KeyEvent.bKeyDown)
				{
					bool prev = false; // input.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED;
					switch (input.Event.KeyEvent.wVirtualKeyCode)
					{
					case '0': Button::button = 0; break;
					case '1': Button::button = 1; break;
					case '2': Button::button = 2; break;
					case '3': Button::button = 3; break;
					case '4': Button::button = 4; break;
					case '5': Button::button = 5; break;
					case '6': Button::button = 6; break;
					case '7': Button::button = 7; break;
					case '8': Button::button = 8; break;
					case '9': Button::button = 9; break;
					case VK_LEFT:  prev = true;   /* falls through */
					case VK_RIGHT:                /* falls through */
					case 'A': Game::help =        static_cast<Assistance>(Game::mnu[1].next(prev)); break;
					case VK_NEXT:  prev = true;   /* falls through */ // PAGE DOWN
					case VK_PRIOR:                /* falls through */ // PAGE UP
					case 'D': Sudoku::level =     static_cast<Difficulty>(Game::mnu[0].next(prev)); /* falls through */
					case VK_TAB:                  /* falls through */
					case 'N': Sudoku::generate(); Game::draw(); Button::button = 0; Sudoku::Timer::start(); break;
					case VK_RETURN:               /* falls through */
					case 'S': Sudoku::solve();    Game::draw(); Button::button = 0; break;
					case VK_BACK:                 /* falls through */
					case 'U': Sudoku::undo();     Game::draw(); break;
					case VK_DELETE:               /* falls through */
					case 'C': Sudoku::clear();    Game::draw(); Button::button = 0; Sudoku::Timer::reset(); break;
					case VK_HOME:                 /* falls through */
					case 'E': Sudoku::discard();  Sudoku::Timer::reset(); break;
					case VK_END:                  /* falls through */
					case 'T': Sudoku::accept();   break;
					case VK_INSERT:               /* falls through */
					case 'V': Sudoku::save();     break;
					case 'L': if (Sudoku::load()) Game::draw(), Button::button = 0, Sudoku::Timer::start(); break;
					case VK_ESCAPE:               /* falls through */
					case 'Q': return;
					}

					Game::update_menu();
				}

				break;
			}

			if (Sudoku::solved())
			{
				Button::button = 0;
				Sudoku::Timer::stop();
			}

			Game::update();
		}

		Game::update_info();
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
			::con.emplace(::title);
			auto sudoku = Game(::title, std::islower(cmd) ? Difficulty::Easy : Difficulty::Medium);
			LONG style = GetWindowLong(::con->Hwnd, GWL_STYLE);
		//	SetWindowLong(::con->Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX));
			SetWindowLong(::con->Hwnd, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));
			sudoku.game();
			SetWindowLong(::con->Hwnd, GWL_STYLE, style);
			::con.reset();
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto data   = std::vector<uint32_t>();

			if (--argc > 0)
				file = *++argv;

			std::cerr << ::title << " find" << std::endl;

			GetAsyncKeyState(VK_ESCAPE);
			while (!GetAsyncKeyState(VK_ESCAPE))
			{
				sudoku.generate();
				if (sudoku.level > Difficulty::Medium && sudoku.len() > 17)
					sudoku.check();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					std::cout << sudoku << std::endl;
					sudoku.append(file);
				}
			}

			std::cerr << ::title << " find: " << data.size() << " boards found, " << sudoku.Timer::get() << 's' << std::endl;
			break;
		}

		case 'T': // test
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " test: " << lst.size() << " boards loaded" << std::endl;

			for (std::string i: lst)
			{
				std::cerr << ' ' << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(false))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? Sudoku::select_rating : Sudoku::select_threshold);

			for (Sudoku &tab: coll)
				std::cout << tab << std::endl;

			std::cerr << ::title << " test: " << data.size() << " boards found, " << sudoku.Timer::get() << 's' << std::endl;
			break;
		}

		case 'S': // sort
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();

			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " sort: " << lst.size() << " boards loaded" << std::endl;

			for (std::string i: lst)
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

			for (Sudoku &tab: coll)
				std::cout << tab << std::endl;

			std::cerr << ::title << " sort: " << data.size() << " boards found, " << sudoku.Timer::get() << 's' << std::endl;
			break;
		}

		case 'C': // check
		{
			auto sudoku = Sudoku(Difficulty::Medium);
			auto data   = std::vector<uint32_t>();
			auto coll   = std::vector<Sudoku>();
			auto lst    = std::vector<std::string>();


			while (--argc > 0)
				Sudoku::load(lst, *++argv);
			if (lst.size() == 0)
				Sudoku::load(lst, file);

			std::cerr << ::title << " check: " << lst.size() << " boards loaded" << std::endl;

			for (std::string i: lst)
			{
				std::cerr << ' ' << ++cnt << '\r';
				sudoku.init(i);
				sudoku.check();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), Sudoku::select_rating);

			for (Sudoku &tab: coll)
				std::cout << tab << std::endl;

			std::cerr << ::title << " check: " << data.size() << " boards found, " << sudoku.Timer::get() << 's' << std::endl;
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
			             "sudoku -T [file] - test and sort by threshold\n"
			             "sudoku -s [file] - sort by rating / length\n"
			             "sudoku -S [file] - sort by length / rating\n"
			             "sudoku -c [file] - check and show extreme only\n"
			             "sudoku -C [file] - check and show all\n"
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
