/******************************************************************************

   @file    sudoku.cpp
   @author  Rajmund Szymanski
   @date    04.06.2019
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
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <conio.h>
#include "console.hpp"

#define LENGTH     20
#define RATING    (10 * len())

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
"..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...",//3:21:391:24160b86
".........9..8.2....5.74...312.9...6...5..7.....6...13......658..1....3...8..2..9.",//3:24:384:5fbde9a4
".........61...497..3...5.6.97..318............4.7.8...4.......6....5..3.89...7.1.",//3:24:379:6fb47b6f
".79....6.....5......53.17....6.2....49.....83.......1....8..1.4.....5..8....9..5.",//3:22:376:411408a6
"..4.........1.82...8.2....359............61...4..5..7..178...6.6..71..48.........",//3:23:374:0cc8e545
"..9.84.27.........7.......3..325..1..1..4.6...5....3...9...7...54..6..9.......8..",//3:23:374:1ad485dc
"6........4251...9.8.....241.7..6...8.....5...5...7..343.981.....4...2.8.........3",//3:26:368:9dc56091
"..3.....846...8..9..25...........92.54.829..32.....48..243...7..7..4....3..6.7...",//3:28:362:8a6bbdd1
"..82..7...3..1.....7.9...6......36...6.....4.1.95...2.......5....4......7.58...1.",//3:22:352:889f6a9d
"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1",//3:21:350:d35727f5
"1.8..4..2.273.........8.7....2...9.1.......2.4...5............985...9.6..6..354..",//3:24:348:0fc1405b
"61....4...5........8.21..5...54.69...9.7.....2............89.17.............7..28",//3:22:346:488309b4
".......1.3.8......7.6..28.9....91...57.3.....1...2.6.........4......8....4.1..783",//3:23:345:9912d585
"91.2.......28.........16.......8.4.5..8.3.2.....52..873....574...7...1..6..7.....",//3:25:345:4c9b81ea
"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..",//3:21:341:a331b75e
".7.....15.4.51...36...........49....9...35.....6...2..........2.6...45...2..517.4",//3:24:341:2ab61bd7
"8..5......72.....55312...46.....3..9.9..2..1...3.5..2....4.2..178.1...6..........",//3:26:339:73066669
".1.8..72456.......8.......6.8..4.....4.9.3..5.2...841.....8..4...8..9..1.3.1...6.",//3:27:339:6f9a3ac8
"..3.9...2.298...7.6...1.......5.69....2..8..7.....1.8.3.8.......7...3.14.4.......",//3:24:336:2b38debd
"...7..2..4..2...39....9..1.6....394...1.....6.8.9.....1......63..7......3..82....",//3:23:334:a3875d51
"5.3..8..9......7.....9.2...8..5..3....2.....84....3.6.2......34.1.6.5........95..",//3:23:331:4780c9c4
"...2.6.....23.87...8..9.2.19.......6..1......8.5..1.7..5....94...842..........6.8",//3:25:331:d64896e4
"..1..2.7..4.51...3.......6...7....8..8....6.791...3........5...43....8....2.36...",//3:23:330:7084ad0b
"..1...3.9.......685.3....4......8..26..9......74.3......681...4....7..9......9..6",//3:23:328:8d0964d6
"23....5...9....6...45..82..9..1..........6..2....3.14....84.......65..8185.71..2.",//3:27:328:767536aa
".....17...7...2...91..8..6...9......3..8......27...6.5...9...4...5.4......63..2.8",//3:23:326:09f0555c
".....95..3..8........6.4.98874.12...51.......9..4...8...1..7...4.....65..3.....2.",//3:25:325:22633efc
"...42.8...6...5..7..1...2..3..........7.1...4.243....6.3..8.69..7...9.5....6.....",//3:24:323:21147c76
"...9....6..7.81....1.67.....257...6.....6.9....8.....25..1............3.3..54.2.9",//3:24:323:7c116398
"..6.5.........8..5.4...28.1.6....1721............7..833...8..17...3...4..94.2....",//3:25:323:55d2bc9c
"....5.739...13..4............6.....8.7164....2.....5..7......5..3..14...9.276....",//3:24:322:1359bf93
".....5...3.1..8......6..3.1..5.436......6.4..467......2.....98......412..18...547",//3:27:322:560654d7
};

const char *title = "SUDOKU";

Console con(title);

std::ranlux48 rnd(std::time(nullptr));

#define RND(v) (rnd() % (v))
 
class CRC32
{
	unsigned calc( const void *, size_t, unsigned );
public:
	template<class T>
	unsigned operator()( const T  data,              unsigned crc ) { return calc(&data,        sizeof(T), crc); }
	template<class T>
	unsigned operator()( const T *data, size_t size, unsigned crc ) { return calc( data, size * sizeof(T), crc); }
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
	int  tmp;
	bool immutable;

	Cell( int p, int n = 0 ): pos(p), num(n), tmp(0), immutable(false) {}

	std::vector<Cell *> lst;
	std::vector<Cell *> row;
	std::vector<Cell *> col;
	std::vector<Cell *> seg;

	void clear  ();
	void link   ( std::vector<Cell *> & );
	void swap   ( Cell * );
	int  len    ();
	int  range  ();
	bool equal  ( int );
	bool allowed( int );
	int  sure   ( int = 0 );
	bool set    ( int, bool );
	void put    ( std::ostream & );
	void draw   ();
	void update ( int, int );

	friend
	bool min_length( Cell *, Cell * );
	friend
	bool min_range( Cell *, Cell * );
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

struct Temp: public std::array<int, 81>
{
	std::vector<Cell *> &tmp;
	bool res;

	Temp( std::vector<Cell *> &, bool = true );
	~Temp();

	void restore();
};

Temp::Temp( std::vector<Cell *> &vec, bool restore ): tmp(vec), res(restore)
{
	for (Cell *c: vec)
	{
		data()[c->pos] = c->num;
		if (c->immutable) data()[c->pos] += 256;
	}
}

Temp::~Temp()
{
	if (res)
		restore();
}

void Temp::restore()
{
	for (Cell *c: tmp)
	{
		c->num = data()[c->pos] % 256;
		c->immutable = data()[c->pos] >= 256;
	}
}

void Cell::clear()
{
	num = 0;
	immutable = false;
}

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

void Cell::swap( Cell *cell )
{
	int  n = cell->num;       cell->num = num;             num = n;
	bool i = cell->immutable; cell->immutable = immutable; immutable = i;
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

bool min_length( Cell *a, Cell *b )
{
	return a->num == 0 && (b->num != 0 || a->len() < b->len());
}

bool min_range( Cell *a, Cell *b )
{
	return a->num == 0 && (b->num != 0 || a->range() < b->range());
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
	if (len() == 1)         return n;
	if (!::allowed(row, n)) return n;
	if (!::allowed(col, n)) return n;
	if (!::allowed(seg, n)) return n;

	return 0;
}	

bool Cell::set( int n, bool save )
{
	n %= 256;
	if (!allowed(n) && (n != 0 || immutable))
		return false;
	if (save)
		undo.emplace_back(this);
	num = n;
	return true;
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

	int  len          ( int = 0 );
	bool empty        ();
	bool solved       ();
	bool found        ();
	bool tips         ();

	void confirm      ();
	void init         ( std::string );
	void clear        ( bool = true );
	void again        ();
	void swap_rows    ( int, int );
	void swap_cols    ( int, int );
	void shuffle      ();
	bool solvable     ();
	bool correct      ();
	bool simplify     ();
	bool solve_next   ( std::vector<Cell *> & );
	void solve        ();
	bool solve_test   ( std::vector<Cell *> & );
	bool generate_next( Cell *, bool = false );
	void generate     ( bool = false );
	int  rating_next  ();
	void rating_calc  ();
	void signat_calc  ();
	void level_calc   ();
	void specify      ();
	void put          ( std::ostream & = std::cout );
	void load         ( std::string );
	void save         ( std::string );
	bool test         ();
	void draw         ();
	void draw_spec    ();
	void update       ();
	void back         ();
	void game         ();
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

int Sudoku::len( int n )
{
	int result = 0;
	for (Cell *c: *this)
		if (c->num == n) result++;
	return n ? result : 81 - result;
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

bool Sudoku::found()
{
	return level == 0 || level >= 3 && rating >= RATING;
}

bool Sudoku::tips()
{
	for (Cell *c: *this)
		if (c->sure(0) != 0)
			return true;
	return false;
}

void Sudoku::confirm()
{
	for (Cell *c: *this)
		c->immutable = c->num != 0;
	specify();
	undo.clear();
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

void Sudoku::clear( bool full )
{
	for (Cell *c: *this)
		c->clear();

	if (full)
	{
		rating = signature = 0;
		if (level > 0 && level < 4) level = 1;
	}
}

void Sudoku::again()
{
	for (Cell *c: *this)
		if (c->immutable == false)
			c->num = 0;
	undo.clear();
}

void Sudoku::swap_rows( int r1, int r2 )
{
	r1 *= 9; r2 *= 9;
	for (int c = 0; c < 9; c++)
		data()[r1 + c]->swap(data()[r2 + c]);
}

void Sudoku::swap_cols( int c1, int c2 )
{
	for (int r = 0; r < 81; r += 9)
		data()[r + c1]->swap(data()[r + c2]);
}

void Sudoku::shuffle()
{
	int v[10]; for (int i = 0; i < 10; i++) v[i] = i;
	std::shuffle(v + 1, v + 10, rnd);
	for (Cell *c: *this) c->num = v[c->num];
/*
	for (int i = 0; i < 9; i++)
		for (int j = i + 1; j < 9; j++)
			data()[9 * i + j]->swap(data()[9 * j + i]);
*/
	for (int i = 0; i < 81; i++)
	{
		int p = RND(24);

		if      (p <  9)
		{
			int c1 = p;
			int c2 = 3 * (c1 / 3) + (c1 + 1) % 3;
			swap_cols(c1, c2);
		}
		else if (p < 18)
		{
			int r1 = p - 9;
			int r2 = 3 * (r1 / 3) + (r1 + 1) % 3;
			swap_rows(r1, r2);
		}
		else if (p < 21)
		{
			int c1 = p - 18;
			int c2 = (c1 + 1) % 3;
			c1 *=3; c2 *= 3;
			for (int i = 0; i < 3; i++)
				swap_cols(c1 + i, c2 + i);
		}
		else
		{
			int r1 = p - 21;
			int r2 = (r1 + 1) % 3;
			r1 *= 3; r2 *= 3;
			for (int i = 0; i < 3; i++)
				swap_rows(r1 + i, r2 + i);
		}
	}
}

bool Sudoku::solvable()
{
	Temp tmp(*this);

	clear(false);

	for (Cell *c: *this)
		if (!c->set(tmp[c->pos], false))
			return false;

	return true;
}

bool Sudoku::correct()
{
	if (len() < 17)
		return false;

	Temp tmp(*this);

	solve_next(*this);

	for (Cell *c: *this)
		if (generate_next(c, true) == c->immutable)
			return false;

	return true;
}

bool Sudoku::simplify()
{
	bool result = false;

	again:
	for (Cell *c: *this)
		if (c->num == 0 && (c->num = c->sure(0)) != 0)
		{
			result = true;
			goto again;
		}

	return result;
}

bool Sudoku::solve_next( std::vector<Cell *> &lst )
{
	Cell *cell = *std::min_element(lst.begin(), lst.end(), ::min_length);
	if (cell->num != 0) cell = *std::min_element(begin(), end(), ::min_length);
	if (cell->num != 0) return true;

	Value val(cell); val.shuffle();
	for (int v: val)
		if ((cell->num = v) != 0 && solve_next(cell->lst))
			return true;

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

bool Sudoku::solve_test( std::vector<Cell *> &lst )
{
	Cell *cell = *std::min_element(lst.begin(), lst.end(), ::min_length);
	if (cell->num != 0) cell = *std::min_element(begin(), end(), ::min_length);
	if (cell->num != 0) return true;

	Value val(cell); val.shuffle();
	for (int v: val)
		if ((cell->num = v) != 0 && solve_test(cell->lst))
		{
			cell->num = 0;
			return true;
		}

	cell->num = 0;
	return false;
}

bool Sudoku::generate_next( Cell *cell, bool test )
{
	if (cell->num == 0 || cell->immutable)
		return false;

	int num = cell->num;

	cell->num = 0;
	if (cell->sure(num))
		return true;

	cell->num = num;
	if (level == 0 && !test)
		return false;

	Value val(cell); val.shuffle();
	for (int v: val)
		if ((cell->num = v) != 0 && solve_test(cell->lst))
		{
			cell->num = num;
			return false;
		}

	cell->num = 0;
	return true;
}

void Sudoku::generate( bool force )
{
	if (level == 4)
	{
		init(extreme[RND(extreme.size())]);
		shuffle();
	}
	else
	{
		do
		{
			clear();
			solve_next(*this);
			std::vector<Cell *> tab(*this);
			std::shuffle(tab.begin(), tab.end(), rnd);
			for (Cell *c: tab)
				generate_next(c);
		}
		while (force && level > 0 && !kbhit() && simplify() && solved());
		confirm();
	}
}

int Sudoku::rating_next()
{
	if (solved())
		return 0;

	for (Cell *c: *this)
		if (c->num == 0 && c->sure(c->tmp) != 0)
		{
			c->num = c->tmp;
			int result = rating_next() + 1;
			c->num = 0;
			return result;
		}

	int range  = (*std::min_element(begin(), end(), ::min_range))->range();
	int result = (~0U) >> 1;

	for (Cell *c: *this)
		if (c->num == 0 && c->range() == range)
		{
			c->num = c->tmp;
			int r = rating_next() + 1 + range;
			c->num = 0;
			if (r < result) result = r;
		}

	return result;
}

void Sudoku::rating_calc()
{
	if ( empty())    { rating = -1; return; }
	if ( solved())   { rating = -2; return; }
	if (!solvable()) { rating = -3; return; }
	if (!correct())  { rating = -4; return; }

	solve_next(*this);
	for (Cell *c: *this) c->tmp = c->num;
	again();

	rating = rating_next();
}

void Sudoku::level_calc()
{
	if ( level == 0) {            return; }
	if ( level == 4) {            return; }
	if ( rating < 0) { level = 1; return; }
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
	auto file = std::ifstream(filename.c_str());
	if (!file.is_open())
		return;

	extreme.clear();
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

bool Sudoku::test()
{
	if (rating == -1) { std::cerr << "ERROR: empty"      << std::endl; return false; }
	if (rating == -2) { std::cerr << "ERROR: solved"     << std::endl; return false; }
	if (rating == -3) { std::cerr << "ERROR: unsolvable" << std::endl; return false; }
	if (rating == -4) { std::cerr << "ERROR: ambiguous"  << std::endl; return false; }
	if (!found())     {                                                return false; }

	return true;
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

	friend
	bool sort_rating( Base &, Base & );
	friend
	bool sort_length( Base &, Base & );
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

bool sort_rating( Base &a, Base &b )
{
	return a.rating > b.rating || a.rating == b.rating && (a.len < b.len || a.len == b.len && a.signature < b.signature);
}

bool sort_length( Base &a, Base &b )
{
	return a.len < b.len || a.len == b.len && (a.rating > b.rating || a.rating == b.rating && a.signature < b.signature);
}

int main( int argc, char **argv )
{
	char cmd  = 'g';
	auto file = std::string(*argv) + ".board";

	if (--argc > 0 && (**++argv == '/' || **argv == '-'))
	{
		cmd = *++*argv;
		if (--argc > 0) file = std::string(*++argv);
	}

	switch (std::toupper(cmd))
	{
		case 'S': // sort
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();
			auto coll   = std::vector<Base>();

			sudoku.load(file);

			std::cerr << title << " sort: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end())
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), sort_length);

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

			sudoku.load(file);

			std::cerr << title << " test: " << extreme.size() << " boards loaded" << std::endl;

			for (std::string i: extreme)
			{
				sudoku.init(i);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test())
				{
					data.push_back(sudoku.signature);
					coll.emplace_back(sudoku);
				}
			}

			std::sort(coll.begin(), coll.end(), sort_rating);

			for (Base &base: coll)
				base.put();

			std::cerr << title << " test: " << data.size() << " boards found" << std::endl;
			break;
		}

		case 'F': // find
		{
			auto sudoku = Sudoku(1);
			auto data   = std::vector<unsigned>();

			std::cerr << title << " find" << std::endl;

			while (!kbhit() || getch() != 27)
			{
				sudoku.generate(true);
				if (std::find(data.begin(), data.end(), sudoku.signature) == data.end() && sudoku.test())
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

			int font = con.GetFontSize();
			con.SetFontSize(48);
			con.Maximize();
			con.HideCursor();

			sudoku.game();

			con.SetFontSize(font);
			con.Maximize();

			break;
		}

		case '?': /* falls through */
		case 'H': // help
		{
			std::cerr << title << ": help"             << std::endl
			          << "Usage:"                      << std::endl
			          << "sudoku  /sort [file] - sort" << std::endl
			          << "sudoku  /test [file] - test" << std::endl
			          << "sudoku  /find [file] - find" << std::endl
			          << "sudoku [/game]       - game" << std::endl
			          << "sudoku  /help        - help" << std::endl;
			break;
		}

		default:
		{
			std::cerr << title << ": unknown command" << std::endl;
			break;
		}
	}
}
