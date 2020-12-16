/******************************************************************************

   @file    sudoku.hpp
   @author  Rajmund Szymanski
   @date    10.12.2020
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
#include <tchar.h>

class SudokuCell;
class Sudoku;

using cell_ref = std::reference_wrapper<SudokuCell>;
using cell_array = std::array<SudokuCell, 81>;
using uint = unsigned int;

enum Difficulty
{
	Any = -1,
	Easy = 0,
	Medium,
	Hard,
	Expert,
	Extreme,
};

enum class Force
{
	Direct,
	Careful,
	Safe,
};

class SudokuCell
{
	using Cell = SudokuCell;

public:

	uint pos{0};
	uint num{0};
	bool immutable{false};

	class Values: public std::array<uint, 10>
	{
	public:

		Values( Cell &cell, bool shuffled = false )
		{
		 	std::iota(Values::begin(), Values::end(), 0);

			Values::at(cell.num) = 0;
			for (Cell &c: cell.lst)
				Values::at(c.num) = 0;

			if (shuffled)
				std::shuffle(Values::begin(), Values::end(), std::mt19937{std::random_device{}()});
		}

		uint len()
		{
			return std::count_if(Values::begin(), Values::end(), []( uint v ){ return v != 0; });
		}
	};

private:

	std::vector<cell_ref> row{};
	std::vector<cell_ref> col{};
	std::vector<cell_ref> seg{};
	std::vector<cell_ref> lst{};

	bool in_row( const Cell &c )
	{
		if (Cell::pos == c.pos)
			return false;

		uint r1 = Cell::pos / 9;
		uint r2 = c.pos / 9;

		return r1 == r2;
	}

	bool in_col( const Cell &c )
	{
		if (Cell::pos == c.pos)
			return false;

		uint c1 = Cell::pos % 9;
		uint c2 = c.pos % 9;

		return c1 == c2;
	}

	bool in_seg( const Cell &c )
	{
		if (Cell::pos == c.pos)
			return false;

		uint r1 = Cell::pos / 9;
		uint r2 = c.pos / 9;
		uint c1 = Cell::pos % 9;
		uint c2 = c.pos % 9;
		uint s1 = (r1 / 3) * 3 + (c1 / 3);
		uint s2 = (r2 / 3) * 3 + (c2 / 3);

		return s1 == s2;
	}
	
	bool in_lst( const Cell &c )
	{
		return Cell::in_row(c) || Cell::in_col(c) || Cell::in_seg(c);
	}

	void link( Cell &c )
	{
		if (Cell::in_row(c)) Cell::row.push_back(std::ref(c));
		if (Cell::in_col(c)) Cell::col.push_back(std::ref(c));
		if (Cell::in_seg(c)) Cell::seg.push_back(std::ref(c));
		if (Cell::in_lst(c)) Cell::lst.push_back(std::ref(c));
	}

public:

	bool linked( Cell *c )
	{
		if (c == nullptr)
			return false;

		return in_lst(*c);
	}

	void init( uint p )
	{
		Cell::pos = p;

		std::for_each(this - p, this, [this]( Cell &c )
		{
			Cell::link(c);
			c.link(*this);
		});
	}

	uint len()
	{
		if (Cell::num != 0)
			return 0;

		return Cell::Values(*this).len();
	}

	uint range()
	{
		return std::accumulate(std::begin(Cell::lst), std::end(Cell::lst), 0U, []( uint r, Cell &c ){ return r + c.len(); });
	}

	uint solid()
	{
		if (Cell::num == 0)
			return 0;

		return std::count_if(std::begin(Cell::lst), std::end(Cell::lst), []( Cell &c ){ return c.num != 0; }) + 1;
	}

	bool empty()
	{
		return Cell::num == 0;
	}

	bool equal( uint n )
	{
		return Cell::num != 0 && Cell::num == n;
	}

	bool passable( uint n )
	{
		if (Cell::num != 0)
			return false;

		if (n == 0)
			return true;

		return std::none_of(std::begin(Cell::lst), std::end(Cell::lst), [n]( Cell &c ){ return c.num == n; });
	}

	bool corrupt()
	{
		if (Cell::num == 0)
			return Cell::len() == 0;

		return std::any_of(std::begin(Cell::lst), std::end(Cell::lst), [this]( Cell &c ){ return c.num == Cell::num; });
	}

	bool allowed( uint n )
	{
		if (Cell::num != 0 || n == 0)
			return false;

		Cell::num = n;
		bool result = std::none_of(std::begin(Cell::lst), std::end(Cell::lst), []( Cell &c ){ return c.corrupt(); });
		Cell::num = 0;

		return result;
	}

	bool accept( uint n )
	{
		return Cell::num == n || Cell::allowed(n);
	}

	uint sure( uint n = 0 )
	{
		if (Cell::num == 0 && n == 0)
		{
			for (uint v: Cell::Values(*this))
				if (v != 0 && Cell::sure(v))
					return v;

			return 0;
		}

		if (!Cell::allowed(n)) return 0;
		if ( Cell::len() == 1) return n;

		if (std::none_of(std::begin(Cell::row), std::end(Cell::row), [n]( Cell &c ){ return c.allowed(n); })) return n;
		if (std::none_of(std::begin(Cell::col), std::end(Cell::col), [n]( Cell &c ){ return c.allowed(n); })) return n;
		if (std::none_of(std::begin(Cell::seg), std::end(Cell::seg), [n]( Cell &c ){ return c.allowed(n); })) return n;

		return 0;
	}

	void clear()
	{
		Cell::num = 0;
		Cell::immutable = false;
	}

	bool set( uint n )
	{
		if (Cell::immutable || (n != 0 && !Cell::allowed(n)))
			return false;

		Cell::num = n;
		return true;
	}

	bool solve( bool check = false )
	{
		cell_ref c = *std::min_element(std::begin(Cell::lst), std::end(Cell::lst), Cell::by_length);
		if (c.get().num != 0)
		{
			Cell * const tab = this - Cell::pos;
			c = std::ref(*std::min_element(tab, tab + 81, Cell::by_length));
			if (c.get().num != 0)
				return true;
		}

		Cell &cell = c.get();
		for (uint v: Cell::Values(cell, true))
		{
			if ((cell.num = v) != 0 && cell.solve(check))
			{
				if (check)
					cell.num = 0;

				return true;
			}
		}

		cell.num = 0;
		return false;
	}

	bool generate( Difficulty level, bool check = false )
	{
		if (Cell::num == 0 || Cell::immutable)
			return false;

		uint n = Cell::num;

		Cell::num = 0;
		if (Cell::sure(n))
			return true;

		Cell::num = n;
		if (level == Difficulty::Easy && !check)
			return false;

		for (uint v: Cell::Values(*this))
		{
			if ((Cell::num = v) != 0 && Cell::solve(true))
			{
				Cell::num = n;
				return false;
			}
		}

		Cell::num = 0;
		return true;
	}

	static
	bool by_length( Cell &a, Cell &b )
	{
		uint a_len = a.len();
		uint b_len = b.len();

		return a.num == 0 &&
		      (b.num != 0 || a_len <  b_len ||
		                    (a_len == b_len && a.range() < b.range()));
	}

	static
	bool by_solid( Cell &a, Cell &b )
	{
		return a.solid() > b.solid();
	}

	template<class T> friend
	std::basic_ostream<T> &operator <<( std::basic_ostream<T> &out, const Cell &cell )
	{
		out << (cell.immutable ? ".123456789" : ".ABCDEFGHI")[cell.num];

		return out;
	}
};

class Sudoku: public cell_array
{
	using Cell = SudokuCell;

	static const
	std::vector<std::basic_string<TCHAR>> extreme;

	static const
	std::basic_string<TCHAR> html;

	std::list<std::pair<Cell *, uint>> mem;

	class Backup: public std::array<std::tuple<Cell *, uint, bool>, 81>
	{
	public:

		Backup( cell_array *tab )
		{
			std::transform(std::begin(*tab), std::end(*tab), Backup::begin(), []( Cell &c )
			{
				return std::make_tuple(&c, c.num, c.immutable);
			});
		}

		void reload()
		{
			std::for_each(Backup::begin(), Backup::end(), []( std::tuple<Cell *, uint, bool> &t )
			{
				Cell *c = std::get<Cell *>(t);
				std::get<uint>(t) = c->num;
				std::get<bool>(t) = c->immutable;
			});
		}

		void restore()
		{
			std::for_each(Backup::begin(), Backup::end(), []( std::tuple<Cell *, uint, bool> &t )
			{
				Cell *c = std::get<Cell *>(t);
				c->num = std::get<uint>(t);
				c->immutable = std::get<bool>(t);
			});
		}

		bool changed()
		{
			return std::any_of(Backup::begin(), Backup::end(), []( std::tuple<Cell *, uint, bool> &t )
			{
				Cell *c = std::get<Cell *>(t);
				return c->num != std::get<uint>(t);
			});
		}

		uint len()
		{
			return std::count_if(Backup::begin(), Backup::end(), []( std::tuple<Cell *, uint, bool> &t )
			{
				return std::get<uint>(t) != 0;
			});
		}
	};

	class Temp: public Sudoku::Backup
	{
	public:

		Temp( cell_array *tab ): Sudoku::Backup(tab) {}
		~Temp() { Sudoku::Backup::restore(); }

		bool reset()
		{
			return std::all_of(Sudoku::Backup::begin(), Sudoku::Backup::end(), []( std::tuple<Cell *, uint, bool> &t )
			{
				Cell *c = std::get<Cell *>(t);
				return c->set(std::get<uint>(t));
			});
		}
	};

	class Random: public std::vector<cell_ref>
	{
		std::mt19937 rnd;

	public:

		Random( cell_array *tab ): std::vector<cell_ref>(std::begin(*tab), std::end(*tab)), rnd{std::random_device{}()}
		{
			std::shuffle(Random::begin(), Random::end(), Random::rnd);
		}

		Cell& operator()()
		{
			return Random::at(Random::rnd() % Random::size());
		}
	};

	class Sorted: public std::vector<cell_ref>
	{
	public:

		Sorted( cell_array *tab, bool(*compare)(Cell &, Cell&) ): std::vector<cell_ref>(std::begin(*tab), std::end(*tab))
		{
			std::sort(Sorted::begin(), Sorted::end(), compare);
		}
	};

public:

	Difficulty level;
	int        rating;
	uint32_t   signature;

	Sudoku( Difficulty l = Difficulty::Easy ): mem{}, level{l}, rating{0}, signature{0}
	{
		for (Cell &cell: *this)
		{
			auto pos = &cell - this->cell_array::data();
			cell.init(static_cast<uint>(pos));
		}
	}

	uint len()
	{
		return std::count_if(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.num != 0; });
	}

	uint count( uint n )
	{
		return std::count_if(Sudoku::begin(), Sudoku::end(), [n]( Cell &c ){ return c.num == n; });
	}

	bool empty()
	{
		return std::all_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.empty(); });
	}

	bool corrupt()
	{
		for (auto i = Sudoku::begin(); i != Sudoku::end(); i += 9)
			for (uint n = 1; n <= 9; ++n)
				if (std::none_of(i, i + 9, [n]( Cell &c ){ return c.accept(n); }))
					return true;

		return std::any_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.corrupt(); });
	}

	bool solved()
	{
		return std::none_of(Sudoku::begin(), Sudoku::end(), []( Cell &c ){ return c.empty() || c.corrupt(); });
	}

	bool set( Cell *cell, uint n, Force force = Force::Direct )
	{
		if (cell == nullptr)
			return false;

		uint t = cell->num;

		if (t == n)
			return false;

		switch (force)
		{
		case Force::Direct:
			if (cell->immutable)
				return false;
			cell->num = n;
			break;
		case Force::Careful:
			if (!cell->passable(n))
				return false;
			cell->num = n;
			break;
		case Force::Safe:
			if (!cell->set(n))
				return false;
			break;
		}

		Sudoku::mem.emplace_back(cell, t);
		Sudoku::rating = 0;
		return true;
	}

	void clear( bool deep = true )
	{
		for (Cell &c: *this)
			c.clear();

		if (deep)
		{
			Sudoku::rating = 0;
			Sudoku::signature = 0;
			if (Sudoku::level > Difficulty::Easy && Sudoku::level < Difficulty::Extreme)
				Sudoku::level = Difficulty::Medium;
		}
	}

	void discard()
	{
		for (Cell &c: *this)
			c.immutable = false;
	}

	void accept( bool estimate = false, Difficulty difficulty = Difficulty::Any )
	{
		for (Cell &c: *this)
			c.immutable = c.num != 0;

		if (difficulty != Difficulty::Any)
			Sudoku::level = difficulty;

		Sudoku::specify_layout(estimate);
		Sudoku::mem.clear();
	}

	void init( std::basic_string<TCHAR> txt )
	{
		Sudoku::clear();

		for (Cell &c: *this)
		{
			if (c.pos < txt.size())
			{
				int x = txt[c.pos] - _T('0');
				c.set(x >= 0 && x <= 9 ? static_cast<uint>(x) : 0U);
			}
		}

		Sudoku::accept();

		for (Cell &c: *this)
		{
			if (c.pos < txt.size())
			{
				int x = txt[c.pos] - _T('@');
				c.set(x >= 0 && x <= 9 ? static_cast<uint>(x) : 0U);
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

private:

	void swap_cells( uint p1, uint p2 )
	{
		std::swap(Sudoku::at(p1).num,       Sudoku::at(p2).num);
		std::swap(Sudoku::at(p1).immutable, Sudoku::at(p2).immutable);
	}

	void swap_rows( uint r1, uint r2 )
	{
		r1 *= 9; r2 *= 9;
		for (uint c = 0; c < 9; c++)
			Sudoku::swap_cells(r1 + c, r2 + c);
	}

	void swap_cols( uint c1, uint c2 )
	{
		for (uint r = 0; r < 81; r += 9)
			Sudoku::swap_cells(r + c1, r + c2);
	}

	void shuffle()
	{
		auto rnd = std::mt19937{std::random_device{}()};

		uint v[10];
	 	std::iota(v, v + 10, 0);
		std::shuffle(v + 1, v + 10, rnd);

		for (Cell &c: *this)
			c.num = v[c.num];

		for (uint i = 0; i < 81; i++)
		{
			uint c1 = rnd() % 9;
			uint c2 = 3 * (c1 / 3) + (c1 + 1) % 3;
			Sudoku::swap_cols(c1, c2);

			uint r1 = rnd() % 9;
			uint r2 = 3 * (r1 / 3) + (r1 + 1) % 3;
			Sudoku::swap_rows(r1, r2);

			c1 = rnd() % 3;
			c2 = (c1 + 1) % 3;
			c1 *= 3; c2 *= 3;
			for (uint j = 0; j < 3; j++)
				Sudoku::swap_cols(c1 + j, c2 + j);

			r1 = rnd() % 3;
			r2 = (r1 + 1) % 3;
			r1 *= 3; r2 *= 3;
			for (uint j = 0; j < 3; j++)
				Sudoku::swap_rows(r1 + j, r2 + j);
		}
	}

	int solvable()
	{
		if (Sudoku::corrupt())
			return -2;

		auto tmp = Sudoku::Temp(this);
		Sudoku::clear(false);

		if (!tmp.reset())
			return -2;

		return 0;
	}

	int correct()
	{
		auto tmp = Sudoku::Temp(this);

		std::max_element(Sudoku::begin(), Sudoku::end(), Cell::by_length)->solve();
		if (!Sudoku::solved())
			return -2;

		if (!std::all_of(Sudoku::begin(), Sudoku::end(), [this]( Cell &c ){ return c.generate(Sudoku::level, true) != c.immutable; }))
			return -1;

		return 0;
	}

	bool simplify( bool confirm = false )
	{
		bool result = false;

		bool simplified;
		do
		{
			simplified = false;
			for (Cell &c: *this)
			{
				if (c.num == 0 && (c.num = c.sure(0)) != 0)
				{
					if (confirm)
						c.immutable = true;
					result = simplified = true;
				}
			}
		}
		while (simplified);

		if (confirm)
		{
			Sudoku::level = Difficulty::Expert;
			Sudoku::rating = 0;
			Sudoku::signature = 0;
		}

		return result;
	}

	int weight()
	{
		return Sudoku::rating - static_cast<int>(Sudoku::len()) * 25;
	}

public:

	void solve()
	{
		if (Sudoku::solvable() == 0)
		{
			std::max_element(Sudoku::begin(), Sudoku::end(), Cell::by_length)->solve();
			Sudoku::mem.clear();
		}
	}

	void generate( Difficulty difficulty = Difficulty::Any )
	{
		if (difficulty != Difficulty::Any)
			Sudoku::level = difficulty;

		if (Sudoku::level == Difficulty::Extreme)
		{
			auto rnd = std::mt19937{std::random_device{}()};

			Sudoku::init(Sudoku::extreme[rnd() % Sudoku::extreme.size()]);
			Sudoku::shuffle();
		}
		else
		{
			auto rnd = Sudoku::Random(this);
			Sudoku::clear();
			rnd().solve();
			for (Cell &c: rnd)
				c.generate(Sudoku::level);
			Sudoku::accept();
		}
	}

	bool verify( bool forced )
	{
		Difficulty current = Sudoku::level;
		Sudoku::accept(true);
		if (Sudoku::rating >= 0 && (!forced || (Sudoku::level >= current && Sudoku::level >= Difficulty::Hard)))
		{
			if (forced && Sudoku::level == Difficulty::Hard && Sudoku::level > current)
				Sudoku::simplify(true);
			return true;
		}
		Sudoku::level = current;
		return false;
	}

	void raise( bool force = true, bool show = true )
	{
		Sudoku::accept(false, Difficulty::Medium);
		if (show)
			std::cerr << *this << std::endl;

		if (force && Sudoku::level == Difficulty::Hard)
		{
			Sudoku::simplify(true);
			if (show)
			{
				if (Sudoku::len() <= 20)
					accept();
				std::cerr << *this << std::endl;
			}
		}

		bool forced = false;
		bool success = true;
		while (success && Sudoku::len() > 17)
		{
			forced = forced || (force && (Sudoku::level >= Difficulty::Hard || Sudoku::len() <= 30));
			success = false;
			auto vec = Sudoku::Sorted(this, Cell::by_solid);
			for (auto i = vec.begin(); i != vec.end(); ++i)
			{
				Cell &ci = *i;
				if (ci.num == 0) continue;
				uint ni = ci.num;
				ci.num = 0;

				for (auto j = i + 1; j != vec.end(); ++j)
				{
					Cell &cj = *j;
					if (cj.num == 0) continue;
					uint nj = cj.num;
					cj.num = 0;

					for (Cell &cell: Sudoku::Random(this))
					{
						if (cell.num != 0) continue;
						if (&cell != &ci && &cell != &cj && !cell.linked(&ci) && !cell.linked(&cj)) continue;

						for (uint v: Cell::Values(cell))
						{
							if ((cell.num = v) != 0 && Sudoku::verify(forced))
							{
								if (show)
								{
									if (Sudoku::len() <= 20)
										accept();
									std::cerr << *this << std::endl;
								}

								success = true;
								break;
							}
						}

						if (success) break;
						cell.num = 0;
					}

					if (success) break;
					cj.num = nj;
				}

				if (success) break;
				ci.num = ni;
			}
		}

		Sudoku::accept();
		if (show)
			std::cerr << *this << std::endl;
	}

	bool test( bool all )
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

		return Sudoku::level == Difficulty::Easy || all || Sudoku::weight() >= 0;
	}

	bool undo()
	{
		if (!Sudoku::mem.empty())
		{
			std::get<Cell *>(Sudoku::mem.back())->num = std::get<uint>(Sudoku::mem.back());
			Sudoku::mem.pop_back();
			return false;
		}
		else
		{
			Sudoku::again();
			Sudoku::specify_layout();
			return true;
		}
	}

private:

	int parse_rating()
	{
		std::vector<std::pair<Cell *, uint>> sure;
		for (Cell &c: *this)
		{
			if (c.num == 0)
			{
				uint n = c.sure();
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
			for (std::pair<Cell *, uint> &p: sure)
				if (!std::get<Cell *>(p)->set(std::get<uint>(p)))
					success = false;
			if (success)
				result = Sudoku::parse_rating() + 1;
			for (std::pair<Cell *, uint> &p: sure)
				std::get<Cell *>(p)->num = 0;
			return result;
		}
			
		Cell &cell = *std::min_element(Sudoku::begin(), Sudoku::end(), Cell::by_length);
		if (cell.num != 0) // solved!
			return 1;

		uint len    = cell.len();
		uint range  = cell.range();
		int result = 0;
		for (Cell &c: *this)
		{
			if (c.num == 0 && c.len() == len && c.range() == range)
			{
				int r = 0;
				for (uint v: Cell::Values(c))
				{
					if (v != 0 && c.set(v))
					{
						r += Sudoku::parse_rating();
						c.num = 0;
					}
				}
				if (result == 0 || r < result)
					result = r;
			}
		}

		return result + 1;
	}

	template <size_t N>
	uint32_t calculate_crc32( const std::array<uint32_t, N> &data, uint32_t crc = 0 )
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

	void calculate_rating( bool estimate = false )
	{
		Sudoku::rating = Sudoku::solvable(); if (Sudoku::rating != 0) return;
		Sudoku::rating = Sudoku::correct();  if (Sudoku::rating != 0) return;

		if (estimate || Sudoku::level == Difficulty::Extreme) return;

		int msb = 0;
		int result = Sudoku::parse_rating();
		for (uint i = Sudoku::count(0); result > 0; Sudoku::rating += static_cast<int>(i--), result >>= 1)
			msb = (result & 1) ? msb + 1 : 0;
		Sudoku::rating += msb - 1;
	//	Sudoku::rating = Sudoku::parse_rating();
	}

	void calculate_level()
	{
		if ( Sudoku::level                      == Difficulty::Easy)    { return; }
		if ( Sudoku::level                      == Difficulty::Extreme) { return; }
		if ( Sudoku::rating < 0) { Sudoku::level = Difficulty::Medium;    return; }
		if ( Sudoku::solved())   { Sudoku::level = Difficulty::Medium;    return; }
		if (!Sudoku::simplify()) { Sudoku::level = Difficulty::Expert;    return; }
		if (!Sudoku::solved())   { Sudoku::level = Difficulty::Hard;              }
		else                     { Sudoku::level = Difficulty::Medium;            }
		Sudoku::again();
	}

	void calculate_signature( bool estimate = false )
	{
		if (estimate)
		{
			Sudoku::signature = 0;
			return;
		}

		std::array<uint32_t, 10> v = { 0 };
		std::array<uint32_t, 81> l;
		std::array<uint32_t, 81> r;

		for (Cell &c: *this)
		{
			v[c.num]++;
			l[c.pos] = static_cast<uint32_t>(c.len());
			r[c.pos] = static_cast<uint32_t>(c.range());
		}

		std::sort(std::begin(v), std::end(v));
		std::sort(std::begin(l), std::end(l));
		std::sort(std::begin(r), std::end(r));

		Sudoku::signature = Sudoku::calculate_crc32(v);
		Sudoku::signature = Sudoku::calculate_crc32(l, Sudoku::signature);
		Sudoku::signature = Sudoku::calculate_crc32(r, Sudoku::signature);
	}

	void specify_layout( bool estimate = false )
	{
		Sudoku::calculate_rating(estimate);
		Sudoku::calculate_level();		// must be after calculate_rating (depends on the rating)
		Sudoku::calculate_signature(estimate);
	}

public:

	static
	bool by_weight( Sudoku &a, Sudoku &b )
	{
		int a_wgt = a.weight();
		int b_wgt = b.weight();
		uint a_len = a.len();
		uint b_len = b.len();

		return a_wgt  > b_wgt ||
		      (a_wgt == b_wgt && (a_len  < b_len ||
		                         (a_len == b_len && (a.level  > b.level ||
		                                            (a.level == b.level && a.signature < b.signature)))));
	}

	static
	bool by_rating( Sudoku &a, Sudoku &b )
	{
		uint a_len = a.len();
		uint b_len = b.len();

		return a.rating  > b.rating ||
		      (a.rating == b.rating && (a_len  < b_len ||
		                               (a_len == b_len && (a.level  > b.level ||
		                                                  (a.level == b.level && a.signature < b.signature)))));
	}

	static
	bool by_length( Sudoku &a, Sudoku &b )
	{
		uint a_len = a.len();
		uint b_len = b.len();

		return a_len  < b_len ||
		      (a_len == b_len && (a.rating  > b.rating ||
		                         (a.rating == b.rating && (a.level  > b.level ||
		                                                  (a.level == b.level && a.signature < b.signature)))));
	}

	template<class T> friend
	std::basic_istream<T> &operator >>( std::basic_istream<T> &in, Sudoku &sudoku )
	{
		std::basic_string<T> line;
		if (std::getline(in, line))
		{
			sudoku.level = Difficulty::Medium;
			if (line.size() > 0)
				sudoku.init(line.substr(0, 81));
		}

		return in;
	}

	template<class T> friend
	std::basic_ostream<T> &operator <<( std::basic_ostream<T> &out, Sudoku &sudoku )
	{
		for (Cell &c: sudoku)
			out << c;
		out << '|'          <<             sudoku.level     << ':'
		    << std::setw(2) <<             sudoku.len()     << ':'
		    << std::setw(3) <<             sudoku.rating    << ':'
		    << std::setw(8) << std::hex << sudoku.signature << std::dec;
		return out;
	}

	bool load( const TCHAR *filename = _T("sudoku.board") )
	{
		auto file = std::basic_ifstream<TCHAR>(filename);
		if (!file.is_open())
			return false;

		Sudoku::Backup tmp(this);

		file >> *this;

		file.close();

		return tmp.changed();
	}

	void save( const TCHAR *filename = _T("sudoku.board") )
	{
		auto file = std::basic_ofstream<TCHAR>(filename, std::ios::out);
		if (!file.is_open())
			return;

		file << *this << std::endl;

		file.close();

		auto htmlname = std::basic_string<TCHAR>(filename) + _T(".html");
		file = std::basic_ofstream<TCHAR>(htmlname.c_str(), std::ios::out);
		if (!file.is_open())
			return;

		size_t pos = 0;
		auto puzzle = html;
		for (Cell &c: *this)
		{
			pos = puzzle.find(_T(" </td>"), pos + 1);
			puzzle[pos] = _T(" 123456789")[c.num];
		}
		
		file << puzzle;

		file.close();
	}

	void append( const TCHAR *filename )
	{
		auto file = std::basic_ofstream<TCHAR>(filename, std::ios::app);
		if (!file.is_open())
			return;

		file << *this << std::endl;

		file.close();
	}

	static
	void load( std::vector<std::basic_string<TCHAR>> &lst, const TCHAR *filename )
	{
		auto file = std::basic_ifstream<TCHAR>(filename);
		if (!file.is_open())
			return;

		std::basic_string<TCHAR> line;
		while (std::getline(file, line))
		{
			if (line.size() > 0)
				lst.push_back(line.substr(0, 81));
		}

		file.close();
	}
};

const
std::vector<std::basic_string<TCHAR>> Sudoku::extreme =
{
_T(".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..|3:21:702:9c071975"),
_T("4.....8.5.3..........7......2.....6.....8.4...4..1.......6.3.7.5.32.1...1.4......|3:20:666:10144aa6"),
_T("52...6.........7.131..........4..8..6......5...........418.........3..28.387.....|3:20:666:f54090de"),
_T(".9............15...68........2.5.4.....8...9...1....5..2.6...85....75...4.....96.|3:21:654:ae498c1b"),
_T("12....3.8.6.4..............2.3.1...........758.7......97.5...6..31.8.2...........|3:21:654:ea3fbd5f"),
_T("7.48..............328...16....2....15.......8....93........6.....63..5...351.2...|3:22:642:16cb5f8e"),
_T("52.....8...........1....7.575694......467...............8.1..29.6...24.......9..8|3:23:630:346fd02a"),
_T("6.....8.3.4.7.................5.4.7.3.42.1...1.6.......2.....5.....8.6...6..1....|3:20:617:d9865cd1"),
_T("2.....31..9.3.......35.64..721.........1.3.7....7.4....18.....5....3.6..........8|3:23:584:c05d7af0"),
_T("...2.8.1..4.3.18............94.2...56.7.5..8.1........7.6...35......7..44........|3:23:584:e8d23d52"),
_T("..7.........9....384..1..2..7....2..36....7.......7.8.......94.18..4...2.....216.|3:23:583:027907b4"),
_T("4.....3.8...8.2...8..7.....2..1...8734.......6........5.4.6.8......184...82......|3:23:583:2ab93c5f"),
_T(".56....82..........28...1.6....56.....5..13....14.........1...8.....2..7.7.59.4..|3:23:583:2e45ac45"),
_T(".9............15...68........2.5.4...5.8...9........5....649185...1.....4.....967|3:23:583:f6415629"),
_T("48.3............7112.......7.5....6....2..8.............1.76...3.....4......53...|3:19:576:0f8412ed"),
_T(".923.........8.1...........1.7.4...........658.6.......6.5.2...4.....7.....9.4...|3:19:575:abdc6334"),
_T(".68.......52..7..........845..3...9..7...5...1..............5.78........3..4..2.8|3:20:567:d8cd3478"),
_T("458....3....8.1....9.....8.....5.39.2..7......4........1..48.........7.2...6.....|3:20:565:ed9dbd09"),
_T("4.....8.5.3........5.7......2.....6.....5.4......1.......693.71..32.1...1.9......|3:21:560:ad7de212"),
_T(".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....|3:21:556:33afc54f"),
_T("8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..|3:21:555:078b18c4"),
_T(".1..6..9...795.......32..4.....42.3...9...8.............8..6..1.2..3.7..4........|3:21:555:0bc328c9"),
_T("..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...|3:21:555:2fde6b62"),
_T("6....5....9....4.87..2............1..1....764....1.8.9.....2....4.6.....38.5.....|3:21:555:495d82fa"),
_T("1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1|3:21:555:f8bffe1a"),
_T("....14....3....2...7..........9...3.6.1.............8.2.....1.4....5.6.8...7.8...|3:18:533:102bebdc"),
_T("3...8.......7....51.......3......36...2..4....7...........6.13..452...........85.|3:19:522:88f2a7a1"),
_T("......5..........39..64......8.7......3.....2....6..4.67.....9......58..48...6...|3:19:522:95d90963"),
_T("...5.1....9....8...6.......4.1..........7..9........3.8.....1.5...21.4.3.1.36....|3:20:518:21d22039"),
_T(".7...15..63..4...........8......7.3...5....4......96.....8..9..2...6...1....5...8|3:20:513:0a6a7338"),
_T("49.....7..8..6..2......5..3.2.....4...8..2........1...3...7.1....1...3.5.6.......|3:20:513:399a75f8"),
_T(".26.........6....3.74.8.........3..2.8..4..1.6..5.........1.78.5....9..........4.|3:20:513:5ff0c9e4"),
_T(".98.1....2......6.............3.2.5..84.........6.4.......4.8.93..5.....8.....1.5|3:20:513:ebd7fdce"),
};

const
std::basic_string<TCHAR> Sudoku::html =
_T("<!doctype html><html><head><title>sudoku</title><style>table{margin-left:auto;margin-right:auto;font-family:Tahoma,Verdana,sans-serif;font-weight:bold;}table,td{border:1px solid black;border-collapse:collapse;}td{width:80px;height:80px;font-size:64px;text-align:center;vertical-align:middle}#top-left{border-top-width:5px;border-left-width:5px;}#top{border-top-width:5px;}#top-right{border-top-width:5px;border-right-width:5px;}#left{border-left-width:5px;}#center{}#right{border-right-width:5px;}#bottom-left{border-bottom-width:5px;border-left-width:5px;}#bottom{border-bottom-width:5px;}#bottom-right{border-bottom-width:5px;border-right-width:5px;}</style></head><body><table><caption><h1>SUDOKU</h1></caption><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr></table></body></html>");
