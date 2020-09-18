/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    18.09.2020
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

#include "console.hpp"

const Console::Rectangle TAB(0, 1, 25, 13);
const Console::Rectangle BAR(TAB.right + 1, TAB.top,  3, TAB.height);
const Console::Rectangle MNU(BAR.right + 1, TAB.top, 14, TAB.height);
const Console::Rectangle WIN(TAB.left, 0, MNU.right - TAB.left + 1, TAB.bottom + 1);

const char *title = "SUDOKU";

auto con = Console(title);

auto rnd = std::mt19937_64(std::time(nullptr));

std::vector<std::string> extreme =
{
".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..",//3:21:702:9cd895a7
"7.48..............328...16....2....15.......8....93........6.....63..5...351.2...",//3:22:642:78d35561
"52.....8...........1....7.575694......467...............8.1..29.6...24.......9..8",//3:23:630:1304dc2d
"...2.8.1..4.3.18............94.2...56.7.5..8.1........7.6...35......7..44........",//3:23:584:b52495ca
"2.....31..9.3.......35.64..721.........1.3.7....7.4....18.....5....3.6..........8",//3:23:584:c8a9dcea
"..7.........9....384..1..2..7....2..36....7.......7.8.......94.18..4...2.....216.",//3:23:583:068b3d5a
".56....82..........28...1.6....56.....5..13....14.........1...8.....2..7.7.59.4..",//3:23:583:44a09195
".9............15...68........2.5.4...5.8...9........5....649185...1.....4.....967",//3:23:583:a672ceba
"....14.3.1..9...47.4..63.1..5.....2...9....5.42....16..6..32...8................1",//3:24:572:49903533
"2...........8...6.5...2.3.4...958...9...42....8........1..8..498.......142..7...5",//3:24:572:5adcc2a2
".5.......4.....753....579.1.4.8...3.9.2..........4.....9.6.......4.7.26..2...4.9.",//3:24:572:5bb73733
"..7......12....4.3......9..2.6.947.....278...7....1.9......9..8...4.2....5...3..9",//3:24:572:ceed2c1e
"7....4...32..57...9...6..7....79..62.....1....356.........7.4..5..2.9.3.......8..",//3:24:572:f634a664
".68.......52..7..........845..3...9..7...5...1..............5.78........3..4..2.8",//3:20:567:020b33c2
".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....",//3:21:556:bd715305
"..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...",//3:21:555:24160b86
"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..",//3:21:555:a331b75e
"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1",//3:21:555:d35727f5
"6....5....9....4.87..2............1..1....764....1.8.9.....2....4.6.....38.5.....",//3:21:555:f33dc0aa
".5.....8.71.64...9.........57...2..1...7....5..29....4.27.........1......6..3...7",//3:22:548:e5fd5428
"..6...1.882..9.6.......................315.46.1...6..57...3.2....3....9...4..8...",//3:22:545:1262668b
"...9..6.......71...64.5.3..............8.2....16......9......5.675.2..4.3....5.2.",//3:22:545:2199fc41
"...37...88..........71....9.13......7....5.2........86.4.2.6.....9....6......98.4",//3:22:545:306cb144
".1...........6.....39..8.7...4.....5.8...59...6.7.1....4.92..6...2............852",//3:22:545:7848155d
"....2..56..7....8...8..5..2..1..6...........5.9..4..1.46....8...1.26..3....9.....",//3:22:545:7b0d8493
"4....1.....9...5...6...9..4.9........2...73....1....8.8..5....1.4.21.8.....8...3.",//3:22:545:f1281efc
"..........6..9712.....1.9.7...2.4.568.5..........53.....9..87....7.2..9.......4..",//3:23:541:1da63c2d
"...9.8.75........3.2..3...4......1.74.........79..643..5..73...8............92.4.",//3:23:536:08e08fc9
".8.2...7...1.7.6.....5..8....7.45...8........5647.....248............3.1.....9.6.",//3:23:536:58a36231
"8.5....1..14...58.3...........1.......8.64.......32.........6...6...32....17.6.95",//3:23:536:ca492fe3
"827.....1...8..4371........2..9............79..56......1..48..........58...7.6.4.",//3:23:536:da6aa4ce
".82.........6.....9....2..7.1...3..4.......654......8127.5..4..3.584....1........",//3:23:536:fc84c257
};

struct CRC32
{
	template<class T>
	unsigned operator()( const T  data,              unsigned crc ) { return calc(&data,        sizeof(T), crc); }
	template<class T>
	unsigned operator()( const T *data, size_t size, unsigned crc ) { return calc( data, size * sizeof(T), crc); }
private:
	unsigned calc( const void *, size_t, unsigned );
};

struct Button
{
	static int button;
	static int menu;

	int num;

	Button( int n ): num(n) {}

	void draw  ();
	void update();
};

struct Menu: std::vector<const char *>
{
	static int  menu;
	static bool back;

	const char *key;
	int pos;
	int idx;

	Menu( const char *k, const int p ): key(k), pos(p), idx(0) {}

	Menu &add  ( const char * );
	int  next  ( bool );
	void draw  ();
	void update();
};

struct Cell
{
	static Cell *focus;

	unsigned pos;
	int  num;
	bool immutable;
	std::pair<int, bool> tmp;

	Cell( int p, int n = 0 ): pos(p), num(n), immutable(false), tmp(std::pair<int, bool> { 0, false }) {}

	std::vector<Cell *> lst;
	std::vector<Cell *> row;
	std::vector<Cell *> col;
	std::vector<Cell *> seg;

	void link   ( std::vector<Cell *> & );
	int  len    ();
	int  range  ();
	bool equal  ( int );
	bool dummy  ();
	bool allowed( int );
	int  sure   ( int = 0 );
	void clear  ();
	bool set    ( int, bool );
	void reload ();
	void restore();
	void restore( std::pair<int, bool> );
	bool reset  ();
	bool changed();
	void put    ( std::ostream & );
	void draw   ();
	void update ( int, int );
};

struct Value: public std::array<int, 10>
{
	Value( Cell * );

	int  len    ();
	void shuffle();
};

struct Sudoku: public std::vector<Cell *>
{
	bool wait;
	int  help;
	int  level;
	int  rating;
	unsigned signature;

	Sudoku( int = 0 );

	std::vector<Button> btn;
	std::vector<Menu>   mnu;

	int  len          ();
	int  len          ( int );
	bool empty        ();
	bool solved       ();
	bool difficult    ();
	bool tips         ();

	void reload       ();
	void restore      ();
	bool changed      ();
	void clear        ( bool = true );
	void discard      ();
	void confirm      ( bool = true );
	void init         ( std::string );
	void again        ();
	void swap_cells   ( int, int );
	void swap_rows    ( int, int );
	void swap_cols    ( int, int );
	void shuffle      ();
	bool solvable     ();
	bool correct      ();
	bool simplify     ();
	bool solve_next   ( std::vector<Cell *> &, bool = false );
	void solve        ();
	bool check_next   ( Cell *, bool );
	void check        ();
	bool generate_next( Cell *, bool = false );
	void generate     ();
	int  rating_next  ();
	void rating_calc  ();
	void signat_calc  ();
	void level_calc   ();
	void specify      ();
	void put          ( std::ostream & = std::cout );
	void save         ( std::string = "sudoku.board" );
	void read         ( std::string = "sudoku.board" );
	bool test         ( bool = false );
	void draw         ();
	void update_info  ();
	void update_menu  ();
	void update_banner();
	void update       ();
	void back         ();
	void game         ();
};

template<class T>
struct Temp
{
	T *tmp;

	Temp( T *obj ): tmp(obj) { tmp->reload();  }
	~Temp()                  { tmp->restore(); }
};

struct Base
{
	int level;
	int len;
	int rating;
	unsigned signature;
	int data[81];

	Base( Sudoku & );

	void put();
};

std::list<std::pair<Cell *, int>> undo;

unsigned CRC32::calc( const void *data, size_t size, unsigned crc )
{
	const unsigned char *buffer = reinterpret_cast<const unsigned char *>(data);

	#define POLY 0xEDB88320

	crc = ~crc;
	while (size--)
	{
		crc ^= *buffer++;
		for (int i = 8; i > 0; i--)
			crc = (crc & 1) ? (crc >> 1) ^ (POLY) : (crc >> 1);
	}
	crc = ~crc;

	return crc;
}

int Button::button = 0;
int Button::menu   = 0;

void Button::draw()
{
	con.Put(BAR.x + 1, BAR.y + Button::num + (Button::num - 1) / 3, '0' + Button::num);
}

void Button::update()
{
	int y = Button::num + (Button::num - 1) / 3;
	if (Button::button == Button::num)
		con.Put(BAR.x + 1, BAR.y + y, Console::Black, Console::White);
	else if (Button::menu == y)
		con.Put(BAR.x + 1, BAR.y + y, Console::White, Console::Grey);
	else
		con.Put(BAR.x + 1, BAR.y + y, Console::LightGrey);
}

int  Menu::menu = 0;
bool Menu::back = false;

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
		con.Put (MNU.x + 1,     MNU.y + Menu::pos, Menu::key);
		con.Put (MNU.x + 4,     MNU.y + Menu::pos, Menu::data()[Menu::idx]);
		con.Fill(MNU.x + 4 + n, MNU.y + Menu::pos, MNU.width - 5 - n, 1);
	}
}

void Menu::update()
{
	if (Menu::menu == Menu::pos)
	{
		if (Menu::size() > 1)
			con.Put (MNU.x + 1, MNU.y + Menu::pos, Menu::back ? "<<" : ">>");
		con.Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::White, Console::Grey);
	}
	else
	{
		if (Menu::size() > 1)
			con.Put (MNU.x + 1, MNU.y + Menu::pos, Menu::key);
		con.Fill(MNU.x + 1, MNU.y + Menu::pos, MNU.width - 2, 1, Console::LightGrey);
	}
}

Value::Value( Cell *cell )
{
 	std::iota(Value::begin(), Value::end(), 0);
	Value::data()[cell->num] = 0;
	for (Cell *c: cell->lst) Value::data()[c->num] = 0;
}

int Value::len()
{
	int result = 0;
	for (int v: *this) if (v != 0) result++;
	return result;
}

void Value::shuffle()
{
	std::shuffle(Value::begin(), Value::end(), rnd);
}

Cell *Cell::focus = nullptr;

void Cell::link( std::vector<Cell *> &tab )
{
	int tr = Cell::pos / 9;
	int tc = Cell::pos % 9;
	int ts = (tr / 3) * 3 + (tc / 3);

	for (Cell *c: tab)
	{
		if (c == this) continue;

		int cr = c->pos / 9;
		int cc = c->pos % 9;
		int cs = (cr / 3) * 3 + (cc / 3);

		if (cr == tr || cc == tc || cs == ts) { Cell::lst.push_back(c); c->lst.push_back(this); }
		if (cr == tr)                         { Cell::row.push_back(c); c->row.push_back(this); }
		if             (cc == tc)             { Cell::col.push_back(c); c->col.push_back(this); }
		if                         (cs == ts) { Cell::seg.push_back(c); c->seg.push_back(this); }
	}
}

int Cell::len()
{
	if (Cell::num != 0) return 0;

	Value val(this);
	return val.len();
}

int Cell::range()
{
	if (Cell::num != 0) return 0;

	int result = Cell::len();
	for (Cell *c: Cell::lst) result += c->len();
	return result;
}

bool Cell::equal( int n )
{
	return Cell::num != 0 && Cell::num == n;
}

bool Cell::dummy()
{
	return Cell::num == 0 && Cell::len() == 0;
}

bool convergent( std::vector<Cell *> &lst )
{
	for (Cell *c: lst)
		if (c->dummy())
			return false;
	return true;
}

bool Cell::allowed( int n )
{
	if (Cell::num != 0 || n == 0)
		return false;

	for (Cell *c: Cell::lst)
		if (c->num == n)
			return false;

	Cell::num = n;
	bool result = ::convergent(Cell::lst);
	Cell::num = 0;

	return result;
}

bool allowed( std::vector<Cell *> &lst, int n )
{
	for (Cell *c: lst)
		if (c->allowed(n))
			return true;
	return false;
}

int Cell::sure( int n )
{
	if (Cell::num == 0 && n == 0)
	{
		Value val(this);
		for (int v: val)
			if (v != 0 && Cell::sure(v))
				return v;
		return 0;
	}

	if (!Cell::allowed(n))  return 0;
	if ( Cell::len() == 1)  return n;
	if (!::allowed(row, n)) return n;
	if (!::allowed(col, n)) return n;
	if (!::allowed(seg, n)) return n;

	return 0;
}

void Cell::clear()
{
	Cell::num = 0;
	Cell::immutable = false;
}

bool Cell::set( int n, bool real )
{
	if (!Cell::allowed(n) && (n != 0 || Cell::immutable))
		return false;
	std::swap(Cell::num, n);
	if (real)
	{
		undo.emplace_back(this, n);
		draw();
	}
	return true;
}

void Cell::reload()
{
	Cell::tmp = std::pair<int, bool> { Cell::num, Cell::immutable };
}

void Cell::restore()
{
	Cell::restore(Cell::tmp);
}

void Cell::restore( std::pair<int, bool> old )
{
	Cell::num = std::get<int>(old);
	Cell::immutable = std::get<bool>(old);
}

bool Cell::reset()
{
	return Cell::set(std::get<int>(Cell::tmp), false);
}

bool Cell::changed()
{
	return Cell::num != std::get<int>(Cell::tmp);
}

void Cell::put( std::ostream &out )
{
	out << (Cell::immutable ? ".123456789" : ".ABCDEFGHI")[Cell::num];
}

void Cell::draw()
{
	int x = TAB.x + 2 + (Cell::pos % 9 + Cell::pos % 9 / 3) * 2;
	int y = TAB.y + 1 + (Cell::pos / 9 + Cell::pos / 9 / 3);

	con.Put(x, y, "-123456789"[Cell::num]);
}

void Cell::update(int n, int h)
{
	int x = TAB.x + 2 + (Cell::pos % 9 + Cell::pos % 9 / 3) * 2;
	int y = TAB.y + 1 + (Cell::pos / 9 + Cell::pos / 9 / 3);

	auto c = Console::LightGrey;
	auto b = Console::Black;

	if (h >= 1 && Cell::equal(n)) {
		c = Cell::immutable     ? Console::White : Console::LightGreen;
		b = Cell::focus == this ? Console::Grey  : Console::Red;
	}
	else if (Cell::num != 0) {
		c = Cell::immutable     ? Console::White : Console::Green;
		b = Cell::focus == this ? Console::Grey  : Console::Black;
	}
	else if (Cell::focus == this) {
		if      (h >= 3 && Cell::sure(n))    b = Console::Red;
		else if (h >= 2 && Cell::allowed(n)) b = Console::Orange;
		else                                 b = Console::Grey;
	}

	con.Put(x, y, c, b);
}

bool select_cell( Cell *a, Cell *b )
{
	return a->num == 0 && (b->num != 0          ||
	                       a->len()  < b->len() ||
	                      (a->len() == b->len() && a->range() < b->range()));
}

Sudoku::Sudoku( int l ): wait(false), help(0), level(l), rating(0), signature(0)
{
	for (int i = 0; i < 81; i++)
	{
		Cell *c = new Cell(i);
		Sudoku::emplace_back(c);
		c->link(*this);
	}

	for (int i = 1; i < 10; i++)
		Sudoku::btn.emplace_back(i);

	Sudoku::mnu.emplace_back("h:",  2); Sudoku::mnu.back().add("none").add("current").add("available").add("sure").idx = Sudoku::help;
	Sudoku::mnu.emplace_back("l:",  1); Sudoku::mnu.back().add("easy").add("medium").add("hard").add("expert").add("extreme").idx = Sudoku::level;
	Sudoku::mnu.emplace_back("n:",  3); Sudoku::mnu.back().add("new");
	Sudoku::mnu.emplace_back("s:",  4); Sudoku::mnu.back().add("solve");
	Sudoku::mnu.emplace_back("u:",  5); Sudoku::mnu.back().add("undo");
	Sudoku::mnu.emplace_back("c:",  6); Sudoku::mnu.back().add("clear");
	Sudoku::mnu.emplace_back("t:",  7); Sudoku::mnu.back().add("test");
	Sudoku::mnu.emplace_back("f:",  8); Sudoku::mnu.back().add("confirm");
	Sudoku::mnu.emplace_back("v:",  9); Sudoku::mnu.back().add("save");
	Sudoku::mnu.emplace_back("r:", 10); Sudoku::mnu.back().add("read");
	Sudoku::mnu.emplace_back("q:", 11); Sudoku::mnu.back().add("quit");
}

int Sudoku::len()
{
	int result = 0;
	for (Cell *c: *this)
		if (c->num != 0) result++;
	return result;
}

int Sudoku::len( int num )
{
	int result = 0;
	for (Cell *c: *this)
		if (c->num == num) result++;
	return result;
}

bool Sudoku::empty()
{
	for (Cell *c: *this)
		if (c->num != 0)
			return false;
	return true;
}

bool Sudoku::solved()
{
	for (Cell *c: *this)
		if (c->num == 0)
			return false;
	return true;
}

bool Sudoku::difficult()
{
	return Sudoku::rating >= (Sudoku::len() - 2) * 25;
}

bool Sudoku::tips()
{
	for (Cell *c: *this)
		if (c->sure(0) != 0)
			return true;
	return false;
}

void Sudoku::reload()
{
	for (Cell *c: *this)
		c->reload();
}

void Sudoku::restore()
{
	for (Cell *c: *this)
		c->restore();
}

bool Sudoku::changed()
{
	for (Cell *c: *this)
		if (c->changed())
			return true;
	return false;
}

void Sudoku::clear( bool deep )
{
	for (Cell *c: *this)
		c->clear();

	if (deep)
	{
		Sudoku::rating = Sudoku::signature = 0;
		if (Sudoku::level > 0 && Sudoku::level < 4) Sudoku::level = 1;
	}
}

void Sudoku::discard()
{
	for (Cell *c: *this)
		c->immutable = false;
}

void Sudoku::confirm( bool deep )
{
	for (Cell *c: *this)
		c->immutable = c->num != 0;

	if (deep)
	{
		Sudoku::specify();
		undo.clear();
	}
}

void Sudoku::init( std::string txt )
{
	Sudoku::clear();
	for (Cell *c: *this)
	{
		if (c->pos < txt.size())
		{
			unsigned char x = txt[c->pos] - '0';
			c->set(x <= 9 ? x : 0, false);
		}
	}
	Sudoku::confirm();
	for (Cell *c: *this)
	{
		if (c->pos < txt.size())
		{
			unsigned char x = txt[c->pos] - '@';
			c->set(x <= 9 ? x : 0, false);
		}
	}
}

void Sudoku::again()
{
	for (Cell *c: *this)
		if (!c->immutable)
			c->num = 0;
	undo.clear();
}

void Sudoku::swap_cells( int p1, int p2 )
{
	std::swap(Sudoku::data()[p1]->num,       Sudoku::data()[p2]->num);
	std::swap(Sudoku::data()[p1]->immutable, Sudoku::data()[p2]->immutable);
}

void Sudoku::swap_rows( int r1, int r2 )
{
	r1 *= 9; r2 *= 9;
	for (int c = 0; c < 9; c++)
		Sudoku::swap_cells(r1 + c, r2 + c);
}

void Sudoku::swap_cols( int c1, int c2 )
{
	for (int r = 0; r < 81; r += 9)
		Sudoku::swap_cells(r + c1, r + c2);
}

void Sudoku::shuffle()
{
	int v[10];
 	std::iota(v, v + 10, 0);
	std::shuffle(v + 1, v + 10, rnd);
	for (Cell *c: *this) c->num = v[c->num];

	for (int i = 0; i < 81; i++)
	{
		int c1 = rnd() % 9;
		int c2 = 3 * (c1 / 3) + (c1 + 1) % 3;
		Sudoku::swap_cols(c1, c2);

		int r1 = rnd() % 9;
		int r2 = 3 * (r1 / 3) + (r1 + 1) % 3;
		Sudoku::swap_rows(r1, r2);

		c1 = rnd() % 3;
		c2 = (c1 + 1) % 3;
		c1 *= 3; c2 *= 3;
		for (int j = 0; j < 3; j++)
			Sudoku::swap_cols(c1 + j, c2 + j);

		r1 = rnd() % 3;
		r2 = (r1 + 1) % 3;
		r1 *= 3; r2 *= 3;
		for (int j = 0; j < 3; j++)
			Sudoku::swap_rows(r1 + j, r2 + j);
	}
}

bool Sudoku::solvable()
{
	if (!::convergent(*this))
		return false;

	auto tmp = Temp<Sudoku>(this);

	Sudoku::clear(false);
	for (Cell *c: *this)
		if (!c->reset())
			return false;
	return true;
}

bool Sudoku::correct()
{
	if (Sudoku::len() < 17)
		return false;

	auto tmp = Temp<Sudoku>(this);

	Sudoku::solve_next(*this);
	for (Cell *c: *this)
		if (Sudoku::generate_next(c, true) == c->immutable)
			return false;
	return true;
}

bool Sudoku::simplify()
{
	bool result = false;

	bool simplified;
	do
	{
		simplified = false;
		for (Cell *c: *this)
			if (c->num == 0 && (c->num = c->sure(0)) != 0)
				simplified = result = true;
	}
	while (simplified);

	return result;
}

bool Sudoku::solve_next( std::vector<Cell *> &lst, bool check )
{
	              Cell *cell = *std::min_element(    lst.begin(),     lst.end(), ::select_cell);
	if (cell->num != 0) cell = *std::min_element(Sudoku::begin(), Sudoku::end(), ::select_cell);
	if (cell->num != 0) return true;

	Value val(cell); val.shuffle();
	for (int v: val)
		if ((cell->num = v) != 0 && Sudoku::solve_next(cell->lst, check))
		{
			if (check)
				cell->num = 0;
			return true;
		}

	cell->num = 0;
	return false;
}

void Sudoku::solve()
{
	if (Sudoku::solvable())
	{
		Sudoku::solve_next(*this);
		undo.clear();
	}
}

bool Sudoku::check_next( Cell *cell, bool strict )
{
	if (cell->num == 0)
		return false;

	int num = cell->num;

	cell->num = 0;
	if (cell->sure(num))
	{
		if (!strict)
			return true;
		cell->num = num;
		return false;
	}

	cell->num = num;
	Value val(cell);
	for (int v: val)
		if ((cell->num = v) != 0 && Sudoku::solve_next(cell->lst, true))
		{
			cell->num = num;
			return false;
		}

	cell->num = 0;
	return true;
}

void Sudoku::check()
{
	Sudoku::level = 1;
	Sudoku::confirm();

	if (Sudoku::level == 1)
		return;

	if (Sudoku::level == 2)
		Sudoku::simplify();

	int len = Sudoku::len();

	std::vector<Cell *> tab(*this);

	do
	{
		if (Sudoku::len() > len) Sudoku::restore();
		else Sudoku::reload(), len = Sudoku::len();

		bool changed;

		do
		{
			changed = false;
			std::shuffle(tab.begin(), tab.end(), rnd);
			for (Cell *c: tab)
			{
				c->immutable = false;
				if (Sudoku::check_next(c, true))
					changed = true;
			}
		}
		while (changed);

		do
		{
			changed = false;
			std::shuffle(tab.begin(), tab.end(), rnd);
			for (Cell *c: tab)
				if (Sudoku::check_next(c, false))
					changed = true;
		}
		while (changed);

		Sudoku::simplify();
	}
	while (Sudoku::changed());

	Sudoku::confirm();
}

bool Sudoku::generate_next( Cell *cell, bool check )
{
	if (cell->num == 0 || cell->immutable)
		return false;

	int num = cell->num;

	cell->num = 0;
	if (cell->sure(num))
		return true;

	cell->num = num;
	if (Sudoku::level == 0 && !check)
		return false;

	Value val(cell);
	for (int v: val)
		if ((cell->num = v) != 0 && Sudoku::solve_next(cell->lst, true))
		{
			cell->num = num;
			return false;
		}

	cell->num = 0;
	return true;
}

void Sudoku::generate()
{
	if (Sudoku::level == 4)
	{
		Sudoku::init(extreme[rnd() % extreme.size()]);
		Sudoku::shuffle();
	}
	else
	{
		Sudoku::clear();
		Sudoku::solve_next(*this);
		std::vector<Cell *> tab(*this);
		std::shuffle(tab.begin(), tab.end(), rnd);
		for (Cell *c: tab)
			Sudoku::generate_next(c);
		Sudoku::confirm();
	}
}

int Sudoku::rating_next()
{
	std::vector<std::pair<Cell *, int>> sure;
	for (Cell *c: *this)
		if (c->num == 0)
		{
			int n = c->sure();
			if (n != 0)
				sure.emplace_back(c, n);
			else
			if (c->len() < 2) // wrong way
				return 0;
		}

	if (!sure.empty())
	{
		int  result  = 0;
		bool success = true;
		for (std::pair<Cell *, int> p: sure)
			if (!std::get<Cell *>(p)->set(std::get<int>(p), false))
				success = false;
		if (success)
			result = Sudoku::rating_next() + 1;
		for (std::pair<Cell *, int> p: sure)
			std::get<Cell *>(p)->num = 0;
		return result;
	}
			
	Cell *cell = *std::min_element(Sudoku::begin(), Sudoku::end(), ::select_cell);
	if (cell->num != 0) // solved!
		return 1;

	int len    = cell->len();
	int range  = cell->range();
	int result = 0;
	for (Cell *c: *this)
		if (c->num == 0 && c->len() == len && c->range() == range)
		{
			Value val(c);
			int r = 0;
			for (int v: val)
				if (v != 0 && c->set(v, false))
				{
					r += Sudoku::rating_next();
					c->num = 0;
				}
			if (result == 0 || r < result) result = r;
		}

	return result + 1;
}

void Sudoku::rating_calc()
{
	if (!Sudoku::solvable()) { Sudoku::rating = -2; return; }
	if (!Sudoku::correct())  { Sudoku::rating = -1; return; }

	Sudoku::solve_next(*this);
	Sudoku::reload();
	Sudoku::again();

	Sudoku::rating = 0;
	int msb = 0;
	int result = Sudoku::rating_next();
	for (int i = Sudoku::len(0); result > 0; Sudoku::rating += i--, result >>= 1)
		msb = (result & 1) ? msb + 1 : 0;
	Sudoku::rating += msb - 1;
//	Sudoku::rating = Sudoku::rating_next();
}

void Sudoku::level_calc()
{
	if ( Sudoku::level == 0) {                    return; }
	if ( Sudoku::level == 4) {                    return; }
	if ( Sudoku::rating < 0) { Sudoku::level = 1; return; }
	if ( Sudoku::solved())   { Sudoku::level = 1; return; }
	if (!Sudoku::simplify()) { Sudoku::level = 3; return; }
	if (!Sudoku::solved())   { Sudoku::level = 2;         }
	else                     { Sudoku::level = 1;         }
	Sudoku::again();
}

void Sudoku::signat_calc()
{
	unsigned x[10] = { 0 };
	unsigned t[81];

	for (Cell *c: *this)
	{
		x[c->num]++;
		t[c->pos] = (unsigned)c->range();
	}

	std::sort(x, x + 10);
	std::sort(t, t + 81);

	CRC32 crc32;
	Sudoku::signature = crc32(x, 10, 0);
	Sudoku::signature = crc32(t, 81, Sudoku::signature);
}

void Sudoku::specify()
{
	Sudoku::rating_calc();
	Sudoku::level_calc();
	Sudoku::signat_calc();
}

void Sudoku::put( std::ostream &out )
{
	out << '"';
	for (Cell *c: *this) c->put(out);
	out << "\",//"      << Sudoku::level     << ':'
	    << std::setw(2) << Sudoku::len()     << ':'
	    << std::setw(3) << Sudoku::rating    << ':'
	    << std::setw(8) << std::setfill('0') << std::hex << Sudoku::signature
	                    << std::setfill(' ') << std::dec << std::endl;
}

void Sudoku::save( std::string filename )
{
	auto file = std::ofstream(filename.c_str(), std::ios::out);
	if (!file.is_open())
		return;

	Sudoku::put(file);

	file.close();
}

void Sudoku::read( std::string filename )
{
	auto file = std::ifstream(filename.c_str());
	if (!file.is_open())
		return;

	std::string line;
	if (getline(file, line))
	{
		int l = line.find("\"", 1) - 1;

		if (l >= 0 && l <= 81 && line.at(0) == '\"')
		{
			Sudoku::level = 1;
			Sudoku::init(line.substr(1, l));
		}
	}

	file.close();
}

void load( std::string filename )
{
	auto file = std::ifstream(filename.c_str());
	if (!file.is_open())
		return;

	static
	bool done = false;
	if (!done)
	{
		extreme.clear();
		done = true;
	}

	std::string line;
	while (getline(file, line))
	{
		int l = line.find("\"", 1) - 1;

		if (l < 0 || l > 81 || line.at(0) != '\"')
		{
			std::cerr << "ERROR: incorrect board entry" << std::endl;
			continue;
		}

		extreme.push_back(line.substr(1, l));
	}

	file.close();
}

void Sudoku::draw()
{
	for (Cell *c: *this) c->draw();
	Sudoku::update_info();
	Sudoku::update_menu();
}

void Sudoku::update_info()
{
	char nfo[16];
	snprintf(nfo, sizeof(nfo), "%5d/%d", Sudoku::rating, Sudoku::len());
	con.Put(BAR.x + 1, WIN.y, Button::button ? '0' + Sudoku::len(Button::button) : ' ');
	con.Put(MNU.Right(strlen(nfo)) - 1, WIN.y, nfo);
}

void Sudoku::update_menu()
{
	Sudoku::mnu[0].idx = Sudoku::help;  Sudoku::mnu[0].draw();
	Sudoku::mnu[1].idx = Sudoku::level; Sudoku::mnu[1].draw();
}

void Sudoku::update_banner()
{
	static const Console::Color c[] =
	{
		Console::Blue,
		Console::Green,
		Console::Orange,
		Console::Red,
		Console::Red
	};

	con.Fill(WIN.x, WIN.y, WIN.width, 1, Console::White, Sudoku::wait ? Console::LightRed : Sudoku::solved() ? Console::Black : c[Sudoku::level]);
}

void Sudoku::update()
{
	update_banner();

	for (Cell *c: *this)
		c->update(Button::button, help);

	for (Button b: Sudoku::btn)
		b.update();

	for (Menu m: Sudoku::mnu)
		m.update();

	update_info();
}

void Sudoku::back()
{
	if (!undo.empty())
	{
		std::get<Cell *>(undo.back())->num = std::get<int>(undo.back());
		undo.pop_back();
	}
	else
	{
		Sudoku::again();
		Sudoku::specify();
	}
}

void Sudoku::game()
{
	con.Fill(WIN.x, WIN.y, WIN.width, 1, Console::White);
	con.Put(TAB.x + 1, WIN.y, title);

	con.DrawSingle(TAB);
	con.DrawSingle(TAB.x, TAB.y + (TAB.height - 1) / 3, TAB.width, (TAB.height - 1) / 3 + 1);
	con.DrawSingle(TAB.x + (TAB.width - 1) / 3, TAB.y, (TAB.width - 1) / 3 + 1, TAB.height);
	con.DrawSingle(BAR);
	con.DrawSingle(MNU);

	for (Button b: Sudoku::btn)
		b.draw();

	for (Menu m: Sudoku::mnu)
		m.draw();

	Sudoku::generate();
	Sudoku::draw();
	Sudoku::update();

	for (;;)
	{
		int x, y;
		INPUT_RECORD input;

		while (con.GetInput(&input))
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
						Cell *c = Sudoku::data()[y * 9 + x];

						switch (input.Event.MouseEvent.dwButtonState)
						{
						case FROM_LEFT_1ST_BUTTON_PRESSED:
							if (c->num == 0)
								c->set(Button::button, true);
							else if (!solved())
								Button::button = c->num;
							break;
						case RIGHTMOST_BUTTON_PRESSED:
							if (!solved() || !c->immutable)
								Button::button = c->num;
							c->set(0, true);
							break;
						}

						Sudoku::update_info();
					}
					else
					if (x >= BAR.left && x <= BAR.right && y > BAR.top && y < BAR.bottom && !solved())
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

						Sudoku::wait = true;
						Sudoku::update();

						switch (y)
						{
						case  2: Sudoku::help  =     Sudoku::mnu[0].next(Menu::back); break;
						case  1: Sudoku::level =     Sudoku::mnu[1].next(Menu::back); /* falls through */
						case  3: Sudoku::generate(); Sudoku::draw(); Button::button = 0; break;
						case  4: Sudoku::solve();    Sudoku::draw(); Button::button = 0; break;
						case  5: Sudoku::back();     Sudoku::draw(); break;
						case  6: Sudoku::clear();    Sudoku::draw(); Button::button = 0; break;
						case  7: Sudoku::check();    Sudoku::draw(); Sudoku::discard();  break;
						case  8: Sudoku::confirm();  break;
						case  9: Sudoku::save();     break;
						case 10: Sudoku::read();     Sudoku::draw(); Button::button = 0; break;
						case 11: return;
						}

						Sudoku::update_info();
						Sudoku::update_menu();
						Sudoku::wait = false;
					}
					break;

				case MOUSE_MOVED:

					Menu::menu = 0;
					Button::menu = 0;
					Cell::focus = nullptr;

					if (x > TAB.left && x < TAB.right && y > TAB.top && y < TAB.bottom)
					{
						x -= TAB.x; y -= TAB.y;
						if (x % 2 != 0 || x % 8 == 0 || y % 4 == 0)
							break;

						x = (x / 2) - (x / 8) - 1;
						y = y - (y / 4) - 1;
						Cell::focus = Sudoku::data()[y * 9 + x];
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

				Sudoku::wait = true;
				Sudoku::update_banner();

				if (input.Event.KeyEvent.bKeyDown)
				{
					bool prev = false; // input.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED;
					switch (input.Event.KeyEvent.wVirtualKeyCode)
					{
					case VK_LEFT:  prev = true;   /* falls through */
					case VK_RIGHT:                /* falls through */
					case 'H': Sudoku::help  =     Sudoku::mnu[0].next(prev); break;
					case VK_NEXT:  prev = true;   /* falls through */ // PAGE DOWN
					case VK_PRIOR:                /* falls through */ // PAGE UP
					case 'L': Sudoku::level =     Sudoku::mnu[1].next(prev); /* falls through */
					case VK_TAB:                  /* falls through */
					case 'N': Sudoku::generate(); Sudoku::draw(); Button::button = 0; break;
					case VK_RETURN:               /* falls through */
					case 'S': Sudoku::solve();    Sudoku::draw(); Button::button = 0; break;
					case VK_BACK:                 /* falls through */
					case 'U': Sudoku::back();     Sudoku::draw(); break;
					case VK_DELETE:               /* falls through */
					case 'C': Sudoku::clear();    Sudoku::draw(); Button::button = 0; break;
					case VK_HOME:                 /* falls through */
					case 'T': Sudoku::check();    Sudoku::draw(); Sudoku::discard();  break;
					case VK_END:                  /* falls through */
					case 'F': Sudoku::confirm();  break;
					case VK_INSERT:               /* falls through */
					case 'V': Sudoku::save();     break;
					case 'R': Sudoku::read();     Sudoku::draw(); Button::button = 0; break;
					case VK_ESCAPE:               /* falls through */
					case 'Q': return;
					}
				}

				Sudoku::update_info();
				Sudoku::update_menu();
				Sudoku::wait = false;

				break;
			}

			if (Sudoku::solved())
				Button::button = 0;

			Sudoku::update();
		}
	}
}

bool Sudoku::test( bool all )
{
	if (Sudoku::rating == -2) { std::cerr << "ERROR: unsolvable" << std::endl; return false; }
	if (Sudoku::rating == -1) { std::cerr << "ERROR: ambiguous"  << std::endl; return false; }

	return Sudoku::level == 0 || all || Sudoku::difficult();
}

Base::Base( Sudoku &sudoku ): level    (sudoku.level),
                              len      (sudoku.len()),
                              rating   (sudoku.rating),
                              signature(sudoku.signature)
{
	for (Cell *c: sudoku) Base::data[c->pos] = c->num;
}

void Base::put()
{
	auto sudoku = Sudoku(Base::level);

	sudoku.rating    = Base::rating;
	sudoku.signature = Base::signature;
	for (Cell *c: sudoku) c->num = Base::data[c->pos];
	sudoku.put();
}

bool select_rating( Base &a, Base &b )
{
	return a.rating  > b.rating ||
	      (a.rating == b.rating && (a.len  < b.len ||
	                               (a.len == b.len && (a.level  > b.level ||
	                                                  (a.level == b.level && a.signature < b.signature)))));
}

bool select_len( Base &a, Base &b )
{
	return a.len  < b.len ||
	      (a.len == b.len && (a.rating  > b.rating ||
	                         (a.rating == b.rating && (a.level  > b.level ||
	                                                  (a.level == b.level && a.signature < b.signature)))));
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
		case 'C': //check
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				::load(*++argv);

			std::cerr << title << " check: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				std::cerr << ++cnt << '\r';
				sudoku.init(i);
				sudoku.check();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(true))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), ::select_rating);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " check: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();

			if (--argc > 0)
				file = *++argv;

			std::cerr << title << " find" << std::endl;

			GetAsyncKeyState(VK_ESCAPE);
			while (!GetAsyncKeyState(VK_ESCAPE))
			{
				sudoku.generate();
				if (sudoku.level == 2) sudoku.check();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					sudoku.put();
					sudoku.save(file);
				}
			}

			std::cerr << title << " find: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'S': // sort
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				::load(*++argv);

			std::cerr << title << " sort: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				std::cerr << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(true))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? ::select_rating : ::select_len);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " sort: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'T': // test
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				::load(*++argv);

			std::cerr << title << " test: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				std::cerr << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(false))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? ::select_rating : ::select_len);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " test: " << data.size() << " boards found, " << elapsed(start) << 's' << std::endl;
			break;
		}

		case 'G': // game
		{
			auto sudoku = Sudoku();

			while (--argc > 0)
				::load(*++argv);

			if (!con) break;
			con.SetFont(48, L"Consolas");
			con.CenterUp(WIN.width, WIN.height);
			con.HideCursor();
			con.Clear();
			sudoku.game();
			con.Clear();
			break;
		}

		case '?': /* falls through */
		case 'H': // help
		{
			std::cerr << "Sudoku game, solver and generator" << std::endl
			          << std::endl
			          << "Copyright (c) 2018 - 2020 Rajmund Szymanski. All rights reserved." << std::endl
			          << "This software is distributed under the MIT License."               << std::endl
			          << "You are free to modify and redistribute it."                       << std::endl
			          << std::endl
			          << "Usage:"                                            << std::endl
			          << "sudoku  -c [file] - check"                         << std::endl
			          << "sudoku  -f [file] - find"                          << std::endl
			          << "sudoku  -F [file] - find and show all"             << std::endl
			          << "sudoku  -s [file] - sort by rating / len"          << std::endl
			          << "sudoku  -S [file] - sort by len / rating"          << std::endl
			          << "sudoku  -t [file] - test and sort by rating / len" << std::endl
			          << "sudoku  -T [file] - test and sort by len / rating" << std::endl
			          << "sudoku  -g [file] - game (default)"                << std::endl
			          << "sudoku  -h        - help"                          << std::endl
			          << "sudoku  -?        - help"                          << std::endl;
			break;
		}

		default:
		{
			std::cerr << title << ": unknown command" << std::endl;
			break;
		}
	}
}
