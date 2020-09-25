/******************************************************************************

   @file    sudoku.hpp
   @author  Rajmund Szymanski
   @date    25.09.2020
   @brief   sudoku class: generator and solver

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

#pragma once

#include <bits/stdc++.h>

struct Cell;
struct Sudoku;

static
auto rnd = std::mt19937_64(std::time(nullptr));

static
auto undo = std::list<std::pair<Cell *, int>>();

struct CRC32
{
	template<class T>
	unsigned operator()( const T data, unsigned crc )
	{
		return CRC32::calc(&data, sizeof(data), crc);
	}

	template<class T>
	unsigned operator()( const T *data, size_t size, unsigned crc )
	{
		return CRC32::calc(data, size * sizeof(T), crc);
	}

	private:
	unsigned calc( const void *data, size_t size, unsigned crc )
	{
		#define POLY 0xEDB88320
		const unsigned char *buffer = reinterpret_cast<const unsigned char *>(data);

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
};

struct Cell
{
	unsigned pos;
	int      num;
	bool     immutable;
	std::pair<int, bool> tmp;

	struct Value: public std::array<int, 10>
	{
		Value( Cell *cell )
		{
		 	std::iota(Value::begin(), Value::end(), 0);

			Value::data()[cell->num] = 0;

			for (Cell *c: cell->lst)
				Value::data()[c->num] = 0;
		}

		int len()
		{
			int result = 0;

			for (int v: *this)
				if (v != 0)
					result++;

			return result;
		}

		void shuffle()
		{
			std::shuffle(Value::begin(), Value::end(), ::rnd);
		}
	};

	Cell( int p, int n = 0 ): pos(p), num(n), immutable(false), tmp(std::pair<int, bool>{ 0, false }) {}

	std::vector<Cell *> lst;
	std::vector<Cell *> row;
	std::vector<Cell *> col;
	std::vector<Cell *> seg;

	void link( std::vector<Cell *> &tab )
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

	int len()
	{
		if (Cell::num != 0)
			return 0;

		Value val(this);
		return val.len();
	}

	int range()
	{
		if (Cell::num != 0)
			return 0;

		int result = Cell::len();

		for (Cell *c: Cell::lst)
			result += c->len();

		return result;
	}

	bool equal( int n )
	{
		return Cell::num != 0 && Cell::num == n;
	}

	bool dummy()
	{
		return Cell::num == 0 && Cell::len() == 0;
	}

	static
	bool convergent( std::vector<Cell *> &lst )
	{
		for (Cell *c: lst)
			if (c->dummy())
				return false;

		return true;
	}

	bool allowed( int n )
	{
		if (Cell::num != 0 || n == 0)
			return false;

		for (Cell *c: Cell::lst)
			if (c->num == n)
				return false;

		Cell::num = n;
		bool result = Cell::convergent(Cell::lst);
		Cell::num = 0;

		return result;
	}

	static
	bool allowed( std::vector<Cell *> &lst, int n )
	{
		for (Cell *c: lst)
			if (c->allowed(n))
				return true;

		return false;
	}

	int sure( int n = 0 )
	{
		if (Cell::num == 0 && n == 0)
		{
			Value val(this);

			for (int v: val)
				if (v != 0 && Cell::sure(v))
					return v;

			return 0;
		}

		if (!Cell::allowed(n))      return 0;
		if ( Cell::len() == 1)      return n;
		if (!Cell::allowed(row, n)) return n;
		if (!Cell::allowed(col, n)) return n;
		if (!Cell::allowed(seg, n)) return n;

		return 0;
	}

	void clear()
	{
		Cell::num = 0;
		Cell::immutable = false;
	}

	bool set( int n, bool real = false )
	{
		if (!Cell::allowed(n) && (n != 0 || Cell::immutable))
			return false;

		if (real)
			::undo.emplace_back(this, Cell::num);

		Cell::num = n;
		return true;
	}

	void reload()
	{
		Cell::tmp = std::pair<int, bool>{ Cell::num, Cell::immutable };
	}

	void restore()
	{
		Cell::restore(Cell::tmp);
	}

	void restore( std::pair<int, bool> old )
	{
		Cell::num = std::get<int>(old);
		Cell::immutable = std::get<bool>(old);
	}

	bool reset()
	{
		return Cell::set(std::get<int>(Cell::tmp));
	}

	bool changed()
	{
		return Cell::num != std::get<int>(Cell::tmp);
	}

	static
	bool select_cell( Cell *a, Cell *b )
	{
		return a->num == 0 && (b->num != 0          ||
		                       a->len()  < b->len() ||
		                      (a->len() == b->len() && a->range() < b->range()));
	}

	friend
	std::ostream &operator <<( std::ostream &out, Cell &cell )
	{
		out << (cell.immutable ? ".123456789" : ".ABCDEFGHI")[cell.num];
		return out;
	}
};

struct Sudoku: public std::vector<Cell *>
{
	static const std::vector<std::string> extreme;

	int      level;
	int      rating;
	unsigned signature;

	struct Temp
	{
		Sudoku *tmp;

		Temp( Sudoku *sudoku ): tmp(sudoku) { tmp->reload();  }
		~Temp()                             { tmp->restore(); }
	};

	Sudoku( int l = 0 ): level(l), rating(0), signature(0)
	{
		for (int i = 0; i < 81; i++)
		{
			Cell *c = new Cell(i);
			Sudoku::emplace_back(c);
			c->link(*this);
		}
	}

	~Sudoku()
	{
		for (int i = 0; i < 81; i++)
		{
			delete Sudoku::data()[i];
			Sudoku::data()[i] = nullptr;
		}
	}

	int len()
	{
		int result = 0;

		for (Cell *c: *this)
			if (c->num != 0)
				result++;

		return result;
	}

	int len( int num )
	{
		int result = 0;

		for (Cell *c: *this)
			if (c->num == num)
				result++;

		return result;
	}

	bool empty()
	{
		for (Cell *c: *this)
			if (c->num != 0)
				return false;

		return true;
	}

	bool solved()
	{
		for (Cell *c: *this)
			if (c->num == 0)
				return false;

		return true;
	}

	bool expected()
	{
		return Sudoku::rating >= (Sudoku::len() - 2) * 25;
	}

	bool tips()
	{
		for (Cell *c: *this)
			if (c->sure(0) != 0)
				return true;

		return false;
	}

	void reload()
	{
		for (Cell *c: *this)
			c->reload();
	}

	void restore()
	{
		for (Cell *c: *this)
			c->restore();
	}

	bool changed()
	{
		for (Cell *c: *this)
			if (c->changed())
				return true;

		return false;
	}

	void clear( bool deep = true )
	{
		for (Cell *c: *this)
			c->clear();

		if (deep)
		{
			Sudoku::rating = Sudoku::signature = 0;
			if (Sudoku::level > 0 && Sudoku::level < 4)
				Sudoku::level = 1;
		}
	}

	void discard()
	{
		for (Cell *c: *this)
			c->immutable = false;
	}

	void confirm()
	{
		for (Cell *c: *this)
			c->immutable = c->num != 0;

		Sudoku::specify();

		::undo.clear();
	}

	void init( std::string txt )
	{
		Sudoku::clear();

		for (Cell *c: *this)
		{
			if (c->pos < txt.size())
			{
				unsigned char x = txt[c->pos] - '0';
				c->set(x <= 9 ? x : 0);
			}
		}

		Sudoku::confirm();

		for (Cell *c: *this)
		{
			if (c->pos < txt.size())
			{
				unsigned char x = txt[c->pos] - '@';
				c->set(x <= 9 ? x : 0);
			}
		}
	}

	void again()
	{
		for (Cell *c: *this)
			if (!c->immutable)
				c->num = 0;

		::undo.clear();
	}

	void swap_cells( int p1, int p2 )
	{
		std::swap(Sudoku::data()[p1]->num,       Sudoku::data()[p2]->num);
		std::swap(Sudoku::data()[p1]->immutable, Sudoku::data()[p2]->immutable);
	}

	void swap_rows( int r1, int r2 )
	{
		r1 *= 9; r2 *= 9;
		for (int c = 0; c < 9; c++)
			Sudoku::swap_cells(r1 + c, r2 + c);
	}

	void swap_cols( int c1, int c2 )
	{
		for (int r = 0; r < 81; r += 9)
			Sudoku::swap_cells(r + c1, r + c2);
	}

	void shuffle()
	{
		int v[10];
	 	std::iota(v, v + 10, 0);
		std::shuffle(v + 1, v + 10, ::rnd);

		for (Cell *c: *this)
			c->num = v[c->num];

		for (int i = 0; i < 81; i++)
		{
			int c1 = ::rnd() % 9;
			int c2 = 3 * (c1 / 3) + (c1 + 1) % 3;
			Sudoku::swap_cols(c1, c2);

			int r1 = ::rnd() % 9;
			int r2 = 3 * (r1 / 3) + (r1 + 1) % 3;
			Sudoku::swap_rows(r1, r2);

			c1 = ::rnd() % 3;
			c2 = (c1 + 1) % 3;
			c1 *= 3; c2 *= 3;
			for (int j = 0; j < 3; j++)
				Sudoku::swap_cols(c1 + j, c2 + j);

			r1 = ::rnd() % 3;
			r2 = (r1 + 1) % 3;
			r1 *= 3; r2 *= 3;
			for (int j = 0; j < 3; j++)
				Sudoku::swap_rows(r1 + j, r2 + j);
		}
	}

	bool solvable()
	{
		if (!Cell::convergent(*this))
			return false;

		auto tmp = Temp(this);

		Sudoku::clear(false);

		for (Cell *c: *this)
			if (!c->reset())
				return false;

		return true;
	}

	bool correct()
	{
		if (Sudoku::len() < 17)
			return false;

		auto tmp = Temp(this);

		Sudoku::solve_next(*this);
		for (Cell *c: *this)
			if (Sudoku::generate_next(c, true) == c->immutable)
				return false;

		return true;
	}

	bool simplify()
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

	bool solve_next( std::vector<Cell *> &lst, bool check = false )
	{
		              Cell *cell = *std::min_element(    lst.begin(),     lst.end(), Cell::select_cell);
		if (cell->num != 0) cell = *std::min_element(Sudoku::begin(), Sudoku::end(), Cell::select_cell);
		if (cell->num != 0) return true;

		Cell::Value val(cell); val.shuffle();

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

	void solve()
	{
		if (Sudoku::solvable())
		{
			Sudoku::solve_next(*this);

			::undo.clear();
		}
	}

	bool check_next( Cell *cell, bool strict )
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
		Cell::Value val(cell);
		for (int v: val)
			if ((cell->num = v) != 0 && Sudoku::solve_next(cell->lst, true))
			{
				cell->num = num;
				return false;
			}

		cell->num = 0;
		return true;
	}

	void check()
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
			if (Sudoku::len() > len)
				Sudoku::restore();
			else
			{
				Sudoku::reload();
				len = Sudoku::len();
			}

			bool changed;
			do
			{
				changed = false;
				std::shuffle(tab.begin(), tab.end(), ::rnd);
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
				std::shuffle(tab.begin(), tab.end(), ::rnd);
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

	bool generate_next( Cell *cell, bool check = false )
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

		Cell::Value val(cell);
		for (int v: val)
			if ((cell->num = v) != 0 && Sudoku::solve_next(cell->lst, true))
			{
				cell->num = num;
				return false;
			}

		cell->num = 0;
		return true;
	}

	void generate()
	{
		if (Sudoku::level == 4)
		{
			Sudoku::init(Sudoku::extreme[rnd() % Sudoku::extreme.size()]);
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

	int rating_next()
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
				if (!std::get<Cell *>(p)->set(std::get<int>(p)))
					success = false;
			if (success)
				result = Sudoku::rating_next() + 1;
			for (std::pair<Cell *, int> p: sure)
				std::get<Cell *>(p)->num = 0;
			return result;
		}
			
		Cell *cell = *std::min_element(Sudoku::begin(), Sudoku::end(), Cell::select_cell);
		if (cell->num != 0) // solved!
			return 1;

		int len    = cell->len();
		int range  = cell->range();
		int result = 0;
		for (Cell *c: *this)
			if (c->num == 0 && c->len() == len && c->range() == range)
			{
				Cell::Value val(c);
				int r = 0;
				for (int v: val)
					if (v != 0 && c->set(v))
					{
						r += Sudoku::rating_next();
						c->num = 0;
					}
				if (result == 0 || r < result)
					result = r;
			}

		return result + 1;
	}

	void rating_calc()
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
//		Sudoku::rating = Sudoku::rating_next();
}

	void level_calc()
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

	void signature_calc()
	{
		unsigned x[10] = { 0 };
		unsigned t[81];

		for (Cell *c: *this)
		{
			x[c->num]++;
			t[c->pos] = static_cast<unsigned>(c->range());
		}

		std::sort(x, x + 10);
		std::sort(t, t + 81);

		CRC32 crc32;
		Sudoku::signature = crc32(x, 10, 0);
		Sudoku::signature = crc32(t, 81, Sudoku::signature);
	}

	void specify()
	{
		Sudoku::rating_calc();
		Sudoku::level_calc();
		Sudoku::signature_calc();
	}

	void undo()
	{
		if (!::undo.empty())
		{
			std::get<Cell *>(::undo.back())->num = std::get<int>(::undo.back());
			::undo.pop_back();
		}
		else
		{
			Sudoku::again();
			Sudoku::specify();
		}
	}

	bool test( bool all = false )
	{
		if (Sudoku::rating == -2)
		{
			std::cerr << "ERROR: unsolvable" << std::endl;
			return false;
		}

		if (Sudoku::rating == -1)
		{
			std::cerr << "ERROR: ambiguous"  << std::endl;
			return false;
		}

		return Sudoku::level == 0 || all || Sudoku::expected();
	}

	friend
	std::istream &operator >>( std::istream &in, Sudoku &sudoku )
	{
		std::string line;
		if (std::getline(in, line))
		{
			int l = line.find("\"", 1) - 1;

			if (l < 0 || l > 81 || line.at(0) != '\"')
				std::cerr << "ERROR: incorrect board entry" << std::endl;
			else
			{
				sudoku.level = 1;
				sudoku.init(line.substr(1, l));
			}
		}

		return in;
	}

	friend
	std::ostream &operator <<( std::ostream &out, Sudoku &sudoku )
	{
		out << "\"";
		for (Cell *c: sudoku)
			out << *c;
		out << "\",//"      << sudoku.level      << ':'
		    << std::setw(2) << sudoku.len()      << ':'
		    << std::setw(3) << sudoku.rating     << ':'
		    << std::setw(8) << std::setfill('0') << std::hex << sudoku.signature
		                    << std::setfill(' ') << std::dec;
		return out;
	}

	void load( std::string filename = "sudoku.board" )
	{
		auto file = std::ifstream(filename);
		if (!file.is_open())
			return;

		file >> *this;

		file.close();
	}

	void save( std::string filename = "sudoku.board" )
	{
		auto file = std::ofstream(filename);
		if (!file.is_open())
			return;

		file << *this << std::endl;

		file.close();
	}

	static
	void load( std::vector<std::string> &lst, std::string filename )
	{
		auto file = std::ifstream(filename);
		if (!file.is_open())
			return;

		static
		bool done = false;
		if (!done)
		{
			lst.clear();
			done = true;
		}

		std::string line;
		while (std::getline(file, line))
		{
			int l = line.find("\"", 1) - 1;

			if (l < 0 || l > 81 || line.at(0) != '\"')
				std::cerr << "ERROR: incorrect board entry" << std::endl;
			else
				lst.push_back(line.substr(1, l));
		}

		file.close();
	}
};

const std::vector<std::string> Sudoku::extreme =
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
"....7..4...932....8..19...52179.35.45..........87..12............6.3..9.......4..",//3:25:563:27f41223
"8.79.....1.4....9.9...68...........5....3....21.....7.5.2.934.7..1..25.......6..3",//3:25:561:1b6cf7c6
"2.....45.5......9...8........39.8......6.4..34.....9...6..9........26.19921.4.5..",//3:25:561:263a09dd
".3...51.6..14...5...5.3.7...6..4.9..1...5.......2.34.........2..5...76.4.1...4...",//3:25:561:7599e867
".....7.5.1..5.387.5........41...6...9.....2....3459.1...1.6..3.8.......63.....1..",//3:25:561:99af7cac
"...4269......31..6........19.8...247.....2.5.32......8.7..5....8.61......3...4...",//3:25:561:a0fe715f
".....8416.154...9.4...7.5.........3....249......3.5...1.4...2....2...9..8...24...",//3:25:561:a5f4df69
"...8........3.21.9.....9823346...9..............53.61............8421.9629.......",//3:25:561:bbb16ab8
"84.9.12....2.83..99......8....1..9.......8..26..7..1...9.217......8.....7.......5",//3:25:561:c7c96e35
"..6......3.9...57..7....9.39.4..27....18..........6....92154......2....9.1..8..5.",//3:25:561:f74085ce
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
"..5........8.645..6..8...1..9....7....3.4...2....2.839..9..........8..2...71.3...",//3:23:535:0df29dff
".74..9....6..4...5...6..4.....37...2....916.3.3......4352..................1..38.",//3:23:535:0e39d71f
"8...4...6...36..9.165...........7...354.........9..8..5...2...4..6.5..83......5..",//3:23:535:11972b92
"4..81....6.5......218.3.....5....8........9.4....9...78..679..1......4....6..3...",//3:23:535:25706a10
".7.....53......4.....6......9.47.1...3...1.4....5...9.9....2.848.......2..6..5..7",//3:23:535:29bd817c
".79..4.36...6...4...............9..5..6...21..1...8...79.5..3...4.......3.82..9..",//3:23:535:2a16dbd7
"....4........9...763....1.......6...86.....7......1864......7....5..2..992...45.8",//3:23:535:33eca3a1
"..........29.....88...9.3.6....19....7...2.........8546..327..........3.5.394....",//3:23:535:41de2991
"51.......3..69.57..4...2......8.........4...26...1.9...5....3.9.6.......13.4....8",//3:23:535:513b6972
"...19..5.6....3........8.4.8.....7....98..51.........4..1...2.5..25.6..3....3...8",//3:23:535:6248d6ae
".1...6....8.1..3..5..3.........31..9..1.4..5..6......4..4.7...8...2.......24.9.7.",//3:23:535:6a3f358d
"4.6......8..2...75...3..4...5.8..2.......6....4...2..7...72.513.....3.....3..8...",//3:23:535:97a8cf42
"4.6......2..8..5..1......9.8..1.63.53....9..45....8......28...3......1....4.7....",//3:23:535:990119ab
"....8..6..6...27..7...6...9.39.......563.....2.......6...1....3......14.81..9...2",//3:23:535:b77c2e52
"2...4.6..........9.5..7.....18...9...2.6..83.....8.2.78.....4.3.........3.4.2..8.",//3:23:535:b7b447bb
"..7..........738.45.46.9.1.7.......24......3...5....9....84.3.5...1.6...8........",//3:23:535:c2b26015
".........471....68.2.........918...3.5..4...2...3........81......4....86....6975.",//3:23:535:c4672754
"7.....3.5....3..6..9...7.....9.....68..5.3.9..4....51.981..2.......9.6..4........",//3:23:535:d0a5cc63
".......6.1....2.7.75.6........16.3......93...3.....1578...21..5...3.......7....4.",//3:23:535:db97b607
"3..2...8......97..2.......6....9...86..58.4..7....1.......1..4..4..3....96...8.5.",//3:23:535:eb3b3f0b
"6.......43.......741.5..3.....6184.........6.........283...65...4.3...7.1...2....",//3:23:535:f009c301
".....2....26.1..5..91...3..6...5.27.1...7.6.....6.....27.8.......5...7........8.9",//3:23:535:f55ccabb
};
