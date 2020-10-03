/******************************************************************************

   @file    sudoku.hpp
   @author  Rajmund Szymanski
   @date    03.10.2020
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

#include <list>
#include <array>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>

struct Cell;
struct Sudoku;

static
auto rnd = std::mt19937_64{std::random_device{}()};

struct Cell
{
	int  pos{0};
	int  num{0};
	bool immutable{false};

	std::vector<Cell *> lst{};
	std::vector<Cell *> row{};
	std::vector<Cell *> col{};
	std::vector<Cell *> seg{};

	struct Values: public std::array<int, 10>
	{
		Values( Cell *cell )
		{
		 	std::iota(Values::begin(), Values::end(), 0);

			Values::data()[cell->num] = 0;
			for (Cell *c: cell->lst)
				Values::data()[c->num] = 0;
		}

		int len()
		{
			return std::count_if(Values::begin(), Values::end(), []( int v ){ return v != 0; });
		}

		Values &shuffled()
		{
			std::shuffle(Values::begin(), Values::end(), ::rnd);
			return *this;
		}
	};

	void init( std::array<Cell, 81> &sudoku )
	{
		int tr = Cell::pos / 9;
		int tc = Cell::pos % 9;
		int ts = (tr / 3) * 3 + (tc / 3);

		for (Cell &c: sudoku)
		{
			if (&c == this)
				break;

			int cr = c.pos / 9;
			int cc = c.pos % 9;
			int cs = (cr / 3) * 3 + (cc / 3);

			if (cr == tr || cc == tc || cs == ts) { Cell::lst.push_back(&c); c.lst.push_back(this); }
			if (cr == tr)                         { Cell::row.push_back(&c); c.row.push_back(this); }
			if             (cc == tc)             { Cell::col.push_back(&c); c.col.push_back(this); }
			if                         (cs == ts) { Cell::seg.push_back(&c); c.seg.push_back(this); }
		}
	}

	int len()
	{
		if (Cell::num != 0)
			return 0;

		return Cell::Values(this).len();
	}

	int range()
	{
		if (Cell::num != 0)
			return 0;

		return std::accumulate(std::begin(Cell::lst), std::end(Cell::lst), Cell::len(), []( int r, Cell *c ){ return r + c->len(); });
	}

	bool equal( int n )
	{
		return Cell::num != 0 && Cell::num == n;
	}

	bool dummy()
	{
		return Cell::num == 0 && Cell::len() == 0;
	}

	bool convergent()
	{
		return std::none_of(std::begin(Cell::lst), std::end(Cell::lst), []( Cell *c ){ return c->dummy(); });
	}

	bool allowed( int n )
	{
		if (Cell::num != 0 || n == 0)
			return false;

		if (std::any_of(std::begin(Cell::lst), std::end(Cell::lst), [=]( Cell *c ){ return c->num == n; }))
			return false;

		Cell::num = n;
		bool result = Cell::convergent();
		Cell::num = 0;

		return result;
	}

	static
	bool evident( std::vector<Cell *> &lst, int n )
	{
		return std::none_of(std::begin(lst), std::end(lst), [=]( Cell *c ){ return c->allowed(n); });
	}

	int sure( int n = 0 )
	{
		if (Cell::num == 0 && n == 0)
		{
			for (int v: Cell::Values(this))
				if (v != 0 && Cell::sure(v))
					return v;

			return 0;
		}

		if (!Cell::allowed(n))      return 0;
		if ( Cell::len() == 1)      return n;
		if ( Cell::evident(row, n)) return n;
		if ( Cell::evident(col, n)) return n;
		if ( Cell::evident(seg, n)) return n;

		return 0;
	}

	void clear()
	{
		Cell::num = 0;
		Cell::immutable = false;
	}

	bool set( int n )
	{
		if (!Cell::allowed(n) && (n != 0 || Cell::immutable))
			return false;

		Cell::num = n;
		return true;
	}

	static
	bool select_ptr( Cell *a, Cell *b )
	{
		return a->num == 0 && (b->num != 0          ||
		                       a->len()  < b->len() ||
		                      (a->len() == b->len() && a->range() < b->range()));
	}

	static
	bool select_ref( Cell &a, Cell &b )
	{
		return a.num == 0 && (b.num != 0         ||
		                      a.len()  < b.len() ||
		                     (a.len() == b.len() && a.range() < b.range()));
	}

	friend
	std::ostream &operator <<( std::ostream &out, Cell &cell )
	{
		out << (cell.immutable ? ".123456789" : ".ABCDEFGHI")[cell.num];

		return out;
	}
};

struct Sudoku: std::array<Cell, 81>
{
	int      level;
	int      rating;
	uint32_t signature;

	std::list<std::pair<Cell *, int>> mem;

	static const
	std::vector<std::string> extreme;

	struct Backup: std::array<std::pair<int, bool>, 81>
	{
		Sudoku *tmp;

		Backup( Sudoku *sudoku ): tmp{sudoku} { Backup::reload(); }

		void reload()
		{
			std::transform(std::begin(*tmp), std::end(*tmp), Backup::begin(), []( Cell &c ){ return std::make_pair(c.num, c.immutable); });
		}

		void restore()
		{
			for (Cell &c: *tmp)
			{
				std::pair<int, bool> &t = Backup::data()[c.pos];
				c.num = std::get<int>(t);
				c.immutable = std::get<bool>(t);
			}
		}

		bool reset()
		{
			return std::all_of(std::begin(*tmp), std::end(*tmp), [this]( Cell &c ){ return c.set(std::get<int>(Backup::data()[c.pos])); });
		}

		bool changed()
		{
			return std::any_of(std::begin(*tmp), std::end(*tmp), [this]( Cell &c ){ return c.num != std::get<int>(Backup::data()[c.pos]); });
		}

		int len()
		{
			return std::count_if(Backup::begin(), Backup::end(), []( std::pair<int, bool> &t ){ return std::get<int>(t) != 0; });
		}
	};

	struct Temp: public Sudoku::Backup
	{
		Temp( Sudoku *sudoku ): Sudoku::Backup(sudoku) {}
		~Temp() { Sudoku::Backup::restore(); }
	};

	struct Shadow: std::array<Cell *, 81>
	{
		Shadow( Sudoku *sudoku )
		{
			std::transform(std::begin(*sudoku), std::end(*sudoku), Shadow::begin(), []( Cell &c ){ return &c; });
		}

		void shuffle()
		{
			std::shuffle(Shadow::begin(), Shadow::end(), ::rnd);
		}
	};

	Sudoku( int l = 0 ): level{l}, rating{0}, signature{0}, mem{}
	{
		for (int i = 0; i < 81; i++)
		{
			Cell &c = Sudoku::data()[i];
			c.pos = i;
			c.init(*this);
		}
	}

	int len()
	{
		return std::count_if(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.num != 0; });
	}

	int count( int n )
	{
		return std::count_if(Sudoku::begin(), Sudoku::end(), [=]( Cell &c ){ return c.num == n; });
	}

	bool empty()
	{
		return std::all_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.num == 0; });
	}

	bool solved()
	{
		return std::all_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.num != 0; });
	}

	bool convergent()
	{
		return std::none_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.dummy(); });
	}

	bool tips()
	{
		return std::any_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.sure(0) != 0; });
	}

	void set( Cell &cell, int n )
	{
		int t = cell.num;

		if (cell.set(n))
			Sudoku::mem.emplace_back(&cell, t);
	}

	void clear( bool deep = true )
	{
		for (Cell &c: *this)
			c.clear();

		if (deep)
		{
			Sudoku::rating = 0;
			Sudoku::signature = 0;
			if (Sudoku::level > 0 && Sudoku::level < 4)
				Sudoku::level = 1;
		}
	}

	void discard()
	{
		for (Cell &c: *this)
			c.immutable = false;
	}

	void confirm()
	{
		for (Cell &c: *this)
			c.immutable = c.num != 0;

		Sudoku::specify();

		Sudoku::mem.clear();
	}

	void init( std::string txt )
	{
		Sudoku::clear();

		for (Cell &c: *this)
		{
			if (c.pos < static_cast<int>(txt.size()))
			{
				unsigned char x = txt[c.pos] - '0';
				c.set(x <= 9 ? x : 0);
			}
		}

		Sudoku::confirm();

		for (Cell &c: *this)
		{
			if (c.pos < static_cast<int>(txt.size()))
			{
				unsigned char x = txt[c.pos] - '@';
				c.set(x <= 9 ? x : 0);
			}
		}
	}

	void again()
	{
		for (Cell &c: *this)
			if (!c.immutable)
				c.num = 0;

		Sudoku::mem.clear();
	}

	void swap_cells( int p1, int p2 )
	{
		std::swap(Sudoku::data()[p1].num,       Sudoku::data()[p2].num);
		std::swap(Sudoku::data()[p1].immutable, Sudoku::data()[p2].immutable);
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

		for (Cell &c: *this)
			c.num = v[c.num];

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
		if (!Sudoku::convergent())
			return false;

		auto tmp = Sudoku::Temp(this);

		Sudoku::clear(false);
		if (!tmp.reset())
			return false;

		return true;
	}

	bool correct()
	{
		if (Sudoku::len() < 17)
			return false;

		auto tmp = Sudoku::Temp(this);

		Sudoku::solve_next();
		return std::all_of(Sudoku::begin(), Sudoku::end(), [this]( Cell &c ){ return Sudoku::generate_next(&c, true) != c.immutable; });
	}

	bool simplify()
	{
		bool result = false;

		bool simplified;
		do
		{
			simplified = false;
			for (Cell &c: *this)
				if (c.num == 0 && (c.num = c.sure(0)) != 0)
					simplified = result = true;
		}
		while (simplified);

		return result;
	}

	bool solve_next( Cell *cell = nullptr, bool check = false )
	{
		if (cell != nullptr)
			cell = *std::min_element(std::begin(cell->lst), std::end(cell->lst), Cell::select_ptr);
		else
		if (cell == nullptr || cell->num != 0)
			cell =  std::min_element(Sudoku::begin(), Sudoku::end(), Cell::select_ref);
		if (cell->num != 0)
			return true;

		for (int v: Cell::Values(cell).shuffled())
		{
			if ((cell->num = v) != 0 && Sudoku::solve_next(cell, check))
			{
				if (check)
					cell->num = 0;

				return true;
			}
		}

		cell->num = 0;
		return false;
	}

	void solve()
	{
		if (Sudoku::solvable())
		{
			Sudoku::solve_next();
			Sudoku::mem.clear();
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
		for (int v: Cell::Values(cell))
		{
			if ((cell->num = v) != 0 && Sudoku::solve_next(cell, true))
			{
				cell->num = num;
				return false;
			}
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

		Sudoku::discard();
		Sudoku::Backup tmp(this);
		Sudoku::Shadow tab(this);

		do
		{
			if (Sudoku::len() > tmp.len())
				tmp.restore();
			else
				tmp.reload();

			bool changed;
			do
			{
				tab.shuffle();
				changed = false;
				for (Cell *c: tab)
					if (Sudoku::check_next(c, true))
						changed = true;
			}
			while (changed);

			do
			{
				tab.shuffle();
				changed = false;
				for (Cell *c: tab)
					if (Sudoku::check_next(c, false))
						changed = true;
			}
			while (changed);

			Sudoku::simplify();
		}
		while (tmp.changed());

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

		for (int v: Cell::Values(cell))
			if ((cell->num = v) != 0 && Sudoku::solve_next(cell, true))
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
			Sudoku::Shadow tab(this);
			tab.shuffle();
			Sudoku::clear();
			Sudoku::solve_next();
			for (Cell *c: tab)
				Sudoku::generate_next(c);
			Sudoku::confirm();
		}
	}

	int rating_next()
	{
		std::vector<std::pair<Cell *, int>> sure;
		for (Cell &c: *this)
		{
			if (c.num == 0)
			{
				int n = c.sure();
				if (n != 0)
					sure.emplace_back(&c, n);
				else
				if (c.len() < 2) // wrong way
					return 0;
			}
		}

		if (!sure.empty())
		{
			int  result  = 0;
			bool success = true;
			for (std::pair<Cell *, int> &p: sure)
				if (!std::get<Cell *>(p)->set(std::get<int>(p)))
					success = false;
			if (success)
				result = Sudoku::rating_next() + 1;
			for (std::pair<Cell *, int> &p: sure)
				std::get<Cell *>(p)->num = 0;
			return result;
		}
			
		Cell *cell = std::min_element(Sudoku::begin(), Sudoku::end(), Cell::select_ref);
		if (cell->num != 0) // solved!
			return 1;

		int len    = cell->len();
		int range  = cell->range();
		int result = 0;
		for (Cell &c: *this)
		{
			if (c.num == 0 && c.len() == len && c.range() == range)
			{
				int r = 0;
				for (int v: Cell::Values(&c))
				{
					if (v != 0 && c.set(v))
					{
						r += Sudoku::rating_next();
						c.num = 0;
					}
				}
				if (result == 0 || r < result)
					result = r;
			}
		}

		return result + 1;
	}

	void rating_calc()
	{
		if (!Sudoku::solvable()) { Sudoku::rating = -2; return; }
		if (!Sudoku::correct())  { Sudoku::rating = -1; return; }

		Sudoku::rating = 0;
		int msb = 0;
		int result = Sudoku::rating_next();
		for (int i = Sudoku::count(0); result > 0; Sudoku::rating += i--, result >>= 1)
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

	template <size_t N>
	uint32_t crc32( const std::array<uint32_t, N> &data, uint32_t crc = 0 )
	{
		#define POLY 0xEDB88320

		crc = ~crc;
		for (uint32_t x: data)
		{
			crc ^= x;
			for (size_t i = 0; i < sizeof(x) * CHAR_BIT; i++)
				crc = (crc & 1) ? (crc >> 1) ^ POLY : (crc >> 1);
		}
		crc = ~crc;

		return crc;
	}

	void signature_calc()
	{
		std::array<uint32_t, 10> x = { 0 };
		std::array<uint32_t, 81> t;

		for (Cell &c: *this)
		{
			x[c.num]++;
			t[c.pos] = static_cast<uint32_t>(c.range());
		}

		std::sort(std::begin(x), std::end(x));
		std::sort(std::begin(t), std::end(t));

		Sudoku::signature = Sudoku::crc32(x);
		Sudoku::signature = Sudoku::crc32(t, Sudoku::signature);
	}

	void specify()
	{
		Sudoku::rating_calc();
		Sudoku::level_calc();
		Sudoku::signature_calc();
	}

	void undo()
	{
		if (!Sudoku::mem.empty())
		{
			std::get<Cell *>(Sudoku::mem.back())->num = std::get<int>(Sudoku::mem.back());
			Sudoku::mem.pop_back();
		}
		else
		{
			Sudoku::again();
			Sudoku::specify();
		}
	}

	bool test( bool all, int threshold = 0 )
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

		return Sudoku::level == 0 || all || Sudoku::expected(threshold);
	}

	bool expected( int threshold )
	{
		return Sudoku::rating - Sudoku::len() * 20 >= threshold;
	}

	static
	bool select_threshold( Sudoku &a, Sudoku &b )
	{
		int a_len = a.len();
		int b_len = b.len();
		int a_thr = a.rating - a_len * 20;
		int b_thr = b.rating - b_len * 20;

		return a_thr  > b_thr ||
		      (a_thr == b_thr && (a_len  < b_len ||
		                         (a_len == b_len && (a.level  > b.level ||
		                                            (a.level == b.level && a.signature < b.signature)))));
	}

	static
	bool select_rating( Sudoku &a, Sudoku &b )
	{
		int a_len = a.len();
		int b_len = b.len();

		return a.rating  > b.rating ||
		      (a.rating == b.rating && (a_len  < b_len ||
		                               (a_len == b_len && (a.level  > b.level ||
		                                                  (a.level == b.level && a.signature < b.signature)))));
	}

	static
	bool select_length( Sudoku &a, Sudoku &b )
	{
		int a_len = a.len();
		int b_len = b.len();

		return a_len  < b_len ||
		      (a_len == b_len && (a.rating  > b.rating ||
		                         (a.rating == b.rating && (a.level  > b.level ||
		                                                  (a.level == b.level && a.signature < b.signature)))));
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
		for (Cell &c: sudoku)
			out << c;
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
		auto file = std::ofstream(filename, filename == "sudoku.board" ? std::ios::out : std::ios::app);
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
			if (line.size() == 0)
				continue;

			int l = line.find("\"", 1) - 1;

			if (l < 0 || l > 81 || line.at(0) != '\"')
				std::cerr << "ERROR: incorrect board entry" << std::endl;
			else
				lst.push_back(line.substr(1, l));
		}

		file.close();
	}
};

const
std::vector<std::string> Sudoku::extreme =
{
".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..",//3:21:702:9cd895a7
"4.....8.5.3..........7......2.....6.....8.4...4..1.......6.3.7.5.32.1...1.4......",//3:20:666:37bf7303
"52...6.........7.131..........4..8..6......5...........418.........3..28.387.....",//3:20:666:9a63a17b
".9............15...68........2.5.4...5.8...9........5....64.185....75...4.5...967",//3:24:663:75fbf454
"7.48..............328...16....2....15.......8....93........6.....63..5...351.2...",//3:22:642:78d35561
"52.....8...........1....7.575694......467...............8.1..29.6...24.......9..8",//3:23:630:1304dc2d
"6.....8.3.4.7.................5.4.7.3.42.1...1.6.......2.....5.....8.6...6..1....",//3:20:617:b18cae9c
"...2.8.1..4.3.18............94.2...56.7.5..8.1........7.6...35......7..44........",//3:23:584:b52495ca
"2.....31..9.3.......35.64..721.........1.3.7....7.4....18.....5....3.6..........8",//3:23:584:c8a9dcea
"..7.........9....384..1..2..7....2..36....7.......7.8.......94.18..4...2.....216.",//3:23:583:068b3d5a
".56....82..........28...1.6....56.....5..13....14.........1...8.....2..7.7.59.4..",//3:23:583:44a09195
".9............15...68........2.5.4...5.8...9........5....649185...1.....4.....967",//3:23:583:a672ceba
"4.....3.8...8.2...8..7.....2..1...8734.......6........5.4.6.8......184...82......",//3:23:583:ec17c195
"48.3............7112.......7.5....6....2..8.............1.76...3.....4......53...",//3:19:576:4a51f480
".923.........8.1...........1.7.4...........658.6.......6.5.2...4.....7.....9.4...",//3:19:575:ae172f02
".68.......52..7..........845..3...9..7...5...1..............5.78........3..4..2.8",//3:20:567:020b33c2
"4.....8.5.3........5.7......2.....6.....5.4......1.......693.71..32.1...1.9......",//3:21:560:fe155cd6
".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....",//3:21:556:bd715305
"..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...",//3:21:555:24160b86
"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..",//3:21:555:a331b75e
"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1",//3:21:555:d35727f5
".1..6..9...795.......32..4.....42.3...9...8.............8..6..1.2..3.7..4........",//3:21:555:f241954d
"6....5....9....4.87..2............1..1....764....1.8.9.....2....4.6.....38.5.....",//3:21:555:f33dc0aa
"....14....3....2...7..........9...3.6.1.............8.2.....1.4....5.6.8...7.8...",//3:18:533:58b23d93
"3...8.......7....51.......3......36...2..4....7...........6.13..452...........85.",//3:19:522:6326fa49
"......5..........39..64......8.7......3.....2....6..4.67.....9......58..48...6...",//3:19:522:f8c281c3
"...5.1....9....8...6.......4.1..........7..9........3.8.....1.5...21.4.3.1.36....",//3:20:518:ef9f6e72
".98.1....2......6.............3.2.5..84.........6.4.......4.8.93..5.....8.....1.5",//3:20:513:14de7966
".7...15..63..4...........8......7.3...5....4......96.....8..9..2...6...1....5...8",//3:20:513:66750dfe
".26.........6....3.74.8.........3..2.8..4..1.6..5.........1.78.5....9..........4.",//3:20:513:f890d4a7
};
