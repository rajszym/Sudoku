/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    07.10.2020
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
#include <chrono>
#include <cstring> // strlen
#include <windows.h>

const char *title = "Sudoku";

const Console::Rectangle TAB(0, 1, 25, 13);
const Console::Rectangle BAR(TAB.right + 1, TAB.top,  3, TAB.height);
const Console::Rectangle MNU(BAR.right + 1, TAB.top, 14, TAB.height);
const Console::Rectangle WIN(TAB.left, 0, MNU.right - TAB.left + 1, TAB.bottom + 1);

std::optional<Console> con;

struct Button
{
	static int button;
	static int menu;

	int num;

	Button( int n ): num{n} {}

	void draw();
	void update();
};

int Button::button{0};
int Button::menu{0};

void Button::draw()
{
	::con->Put(BAR.x + 1, BAR.y + Button::num + (Button::num - 1) / 3, '0' + Button::num);
}

void Button::update()
{
	int y = Button::num + (Button::num - 1) / 3;

	if (Button::button == Button::num)
		::con->Put(BAR.x + 1, BAR.y + y, Console::Black, Console::White);
	else if (Button::menu == y)
		::con->Put(BAR.x + 1, BAR.y + y, Console::White, Console::Grey);
	else
		::con->Put(BAR.x + 1, BAR.y + y, Console::LightGrey);
}

struct Menu: std::vector<const char *>
{
	static int  menu;
	static bool back;

	const char *key;

	int pos;
	int idx;

	Menu( const char *k, const int p ): key{k}, pos{p}, idx{0} {}

	Menu &add  ( const char * );
	int  next  ( bool );
	void draw  ();
	void update();
};

int  Menu::menu{0};
bool Menu::back{false};

Menu &Menu::add( const char *item )
{
	Menu::push_back(item);
	return *this;
}

int Menu::next( bool prev )
{
	int max = Menu::size() - 1;

	if (Menu::key[0] == 'l')
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

void Menu::draw()
{
	if (Menu::size() > 0)
	{
		unsigned n = std::strlen(Menu::data()[Menu::idx]);
		::con->Put (MNU.x + 1,     MNU.y + Menu::pos, Menu::key);
		::con->Put (MNU.x + 4,     MNU.y + Menu::pos, Menu::data()[Menu::idx]);
		::con->Fill(MNU.x + 4 + n, MNU.y + Menu::pos, MNU.width - 5 - n, 1);
	}
}

void Menu::update()
{
	if (Menu::menu == Menu::pos)
	{
		if (Menu::size() > 1)
			::con->Put (MNU.x + 1, MNU.y + Menu::pos, Menu::back ? "<<" : ">>");
		::con->Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::White, Console::Grey);
	}
	else
	{
		if (Menu::size() > 1)
			::con->Put (MNU.x + 1, MNU.y + Menu::pos, Menu::key);
		::con->Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::LightGrey);
	}
}

struct Game: public Sudoku
{
	static int  focus;
	const char *title;

	bool wait;
	int  help;

	std::vector<Button> btn;
	std::vector<Menu>   mnu;

	Game( const char *, int = 0 );
	~Game();

	void draw_cell    ( Cell & );
	void update_cell  ( Cell & );
	void draw_info    ();
	void draw_menu    ();
	void update_banner();
	void draw         ();
	void update       ();
	void game         ();
};

int Game::focus = -1;

Game::Game( const char *t, int l ): Sudoku{l}, title{t}, wait{false}, help{0}
{
	::con->SetFont(56, L"Consolas");
	::con->Center(WIN.width, WIN.height);
	::con->HideCursor();
	::con->Clear();

	Game::btn = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	Game::mnu.emplace_back("l:",  1); Game::mnu.back().add("easy").add("medium").add("hard").add("expert").add("extreme").idx = Sudoku::level;
	Game::mnu.emplace_back("h:",  2); Game::mnu.back().add("none").add("current").add("available").add("sure").idx = Game::help;
	Game::mnu.emplace_back("n:",  3); Game::mnu.back().add("new");
	Game::mnu.emplace_back("s:",  4); Game::mnu.back().add("solve");
	Game::mnu.emplace_back("u:",  5); Game::mnu.back().add("undo");
	Game::mnu.emplace_back("c:",  6); Game::mnu.back().add("clear");
	Game::mnu.emplace_back("e:",  7); Game::mnu.back().add("edit");
	Game::mnu.emplace_back("f:",  8); Game::mnu.back().add("confirm");
	Game::mnu.emplace_back("v:",  9); Game::mnu.back().add("save");
	Game::mnu.emplace_back("r:", 10); Game::mnu.back().add("read");
	Game::mnu.emplace_back("q:", 11); Game::mnu.back().add("quit");
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
	int n = Button::button;
	int h = Game::help;
	int x = TAB.x + 2 + (cell.pos % 9 + cell.pos % 9 / 3) * 2;
	int y = TAB.y + 1 + (cell.pos / 9 + cell.pos / 9 / 3);

	auto fore = Console::LightGrey;
	auto back = Console::Black;

	if (h >= 1 && cell.equal(n))
	{
		fore = cell.immutable          ? Console::White : Console::LightGreen;
		back = Game::focus == cell.pos ? Console::Grey  : Console::Red;
	}
	else
	if (cell.num != 0)
	{
		fore = cell.immutable          ? Console::White : Console::Green;
		back = Game::focus == cell.pos ? Console::Grey  : Console::Black;
	}
	else
	if (Game::focus == cell.pos)
	{
		if      (h >= 3 && cell.sure(n))    back = Console::Green;
		else if (h >= 2 && cell.allowed(n)) back = Console::Orange;
		else                                back = Console::Grey;
	}

	::con->Put(x, y, fore, back);
}

void Game::draw_info()
{
	char nfo[16];
	snprintf(nfo, sizeof(nfo), "%5d/%d", Sudoku::rating, Sudoku::len());
	::con->Put(BAR.x + 1, WIN.y, Button::button ? '0' + Sudoku::count(Button::button) : ' ');
	::con->Put(MNU.Right(strlen(nfo)) - 1, WIN.y, nfo);
}

void Game::draw_menu()
{
	Game::mnu[0].idx = Sudoku::level; Game::mnu[0].draw();
	Game::mnu[1].idx = Game::help;    Game::mnu[1].draw();
}

void Game::update_banner()
{
	static const Console::Color c[] =
	{
		Console::Blue,
		Console::Green,
		Console::Orange,
		Console::Red,
		Console::Red
	};

	::con->Fill(WIN.x, WIN.y, WIN.width, 1, Console::White, Game::wait ? Console::LightRed : Sudoku::solved() ? Console::Black : c[Sudoku::level]);
}

void Game::draw()
{
	for (Cell &c: *this)
		Game::draw_cell(c);

	Game::draw_info();
	Game::draw_menu();
}

void Game::update()
{
	Game::update_banner();

	for (Cell &c: *this)
		Game::update_cell(c);

	for (Button b: Game::btn)
		b.update();

	for (Menu m: Game::mnu)
		m.update();

	Game::draw_info();
}

void Game::game()
{
	::con->Fill(WIN.x, WIN.y, WIN.width, 1, Console::White);
	::con->Put(TAB.x + 1, WIN.y, Game::title);

	::con->DrawSingle(TAB);
	::con->DrawSingle(TAB.x, TAB.y + (TAB.height - 1) / 3, TAB.width, (TAB.height - 1) / 3 + 1);
	::con->DrawSingle(TAB.x + (TAB.width - 1) / 3, TAB.y, (TAB.width - 1) / 3 + 1, TAB.height);
	::con->DrawSingle(BAR);
	::con->DrawSingle(MNU);

	for (Button b: Game::btn)
		b.draw();

	for (Menu m: Game::mnu)
		m.draw();

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
								Sudoku::set(c, Button::button), Game::draw_cell(c);
							else if (!solved())
								Button::button = c.num;
							break;
						case RIGHTMOST_BUTTON_PRESSED:
							if (!Sudoku::solved() || !c.immutable)
								Button::button = c.num;
							Sudoku::set(c, 0), Game::draw_cell(c);
							break;
						}

						Game::draw_info();
					}
					else
					if (x >= BAR.left && x <= BAR.right && y > BAR.top && y < BAR.bottom && !Sudoku::solved())
					{
						x -= BAR.x; y -= BAR.y;

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
					if (x >= MNU.left && x <= MNU.right && y > MNU.top && y < MNU.bottom)
					{
						x -= MNU.x; y -= MNU.y;

						if (input.Event.MouseEvent.dwButtonState != FROM_LEFT_1ST_BUTTON_PRESSED)
							break;

						Game::wait = true;
						Game::update();

						switch (y)
						{
						case  2:   Game::help  =     Game::mnu[1].next(Menu::back); break;
						case  1: Sudoku::level =     Game::mnu[0].next(Menu::back); /* falls through */
						case  3: Sudoku::generate(); Game::draw(); Button::button = 0; break;
						case  4: Sudoku::solve();    Game::draw(); Button::button = 0; break;
						case  5: Sudoku::undo();     Game::draw(); break;
						case  6: Sudoku::clear();    Game::draw(); Button::button = 0; break;
						case  7: Sudoku::discard();  break;
						case  8: Sudoku::confirm();  break;
						case  9: Sudoku::save();     break;
						case 10: Sudoku::load();     Game::draw(); Button::button = 0; break;
						case 11: return;
						}

						Game::draw_info();
						Game::draw_menu();
						Game::wait = false;
					}
					break;

				case MOUSE_MOVED:

					Menu::menu   = 0;
					Button::menu = 0;
					Game::focus = -1;

					if (x > TAB.left && x < TAB.right && y > TAB.top && y < TAB.bottom)
					{
						x -= TAB.x; y -= TAB.y;
						if (x % 2 != 0 || x % 8 == 0 || y % 4 == 0)
							break;

						x = (x / 2) - (x / 8) - 1;
						y = y - (y / 4) - 1;
						p = y * 9 + x;
						Game::focus = p;
					}
					else
					if (x >= BAR.left && x <= BAR.right && y > BAR.top && y < BAR.bottom && !solved())
					{
						x -= BAR.x; y -= BAR.y;
						Button::menu = y;
					}
					else
					if (x >= MNU.left && x <= MNU.right && y > MNU.top && y < MNU.bottom)
					{
						x -= MNU.x; y -= MNU.y;
						Menu::menu = y;
						Menu::back = x < MNU.width / 2;
					}
					break;

				case MOUSE_WHEELED:

					if (x >= TAB.left && x <= BAR.right && y >= BAR.top && y <= BAR.right && !solved())
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

				Game::wait = true;
				Game::update_banner();

				if (input.Event.KeyEvent.bKeyDown)
				{
					bool prev = false; // input.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED;
					switch (input.Event.KeyEvent.wVirtualKeyCode)
					{
					case '0': /* falls through */
					case '1': /* falls through */
					case '2': /* falls through */
					case '3': /* falls through */
					case '4': /* falls through */
					case '5': /* falls through */
					case '6': /* falls through */
					case '7': /* falls through */
					case '8': /* falls through */
					case '9': Button::button = input.Event.KeyEvent.wVirtualKeyCode - '0'; break;
					case VK_LEFT:  prev = true;   /* falls through */
					case VK_RIGHT:                /* falls through */
					case 'H':   Game::help  =     Game::mnu[1].next(prev); break;
					case VK_NEXT:  prev = true;   /* falls through */ // PAGE DOWN
					case VK_PRIOR:                /* falls through */ // PAGE UP
					case 'L': Sudoku::level =     Game::mnu[0].next(prev); /* falls through */
					case VK_TAB:                  /* falls through */
					case 'N': Sudoku::generate(); Game::draw(); Button::button = 0; break;
					case VK_RETURN:               /* falls through */
					case 'S': Sudoku::solve();    Game::draw(); Button::button = 0; break;
					case VK_BACK:                 /* falls through */
					case 'U': Sudoku::undo();     Game::draw(); break;
					case VK_DELETE:               /* falls through */
					case 'C': Sudoku::clear();    Game::draw(); Button::button = 0; break;
					case VK_HOME:                 /* falls through */
					case 'E': Sudoku::discard();  break;
					case VK_END:                  /* falls through */
					case 'F': Sudoku::confirm();  break;
					case VK_INSERT:               /* falls through */
					case 'V': Sudoku::save();     break;
					case 'R': Sudoku::load();     Game::draw(); Button::button = 0; break;
					case 'T': Sudoku::check();    Game::draw(); break;
					case VK_ESCAPE:               /* falls through */
					case 'Q': return;
					}
				}

				Game::draw_info();
				Game::draw_menu();
				Game::wait = false;

				break;
			}

			if (Sudoku::solved())
				Button::button = 0;

			Game::update();
		}
	}
}

double elapsed( std::chrono::time_point<std::chrono::high_resolution_clock> &start )
{
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = stop - start;
	return diff.count();
}

int main( int argc, char **argv )
{
	int  cnt  = 0;
	char cmd  = 'g';
	auto file = std::string(*argv) + ".board";

	if (--argc > 0 && (**++argv == '/' || **argv == '-'))
		cmd = *++*argv;

	auto start = std::chrono::high_resolution_clock::now();

	switch (std::toupper(cmd))
	{
		case 'G': // game
		{
			::con.emplace(::title);
			auto sudoku = Game(::title, std::islower(cmd) ? 0 : 1);
			sudoku.game();
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<uint32_t>();

			if (--argc > 0)
				file = *++argv;

			std::cerr << ::title << " find" << std::endl;

			GetAsyncKeyState(VK_ESCAPE);
			while (!GetAsyncKeyState(VK_ESCAPE))
			{
				sudoku.generate();
				if (sudoku.level > 1 && sudoku.len() > 17)
					sudoku.check();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					std::cout << sudoku << std::endl;
					sudoku.append(file);
				}
			}

			std::cerr << ::title << " find: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'T': // test
		{
			auto sudoku = Sudoku(1);
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

			std::cerr << ::title << " test: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'S': // sort
		{
			auto sudoku = Sudoku(1);
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

			std::cerr << ::title << " sort: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'C': //check
		{
			auto sudoku = Sudoku(1);
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

			std::cerr << ::title << " check: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
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
