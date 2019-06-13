/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    13.06.2019
   @brief   Sudoku game and generator

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

#include <list>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <conio.h>
#include "console.hpp"

#define TEST  true

#define TABX  0
#define TABY  0
#define TABW  25
#define TABH  13

#define BARX (TABX + TABW)
#define BARY (TABY)
#define BARW  3
#define BARH (TABH)

#define MNUX (BARX + BARW + 1)
#define MNUY (BARY)
#define MNUW  12
#define MNUH (BARH)

#define WINW (MNUX + MNUW)
#define WINH (TABY + TABH)

std::vector<std::string> extreme =
{
".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..",//3:21:840:9cd895a7
".8.2...7...1.7.6.....5..8....7.45...8........5647.....248............3.1.....9.6.",//3:23:721:58a36231
"2.....31..9.3.......35.64..721.........1.3.7....7.4....18.....5....3.6..........8",//3:23:721:c8a9dcea
".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....",//3:21:707:bd715305
".5.6.39...........3.6.8.5.75...4...9..13..........1..24.5.3....1.....46.2...1....",//3:24:707:8f597f5f
"....6.......3.5.6..7.....8..3.........5..74..8...9..569..8...45..8....9..5.2.98..",//3:24:707:e70c25db
"7....4...32..57...9...6..7....79..62.....1....356.........7.4..5..2.9.3.......8..",//3:24:707:f634a664
"..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...",//3:21:702:24160b86
"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..",//3:21:702:a331b75e
"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1",//3:21:702:d35727f5
"6....5....9....4.87..2............1..1....764....1.8.9.....2....4.6.....38.5.....",//3:21:702:f33dc0aa
".....8416.154...9.4...7.5.........3....249......3.5...1.4...2....2...9..8...24...",//3:25:693:a5f4df69
"84.9.12....2.83..99......8....1..9.......8..26..7..1...9.217......8.....7.......5",//3:25:693:c7c96e35
"..........7....93.3.8....26.....6..1....1268.16.3....57....1.6..5..6..4.6...2....",//3:25:693:ef3bf228
".1...........6.....39..8.7...4.....5.8...59...6.7.1....4.92..6...2............852",//3:22:689:7848155d
".5.....8.71.64...9.........57...2..1...7....5..29....4.27.........1......6..3...7",//3:22:689:e5fd5428
"82...3..6.....1........5.7..3......4....3....9.2.....5.8...9.2.24.386...79...2.68",//3:26:680:276d53f4
"....5.71....7.......6.4...2..1....3.983..2..7.........15....348..8.....1..45.....",//3:23:676:1128bffd
".79..4.36...6...4...............9..5..6...21..1...8...79.5..3...4.......3.82..9..",//3:23:676:2a16dbd7
"....3..8..2......495......6..61...9.....54..7....9....27.8......6...9.521..3.....",//3:23:676:6e9c5c5e
"2...4.6..........9.5..7.....18...9...2.6..83.....8.2.78.....4.3.........3.4.2..8.",//3:23:676:b7b447bb
"..5.....7..76.2....93.....2.7....38....3.4..613..............53.4...9..8....1.7..",//3:23:676:e3143659
"5.......9.2.........4..285...316...4...728.....6......6..9....51.......7....739..",//3:23:676:f68c25f0
"...2...7...3.......7.41.6..........51....68.9.98...........5.1.5....7.9..698.1...",//3:23:676:ffb46cfd
".7...15..63..4...........8......7.3...5....4......96.....8..9..2...6...1....5...8",//3:20:666:66750dfe
"96..........3...9..37.95.6.4.6.827....81....6..9.......7...1..2..1.3..........5..",//3:24:666:8369df77
".8.64....29.1......4.7....1...2..6.3.2.....1..7....4.....4....69.2..8...8.4....9.",//3:24:664:018133dc
".3.68..5...5.4.8.1.78...3.6..........8..6.2.......5.8..4..3....9..4...1....1...7.",//3:24:664:01a48815
"23......6.....4.5.4.9..............834.2.....5..4.63.1...54367......2.8....6.....",//3:24:664:e110b1bd
".8..16.4..5....97..4...713.4..7........4.2..3.......8...4.......3.6...9.2.5....1.",//3:24:663:00b597cf
".97.8....2......8..68.94...9.2....1......1.3.......87.8...157........3..5.4..2...",//3:24:663:17e5769c
"5.3....6..8..2.......3..91...57..6..6....25.....1..4..8....3.9.......2...9.2...46",//3:24:663:1f6bdc61
};

const char *title = "SUDOKU";

Console con(title);

std::ranlux48 rnd(std::time(nullptr));

#define RND(v) (rnd() % (v))

struct CRC32
{
	template<class T>
	unsigned operator()( const T  data,              unsigned crc ) { return calc(&data,        sizeof(T), crc); }
	template<class T>
	unsigned operator()( const T *data, size_t size, unsigned crc ) { return calc( data, size * sizeof(T), crc); }
private:
	unsigned calc( const void *, size_t, unsigned );
};

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

struct Cell
{
	int  pos;
	int  num;
	bool immutable;
	std::pair<int, bool> tmp;

	Cell( int p, int n = 0 ): pos(p), num(n), immutable(false), tmp(std::pair { 0, false }) {}

	std::vector<Cell *> lst;
	std::vector<Cell *> row;
	std::vector<Cell *> col;
	std::vector<Cell *> seg;

	void link   ( std::vector<Cell *> & );
	int  len    ();
	int  range  ();
	bool equal  ( int );
	bool allowed( int );
	int  sure   ( int = 0 );
	bool dummy  ();
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

Value::Value( Cell *cell )
{
 	std::iota(begin(), end(), 0);
	data()[cell->num] = 0;
	for (Cell *c: cell->lst) data()[c->num] = 0;
}

int Value::len()
{
	int result = 0;
	for (int v: *this) if (v != 0) result++;
	return result;
}

void Value::shuffle()
{
	std::shuffle(begin(), end(), rnd);
}

struct Undo
{
	Cell *cell;
	int   num;

	Undo( Cell *c ): cell(c), num(c->num) {}
};

std::list <Undo> undo;

void Cell::link( std::vector<Cell *> &tab )
{
	int tr = pos / 9;
	int tc = pos % 9;
	int ts = (tr / 3) * 3 + (tc / 3);

	for (Cell *c: tab)
	{
		if (c == this) continue;

		int cr = c->pos / 9;
		int cc = c->pos % 9;
		int cs = (cr / 3) * 3 + (cc / 3);

		if (cr == tr || cc == tc || cs == ts) { lst.push_back(c); c->lst.push_back(this); }
		if (cr == tr)                         { row.push_back(c); c->row.push_back(this); }
		if             (cc == tc)             { col.push_back(c); c->col.push_back(this); }
		if                         (cs == ts) { seg.push_back(c); c->seg.push_back(this); }
	}
}

int Cell::len()
{
	if (num != 0) return 0;

	Value val(this);
	return val.len();
}

int Cell::range()
{
	if (num != 0) return 0;

	int result = len();
	for (Cell *c: lst) result += c->len();
	return result;
}

bool Cell::equal( int n )
{
	return num != 0 && num == n;
}

bool Cell::allowed( int n )
{
	if (num != 0 || n == 0)
		return false;

	for (Cell *c: lst)
		if (c->num == n)
			return false;

	return true;
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
	if (num == 0 && n == 0)
	{
		Value val(this);
		for (int v: val)
			if (v != 0 && sure(v))
				return v;
		return 0;
	}

	if (!allowed(n))        return 0;
	if ( len() == 1)        return n;
	if (!::allowed(row, n)) return n;
	if (!::allowed(col, n)) return n;
	if (!::allowed(seg, n)) return n;

	return 0;
}	

bool Cell::dummy()
{
	if (num == 0 && len() == 0)
		return true;

	return false;
}

void Cell::clear()
{
	num = 0;
	immutable = false;
}

bool Cell::set( int n, bool save )
{
	if (!allowed(n) && (n != 0 || immutable))
		return false;
	if (save)
		undo.emplace_back(this);
	num = n;
	return true;
}

void Cell::reload()
{
	tmp = std::pair { num, immutable };
}

void Cell::restore()
{
	restore(tmp);
}

void Cell::restore( std::pair<int, bool> old )
{
	num = std::get<int>(old);
	immutable = std::get<bool>(old);
}

bool Cell::reset()
{
	return set(std::get<int>(tmp), false);
}

bool Cell::changed()
{
	return num != std::get<int>(tmp);
}

void Cell::put( std::ostream &out )
{
	out << ".123456789"[num];
}

void Cell::draw()
{
	int  x = TABX + 2 + (pos % 9 + pos % 9 / 3) * 2;
	int  y = TABY + 1 + (pos / 9 + pos / 9 / 3);

	con.Put(x, y, "-123456789"[num]);
}

void Cell::update(int n, int h)
{
	int  x = TABX + 2 + (pos % 9 + pos % 9 / 3) * 2;
	int  y = TABY + 1 + (pos / 9 + pos / 9 / 3);

	if      (h >= 3 && sure(n))    con.Put(x, y, Console::White, Console::LightRed);
	else if (h >= 2 && allowed(n)) con.Put(x, y, Console::White, Console::Grey);
	else if (h >= 1 && equal(n))   con.Put(x, y, Console::White, Console::LightBlue);
	else if (          num != 0)   con.Put(x, y, Console::White);
	else                           con.Put(x, y, Console::LightGrey);
}

bool select_cell( Cell *a, Cell *b )
{
	return a->num    == 0        &&
	      (b->num    != 0        ||
	       a->len()   < b->len() ||
	       a->len()  == b->len() &&
	       a->range() < b->range());
}

struct Button
{
	static int button;
	static int menu;

	int num;

	Button( int n ): num(n) {}

	void draw  ();
	void update();
};

int Button::button = 0;
int Button::menu   = 0;

void Button::draw()
{
	con.Put(BARX + 1, BARY + num + (num - 1) / 3, '0' + num);
}

void Button::update()
{
	int y = BARY + num + (num - 1) / 3;
	if (button == num)
		con.Put(BARX + 1, y, Console::Black, Console::White);
	else if (menu == y)
		con.Put(BARX + 1, y, Console::White, Console::Grey);
	else
		con.Put(BARX + 1, y, Console::LightGrey, Console::Black);
}

struct Menu: std::vector<const char *>
{
	static int  menu;
	static bool back;

	int pos;
	int idx;

	Menu( int p ): pos(p), idx(0) {}

	Menu &add  ( const char * );
	int  next  ();
	void draw  ();
	void update();
};

int  Menu::menu = 0;
bool Menu::back = false;

Menu &Menu::add( const char *item )
{
	push_back(item);
	return *this;
}

int Menu::next()
{
	int max = size() - 1;

	if (pos == 5)
	{
		if (back) idx = idx == 0 ? max : idx == max ? 1 : 0;
		else      idx = idx == max ? 0 : idx == 0 ? 1 : max;
	}
	else
	if (max > 0)
	{
		if (back) idx = (idx + max) % (max + 1);
		else      idx = (idx + 1)   % (max + 1);
	}

	return idx;
}

void Menu::draw()
{
	if (size() > 0)
	{
		con.Fill(MNUX, MNUY + pos, MNUW - 2, 1);
		con.Put(MNUX, MNUY + pos, data()[idx]);
	}
}

void Menu::update()
{
	if (menu == pos)
	{
		if (size() == 1)
			con.FillColor(MNUX, MNUY + pos, MNUW, 1, Console::White, Console::Grey);
		else
		{
			con.Put(MNUX + MNUW - 2, MNUY + pos, back ? "<<" : ">>");
			con.FillColor(MNUX, MNUY + pos, MNUW - 2, 1, Console::White, Console::Grey);
			con.FillColor(MNUX + MNUW - 2, MNUY + pos, 2, 1, back ? Console::LightRed : Console::LightGreen, Console::Grey);
		}
	}
	else
	{
		if (size() != 1)
			con.Put(MNUX + MNUW - 2, MNUY + pos, ">>");
		con.FillColor(MNUX, MNUY + pos, MNUW, 1, Console::LightGrey);
	}
}

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
	void confirm      ( bool = true );
	void init         ( std::string );
	void again        ();
	void swap_cells   ( int, int );
	void swap_rows    ( int, int );
	void swap_cols    ( int, int );
	void shuffle      ();
	bool convergent   ( std::vector<Cell *> & );
	bool solvable     ();
	bool correct      ();
	bool simplify     ();
	bool solve_next   ( std::vector<Cell *> &, bool = false );
	void solve        ();
	bool squeeze_next ( Cell *, bool );
	void squeeze      ();
	bool generate_next( Cell *, bool = false );
	void generate     ();
	int  rating_next  ();
	void rating_calc  ();
	void signat_calc  ();
	void level_calc   ();
	void specify      ();
	void put          ( std::ostream & = std::cout );
	void load         ( std::string );
	void save         ( std::string );
	bool test         ( bool = false );
	void draw         ();
	void draw_spec    ();
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

Sudoku::Sudoku( int l ): wait(false), help(0), level(l), rating(0), signature(0)
{
	for (int i = 0; i < 81; i++)
		emplace_back(new Cell(i))->link(*this);

	for (int i = 1; i < 10; i++)
		btn.emplace_back(i);

	mnu.emplace_back( 4).add("NONE").add("CURRENT").add("AVAILABLE").add("SURE").idx = help;
	mnu.emplace_back( 5).add("EASY").add("MEDIUM").add("HARD").add("EXPERT").add("EXTREME").idx = level;
	mnu.emplace_back( 6).add("NEXT");
	mnu.emplace_back( 7).add("CLEAR");
	mnu.emplace_back( 8).add("CONFIRM");
	mnu.emplace_back( 9).add("UNDO");
	mnu.emplace_back(10).add("SOLVE");
	mnu.emplace_back(11).add("EXIT");
}

int Sudoku::len()
{
	int result = 0;
	for (Cell *c: *this)
		if (c->num != 0) result++;
	return result;
}

int Sudoku::len( int n )
{
	int result = 0;
	for (Cell *c: *this)
		if (c->num == n) result++;
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
	return rating >= (len() + 8) * 20;
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
		rating = signature = 0;
		if (level > 0 && level < 4) level = 1;
	}
}

void Sudoku::confirm( bool deep )
{
	for (Cell *c: *this)
		c->immutable = c->num != 0;

	if (deep)
	{
		specify();
		undo.clear();
	}
}

void Sudoku::init( std::string txt )
{
	clear();
	for (Cell *c: *this)
	{
		if (c->pos < txt.size())
		{
			unsigned char x = txt[c->pos] - '0';
			c->set(x <= 9 ? x : 0, false);
//			c->num = x <= 9 ? x : 0;
		}
	}
	confirm();
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
	std::swap(data()[p1]->num,       data()[p2]->num);
	std::swap(data()[p1]->immutable, data()[p2]->immutable);
}

void Sudoku::swap_rows( int r1, int r2 )
{
	r1 *= 9; r2 *= 9;
	for (int c = 0; c < 9; c++)
		swap_cells(r1 + c, r2 + c);
}

void Sudoku::swap_cols( int c1, int c2 )
{
	for (int r = 0; r < 81; r += 9)
		swap_cells(r + c1, r + c2);
}

void Sudoku::shuffle()
{
	int v[10]; for (int i = 0; i < 10; i++) v[i] = i;
	std::shuffle(v + 1, v + 10, rnd);
	for (Cell *c: *this) c->num = v[c->num];
/*
	for (int i = 0; i < 9; i++)
		for (int j = i + 1; j < 9; j++)
			swap_cells(9 * i + j, 9 * j + i);
*/
	for (int i = 0; i < 81; i++)
	{
		int c1 = RND(9);
		int c2 = 3 * (c1 / 3) + (c1 + 1) % 3;
		swap_cols(c1, c2);

		int r1 = RND(9);
		int r2 = 3 * (r1 / 3) + (r1 + 1) % 3;
		swap_rows(r1, r2);

		c1 = RND(3);
		c2 = (c1 + 1) % 3;
		c1 *=3; c2 *= 3;
		for (int i = 0; i < 3; i++)
			swap_cols(c1 + i, c2 + i);

		r1 = RND(3);
		r2 = (r1 + 1) % 3;
		r1 *= 3; r2 *= 3;
		for (int i = 0; i < 3; i++)
			swap_rows(r1 + i, r2 + i);
	}
}

bool Sudoku::convergent( std::vector<Cell *> &lst )
{
	for (Cell *c: lst)
		if (c->dummy())
			return false;

	return true;
}

bool Sudoku::solvable()
{
	if (!convergent(*this))
		return false;

	auto tmp = Temp<Sudoku>(this);

	clear(false);
	for (Cell *c: *this)
		if (!c->reset())
			return false;

	return true;
}

bool Sudoku::correct()
{
	if (len() < 17)
		return false;

	auto tmp = Temp<Sudoku>(this);

	solve_next(*this);

	for (Cell *c: *this)
		if (generate_next(c, TEST) == c->immutable)
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
	              Cell *cell = *std::min_element(lst.begin(), lst.end(), ::select_cell);
	if (cell->num != 0) cell = *std::min_element(    begin(),     end(), ::select_cell);
	if (cell->num != 0) return true;

	Value val(cell); val.shuffle();
	for (int v: val)
		if ((cell->num = v) != 0 && solve_next(cell->lst, check))
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
	if (solvable())
	{
		solve_next(*this);
		undo.clear();
	}
}

bool Sudoku::squeeze_next( Cell *cell, bool strict )
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
		if ((cell->num = v) != 0 && solve_next(cell->lst, TEST))
		{
			cell->num = num;
			return false;
		}

	cell->num = 0;
	return true;
}

void Sudoku::squeeze()
{
	if (level < 2)
		return;

	if (level < 3)
		simplify();

	int length = len();

	std::vector<Cell *> tab(*this);

	do
	{
		if (len() > length) restore();
		else reload(), length = len();

		bool squeezed;

		do
		{
			squeezed = false;
			std::shuffle(tab.begin(), tab.end(), rnd);
			for (Cell *c: tab)
			{
				c->immutable = false;
				if (squeeze_next(c, true))
					squeezed = true;
			}
		}
		while (squeezed);

		do
		{
			squeezed = false;
			std::shuffle(tab.begin(), tab.end(), rnd);
			for (Cell *c: tab)
				if (squeeze_next(c, false))
					squeezed = true;
		}
		while (squeezed);

		simplify();
	}
	while (changed());

	confirm();
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
	if (level == 0 && !check)
		return false;

	Value val(cell);
	for (int v: val)
		if ((cell->num = v) != 0 && solve_next(cell->lst, TEST))
		{
			cell->num = num;
			return false;
		}

	cell->num = 0;
	return true;
}

void Sudoku::generate()
{
	if (level == 4)
	{
		init(extreme[RND(extreme.size())]);
		shuffle();
	}
	else
	{
		clear();
		solve_next(*this);
		std::vector<Cell *> tab(*this);
		std::shuffle(tab.begin(), tab.end(), rnd);
		for (Cell *c: tab)
			generate_next(c);
		confirm();
	}
}

int Sudoku::rating_next()
{
	if (solved())
		return 0;

	std::vector<Cell *> sure;
	for (Cell *c: *this)
		if (c->sure())
			sure.push_back(c);
	if (!sure.empty())
	{
		for (Cell *c: sure)
			c->num = c->sure();
		int result = rating_next();
		for (Cell *c: sure)
			c->num = 0;
		return result + sure.size();
	}
			
	Cell *cell = *std::min_element(begin(), end(), ::select_cell);
	int length = cell->len();

	if (length == 0) // wrong way
		return 1;

	int range  = cell->range();
	int result = ~0U>>1;
	for (Cell *c: *this)
		if (c->num == 0 && c->len() == length && c->range() == range)
		{
			Value val(c);
			int r = 0;
			for (int v: val)
				if ((c->num = v) != 0)
					r += rating_next();
			if (r < result) result = r;
			c->num = 0;
		}

	return result + 1;
}

void Sudoku::rating_calc()
{
	if (!solvable()) { rating = -2; return; }
	if (!correct())  { rating = -1; return; }

	rating = 0;
	int msb = 0;
	int result = rating_next();
	for (int i = len(0); result > 0; rating += i--, result >>= 1)
		msb = (result & 1) ? msb + 1 : 0;
	rating += msb - 1;
}

void Sudoku::level_calc()
{
	if ( level == 0) {            return; }
	if ( level == 4) {            return; }
	if ( rating < 0) { level = 1; return; }
	if ( solved())   { level = 1; return; }
	if (!simplify()) { level = 3; return; }
	if (!solved())   { level = 2;         }
	else             { level = 1;         }
	again();
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
	signature = crc32(x, 10, 0);
	signature = crc32(t, 81, signature);
}

void Sudoku::specify()
{
	rating_calc();
	level_calc();
	signat_calc();
}

void Sudoku::put( std::ostream &out )
{
	out << '"';
	for (Cell *c: *this) c->put(out);
	out << "\",//"      << level    << ':'
	    << std::setw(2) << len()    << ':'
	    << std::setw(3) << rating   << ':'
	    << std::setw(8) << std::setfill('0') << std::hex << signature
	                    << std::setfill(' ') << std::dec << std::endl;
}

void Sudoku::load( std::string filename )
{
	static
	bool done = false;
	if (!done)
	{
		extreme.clear();
		done = true;
	}

	auto file = std::ifstream(filename.c_str());
	if (!file.is_open())
		return;

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

void Sudoku::save( std::string filename )
{
	auto file = std::ofstream(filename.c_str(), std::ios::app);
	if (!file.is_open())
		return;

	put(file);

	file.close();
}

void Sudoku::draw()
{
	for (Cell *c: *this) c->draw();
}

void Sudoku::draw_spec()
{
	con.SetTextColor(Console::Cyan);
	con.SetCursorPos(MNUX + MNUW - 9, MNUY + 3);
	printf("%4d:%2d:%d", rating, len(), level);

	mnu[0].idx = help;  mnu[0].draw();
	mnu[1].idx = level; mnu[1].draw();
}

void Sudoku::update()
{
	con.DrawColor(MNUX, MNUY, MNUW, 3, wait ? Console::LightRed : solved() ? Console::LightBlue : Console::Green);

	for (Cell *c: *this)
		c->update(Button::button, help);

	for (Button b: btn)
		b.update();

	for (Menu m: mnu)
		m.update();

	int l = len(Button::button);
	if (Button::button > 0 && l < 9)
		con.Put(MNUX, MNUY + 3, '0' + l, Console::Yellow);
	else
		con.Put(MNUX, MNUY + 3, ' ');
}

void Sudoku::back()
{
	if (!undo.empty())
	{
		undo.back().cell->num = undo.back().num;
		undo.pop_back();
	}
	else
	{
		again();
		specify();
	}
}

void Sudoku::game()
{
	con.DrawSingle(TABX, TABY, TABW, TABH);
	con.DrawSingle(TABX, TABY + (TABH - 1) / 3, TABW, (TABH - 1) / 3 + 1);
	con.DrawSingle(TABX + (TABW - 1) / 3, TABY, (TABW - 1) / 3 + 1, TABH);
	con.DrawSingle(BARX, BARY,  BARW, BARH);

	con.DrawBold(MNUX, MNUY, MNUW, 3);

	con.Put(MNUX + (MNUW - strlen(title)) / 2,  MNUY + 1, title);

	for (Button b: btn)
		b.draw();
	
	for (Menu m: mnu)
		m.draw();

	generate();
	draw();
	draw_spec();
	update();

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

					if (x > TABX && x < TABX + TABW - 1 && y > TABY && y < TABY + TABH - 1)
					{
						x -= TABX; y -= TABY;
						if (x % 2 != 0 || x % 8 == 0 || y % 4 == 0)
							break;

						x = (x / 2) - (x / 8) - 1;
						y = y - (y / 4) - 1;
						Cell *c = data()[y * 9 + x];

						switch (input.Event.MouseEvent.dwButtonState)
						{
						case FROM_LEFT_1ST_BUTTON_PRESSED:
							c->set(Button::button, true);
							c->draw();
							break;
						case RIGHTMOST_BUTTON_PRESSED:
							c->set(0, true);
							c->draw();
							break;
						}

						draw_spec();
					}
					else
					if (x >= BARX && x < BARX + BARW && y >= BARY && y < BARY + BARH && !solved())
					{
						x -= BARX; y -= BARY;

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
					if (x >= MNUX && x < MNUX + MNUW)
					{
						x -= MNUX; y -= MNUY;

						if (input.Event.MouseEvent.dwButtonState != FROM_LEFT_1ST_BUTTON_PRESSED)
							break;

						wait = true;
						update();

						switch (y)
						{
						case  4: help  = mnu[0].next(); break;
						case  5: level = mnu[1].next(); /* falls through */
						case  6: generate(); draw(); Button::button = 0; break;
						case  7: clear();    draw(); Button::button = 0; break;
						case  8: confirm();  break;
						case  9: back();     draw(); break;
						case 10: solve();    draw(); Button::button = 0; break;
						case 11: return;
						}

						draw_spec();
						wait = false;
					}
					break;

				case MOUSE_MOVED:

					Menu::menu = 0;
					Button::menu = 0;
					if (x >= BARX && x < BARX + BARW && y >= BARY && y < BARY + BARH && !solved())
						Button::menu = y - BARY;
					else
					if (x >= MNUX && x < MNUX + MNUW && y >= MNUY && y < MNUY + MNUH)
					{
						
						Menu::menu = y - MNUY;
						Menu::back = x < MNUX + MNUW / 2;
					}
					break;

				case MOUSE_WHEELED:

					if (x >= TABX && x < BARX + BARW && y >= BARY && y < BARY + BARH && !solved())
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
					switch (input.Event.KeyEvent.wVirtualKeyCode)
					{
					case VK_ESCAPE: return;
					case VK_CANCEL: return;
					case 'S'      : save("sudoku.board"); break;
					case 'C'      : squeeze(); draw(); draw_spec(); break;
					}
				}
				break;
			}

			if (solved())
				Button::button = 0;

			update();
		}
	}
}

bool Sudoku::test( bool all )
{
	if (rating == -2) { std::cerr << "ERROR: unsolvable" << std::endl; return false; }
	if (rating == -1) { std::cerr << "ERROR: ambiguous"  << std::endl; return false; }

	return level == 0 || all || difficult();
}

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

Base::Base( Sudoku &sudoku )
{
	level     = sudoku.level;
	len       = sudoku.len();
	rating    = sudoku.rating;
	signature = sudoku.signature;
	for (Cell *c: sudoku) data[c->pos] = c->num;
}

void Base::put()
{
	auto sudoku = Sudoku(level);

	sudoku.rating    = rating;
	sudoku.signature = signature;
	for (Cell *c: sudoku) c->num = data[c->pos];
	sudoku.put();
}

bool select_rating( Base &a, Base &b )
{
	return a.rating    > b.rating ||
	       a.rating   == b.rating &&
	      (a.len       < b.len    ||
	       a.len      == b.len    &&
	      (a.level     > b.level  ||
	       a.level    == b.level  &&
	       a.signature < b.signature));
}

bool select_length( Base &a, Base &b )
{
	return a.len       < b.len    ||
	       a.len      == b.len    &&
	      (a.rating    > b.rating ||
	       a.rating   == b.rating &&
	      (a.level     > b.level  ||
	       a.level    == b.level  &&
	       a.signature < b.signature));
}

int main( int argc, char **argv )
{
	int  cnt  = 0;
	char cmd  = 'g';
	auto file = std::string(*argv) + ".board";

	if (--argc > 0 && (**++argv == '/' || **argv == '-'))
		cmd = *++*argv;

	con.HideCursor();

	switch (std::toupper(cmd))
	{
		case 'C': //check
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				sudoku.load(*++argv);

			std::cerr << title << " check: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				std::cerr << ++cnt << '\r';
				sudoku.init(i);
				sudoku.squeeze();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(true))
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), ::select_rating);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " check: " << data.size() << " boards found" << std::endl;
			break;
		}

		case 'S': // sort
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				sudoku.load(*++argv);

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

			std::sort(coll.begin(), coll.end(), std::islower(cmd) ? ::select_rating : ::select_length);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " sort: " << data.size() << " boards found" << std::endl;
			break;
		}

		case 'T': // test
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			while (--argc > 0)
				sudoku.load(*++argv);

			std::cerr << title << " test: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				std::cerr << ++cnt << '\r';
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test())
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), ::select_rating);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " test: " << data.size() << " boards found" << std::endl;
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();

			if (--argc > 0)
				file = *++argv;

			std::cerr << title << " find" << std::endl;

			while (!kbhit() || getch() != 27)
			{
				sudoku.generate();
				if (sudoku.level == 2) sudoku.squeeze();
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test(std::isupper(cmd)))
				{
					data.push_back(sudoku.signature);
					sudoku.put();
					sudoku.save(file);
				}
			}

			std::cerr << title << " find: " << data.size() << " boards found" << std::endl;
			break;
		}

		case 'G': // game
		{
			auto sudoku = Sudoku();

			while (--argc > 0)
				sudoku.load(*++argv);

			int font = con.GetFontSize();
			con.SetFontSize(48);
			con.Maximize();

			sudoku.game();

			con.SetFontSize(font);
			con.Maximize();

			break;
		}

		case '?': /* falls through */
		case 'H': // help
		{
			std::cerr << title << ": help"           << std::endl
			          << "Usage:"                    << std::endl
			          << "sudoku  /s [file] - sort by rating / length" << std::endl
			          << "sudoku  /S [file] - sort by length / rating" << std::endl
			          << "sudoku  /t [file] - test"  << std::endl
			          << "sudoku  /f [file] - find"  << std::endl
			          << "sudoku [/g]       - game"  << std::endl
			          << "sudoku  /h        - help"  << std::endl
			          << "sudoku  /?        - help"  << std::endl;
			break;
		}

		default:
		{
			std::cerr << title << ": unknown command" << std::endl;
			break;
		}
	}

	con.ShowCursor();
}
