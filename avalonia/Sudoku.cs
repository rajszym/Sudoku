/******************************************************************************

   @file    Sudoku.cs
   @author  Rajmund Szymanski
   @date    12.10.2026
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

using System.Text;

namespace SudokuGame;

public enum Difficulty
{
	Any = -1,
	Easy = 0,
	Medium,
	Hard,
	Expert,
	Extreme,
}

public enum Force
{
	Direct,
	Careful,
	Safe,
}

internal static class Rnd
{
	public static int Next( int size ) => Random.Shared.Next(size);

	public static void Shuffle<T>( IList<T> list )
	{
		for (int i = list.Count - 1; i > 0; i--)
		{
			int j = Random.Shared.Next(i + 1);
			(list[i], list[j]) = (list[j], list[i]);
		}
	}
}

/*---------------------------------------------------------------------------*/

public sealed class SudokuCell
{
	public readonly int Pos;
	public int  Num;
	public bool Immutable;

	readonly SudokuCell[]     tab;
	readonly List<SudokuCell> row = new(8);
	readonly List<SudokuCell> col = new(8);
	readonly List<SudokuCell> seg = new(8);
	readonly List<SudokuCell> lst = new(20);

	public SudokuCell( SudokuCell[] t, int p )
	{
		tab = t;
		Pos = p;
	}

	// digits that can be entered into the cell (in ascending order or shuffled)
	public List<int> Values( bool shuffled = false )
	{
		Span<bool> used = stackalloc bool[10];
		used[Num] = true;
		foreach (SudokuCell c in lst)
			used[c.Num] = true;

		var result = new List<int>(9);
		for (int v = 1; v <= 9; v++)
			if (!used[v])
				result.Add(v);

		if (shuffled)
			Rnd.Shuffle(result);

		return result;
	}

	int ValuesCount()
	{
		Span<bool> used = stackalloc bool[10];
		used[Num] = true;
		foreach (SudokuCell c in lst)
			used[c.Num] = true;

		int result = 0;
		for (int v = 1; v <= 9; v++)
			if (!used[v])
				result++;

		return result;
	}

	bool InRow( SudokuCell c ) => Pos != c.Pos && Pos / 9 == c.Pos / 9;
	bool InCol( SudokuCell c ) => Pos != c.Pos && Pos % 9 == c.Pos % 9;
	bool InSeg( SudokuCell c ) => Pos != c.Pos && (Pos / 27) * 3 + (Pos % 9) / 3 == (c.Pos / 27) * 3 + (c.Pos % 9) / 3;
	bool InLst( SudokuCell c ) => InRow(c) || InCol(c) || InSeg(c);

	public void Link( SudokuCell c )
	{
		if (InRow(c)) row.Add(c);
		if (InCol(c)) col.Add(c);
		if (InSeg(c)) seg.Add(c);
		if (InLst(c)) lst.Add(c);
	}

	public bool Linked( SudokuCell? c ) => c != null && InLst(c);

	public int Len() => Num != 0 ? 0 : ValuesCount();

	public int Range()
	{
		int result = 0;
		foreach (SudokuCell c in lst)
			result += c.Len();
		return result;
	}

	public int Solid()
	{
		if (Num == 0)
			return 0;

		int result = 1;
		foreach (SudokuCell c in lst)
			if (c.Num != 0)
				result++;
		return result;
	}

	public bool Empty() => Num == 0;

	public bool Equal( int n ) => Num != 0 && Num == n;

	public bool Passable( int n )
	{
		if (Num != 0)
			return false;

		if (n == 0)
			return true;

		foreach (SudokuCell c in lst)
			if (c.Num == n)
				return false;
		return true;
	}

	public bool Corrupt()
	{
		if (Num == 0)
			return Len() == 0;

		foreach (SudokuCell c in lst)
			if (c.Num == Num)
				return true;
		return false;
	}

	public bool Allowed( int n )
	{
		if (Num != 0 || n == 0)
			return false;

		Num = n;
		bool result = true;
		foreach (SudokuCell c in lst)
			if (c.Corrupt()) { result = false; break; }
		Num = 0;

		return result;
	}

	public bool Accept( int n ) => Num == n || Allowed(n);

	public int Sure( int n = 0 )
	{
		if (Num == 0 && n == 0)
		{
			foreach (int v in Values())
				if (Sure(v) != 0)
					return v;

			return 0;
		}

		if (!Allowed(n)) return 0;
		if (Len() == 1)   return n;

		if (!row.Exists(c => c.Allowed(n))) return n;
		if (!col.Exists(c => c.Allowed(n))) return n;
		if (!seg.Exists(c => c.Allowed(n))) return n;

		return 0;
	}

	public void Clear()
	{
		Num = 0;
		Immutable = false;
	}

	public bool Set( int n )
	{
		if (Immutable || (n != 0 && !Allowed(n)))
			return false;

		Num = n;
		return true;
	}

	public bool Solve( bool check = false )
	{
		SudokuCell cell = MinElement(lst, ByLength);
		if (cell.Num != 0)
		{
			cell = MinElement(tab, ByLength);
			if (cell.Num != 0)
				return true;
		}

		foreach (int v in cell.Values(true))
		{
			cell.Num = v;
			if (cell.Solve(check))
			{
				if (check)
					cell.Num = 0;

				return true;
			}
		}

		cell.Num = 0;
		return false;
	}

	public bool Generate( Difficulty level, bool check = false )
	{
		if (Num == 0 || Immutable)
			return false;

		int n = Num;

		Num = 0;
		if (Sure(n) != 0)
			return true;

		Num = n;
		if (level == Difficulty.Easy && !check)
			return false;

		foreach (int v in Values())
		{
			Num = v;
			if (Solve(true))
			{
				Num = n;
				return false;
			}
		}

		Num = 0;
		return true;
	}

	public static bool ByLength( SudokuCell a, SudokuCell b )
	{
		int aLen = a.Len();
		int bLen = b.Len();

		return a.Num == 0 &&
		      (b.Num != 0 || aLen <  bLen ||
		                    (aLen == bLen && a.Range() < b.Range()));
	}

	// equivalents of std::min_element / std::max_element with a comparator
	public static SudokuCell MinElement( IReadOnlyList<SudokuCell> list, Func<SudokuCell, SudokuCell, bool> less )
	{
		SudokuCell result = list[0];
		for (int i = 1; i < list.Count; i++)
			if (less(list[i], result))
				result = list[i];
		return result;
	}

	public static SudokuCell MaxElement( IReadOnlyList<SudokuCell> list, Func<SudokuCell, SudokuCell, bool> less )
	{
		SudokuCell result = list[0];
		for (int i = 1; i < list.Count; i++)
			if (less(result, list[i]))
				result = list[i];
		return result;
	}

	public override string ToString() => (Immutable ? ".123456789" : ".ABCDEFGHI")[Num].ToString();
}

/*---------------------------------------------------------------------------*/

public sealed partial class Sudoku
{
	public readonly SudokuCell[] Cells = new SudokuCell[81];

	readonly List<(SudokuCell Cell, int Num)> mem = new();

	public Difficulty Level;
	public int        Rating;
	public uint       Signature;
	public int        Duration;

	public Sudoku( Difficulty l = Difficulty.Easy )
	{
		for (int i = 0; i < 81; i++)
			Cells[i] = new SudokuCell(Cells, i);

		foreach (SudokuCell c in Cells)
			foreach (SudokuCell d in Cells)
				c.Link(d);

		Level = l;
	}

	// the cells are linked with their neighbours, so only the values can be copied
	public Sudoku( Sudoku s ): this(s.Level)
	{
		Assign(s);
	}

	public void Assign( Sudoku s )
	{
		foreach (SudokuCell c in Cells)
		{
			c.Num       = s.Cells[c.Pos].Num;
			c.Immutable = s.Cells[c.Pos].Immutable;
		}

		mem.Clear();
		Level     = s.Level;
		Rating    = s.Rating;
		Signature = s.Signature;
		Duration  = s.Duration;
	}

	public SudokuCell this[int pos] => Cells[pos];

	/*-----------------------------------------------------------------------*/

	sealed class Backup
	{
		readonly SudokuCell[] cells;
		readonly int[]        num  = new int[81];
		readonly bool[]       imm  = new bool[81];

		public Backup( Sudoku s )
		{
			cells = s.Cells;
			for (int i = 0; i < 81; i++)
			{
				num[i] = cells[i].Num;
				imm[i] = cells[i].Immutable;
			}
		}

		public void Restore()
		{
			for (int i = 0; i < 81; i++)
			{
				cells[i].Num       = num[i];
				cells[i].Immutable = imm[i];
			}
		}

		public bool Changed()
		{
			for (int i = 0; i < 81; i++)
				if (cells[i].Num != num[i])
					return true;
			return false;
		}

		public bool Reset()
		{
			for (int i = 0; i < 81; i++)
				if (!cells[i].Set(num[i]))
					return false;
			return true;
		}
	}

	/*-----------------------------------------------------------------------*/

	public int Len()
	{
		int result = 0;
		foreach (SudokuCell c in Cells)
			if (c.Num != 0)
				result++;
		return result;
	}

	public int Count( int n )
	{
		int result = 0;
		foreach (SudokuCell c in Cells)
			if (c.Num == n)
				result++;
		return result;
	}

	public bool Empty() => Array.TrueForAll(Cells, c => c.Empty());

	public bool Corrupt()
	{
		for (int i = 0; i < 81; i += 9)
			for (int n = 1; n <= 9; n++)
			{
				bool accepted = false;
				for (int j = i; j < i + 9; j++)
					if (Cells[j].Accept(n)) { accepted = true; break; }
				if (!accepted)
					return true;
			}

		return Array.Exists(Cells, c => c.Corrupt());
	}

	public bool Solved() => !Array.Exists(Cells, c => c.Empty() || c.Corrupt());

	public bool Set( SudokuCell? cell, int n, Force force = Force.Direct )
	{
		if (cell == null)
			return false;

		int t = cell.Num;

		if (t == n)
			return false;

		switch (force)
		{
		case Force.Direct:
			if (cell.Immutable)
				return false;
			cell.Num = n;
			break;
		case Force.Careful:
			if (!cell.Passable(n))
				return false;
			cell.Num = n;
			break;
		case Force.Safe:
			if (!cell.Set(n))
				return false;
			break;
		}

		mem.Add((cell, t));
		Rating = 0;
		return true;
	}

	public void Clear( bool deep = true )
	{
		foreach (SudokuCell c in Cells)
			c.Clear();

		if (deep)
		{
			Rating = 0;
			Signature = 0;
			if (Level > Difficulty.Easy && Level < Difficulty.Extreme)
				Level = Difficulty.Medium;
		}
	}

	public void Discard()
	{
		foreach (SudokuCell c in Cells)
			c.Immutable = false;
	}

	public void Accept( bool estimate = false, Difficulty difficulty = Difficulty.Any )
	{
		foreach (SudokuCell c in Cells)
			c.Immutable = c.Num != 0;

		if (difficulty != Difficulty.Any)
			Level = difficulty;

		SpecifyLayout(estimate);
		mem.Clear();
	}

	public void Init( string txt )
	{
		Clear();

		foreach (SudokuCell c in Cells)
		{
			if (c.Pos < txt.Length)
			{
				int x = txt[c.Pos] - '0';
				c.Set(x >= 0 && x <= 9 ? x : 0);
			}
		}

		Accept();

		foreach (SudokuCell c in Cells)
		{
			if (c.Pos < txt.Length)
			{
				int x = txt[c.Pos] - '@';
				c.Set(x >= 0 && x <= 9 ? x : 0);
			}
		}
	}

	public void Again()
	{
		foreach (SudokuCell c in Cells)
			if (!c.Immutable)
				c.Num = 0;

		mem.Clear();
	}

	/*-----------------------------------------------------------------------*/

	void SwapCells( int p1, int p2 )
	{
		(Cells[p1].Num,       Cells[p2].Num)       = (Cells[p2].Num,       Cells[p1].Num);
		(Cells[p1].Immutable, Cells[p2].Immutable) = (Cells[p2].Immutable, Cells[p1].Immutable);
	}

	void SwapRows( int r1, int r2 )
	{
		r1 *= 9; r2 *= 9;
		for (int c = 0; c < 9; c++)
			SwapCells(r1 + c, r2 + c);
	}

	void SwapCols( int c1, int c2 )
	{
		for (int r = 0; r < 81; r += 9)
			SwapCells(r + c1, r + c2);
	}

	void Shuffle()
	{
		var v = new List<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		Rnd.Shuffle(v);
		v.Insert(0, 0);

		foreach (SudokuCell c in Cells)
			c.Num = v[c.Num];

		for (int i = 0; i < 81; i++)
		{
			int c1 = Rnd.Next(9);
			int c2 = (c1 / 3) * 3 + (c1 + 1) % 3;
			SwapCols(c1, c2);

			int r1 = Rnd.Next(9);
			int r2 = (r1 / 3) * 3 + (r1 + 1) % 3;
			SwapRows(r1, r2);

			c1 = Rnd.Next(3);
			c2 = (c1 + 1) % 3;
			c1 *= 3; c2 *= 3;
			for (int j = 0; j < 3; j++)
				SwapCols(c1 + j, c2 + j);

			r1 = Rnd.Next(3);
			r2 = (r1 + 1) % 3;
			r1 *= 3; r2 *= 3;
			for (int j = 0; j < 3; j++)
				SwapRows(r1 + j, r2 + j);
		}
	}

	int Solvable()
	{
		if (Corrupt())
			return -2;

		var tmp = new Backup(this);
		try
		{
			Clear(false);
			if (!tmp.Reset())
				return -2;
		}
		finally
		{
			tmp.Restore();
		}

		return 0;
	}

	int Correct()
	{
		var tmp = new Backup(this);
		try
		{
			SudokuCell.MaxElement(Cells, SudokuCell.ByLength).Solve();
			if (!Solved())
				return -2;

			foreach (SudokuCell c in Cells)
				if (c.Generate(Level, true) == c.Immutable)
					return -1;
		}
		finally
		{
			tmp.Restore();
		}

		return 0;
	}

	bool Simplify()
	{
		bool result = false;

		bool simplified;
		do
		{
			simplified = false;
			foreach (SudokuCell c in Cells)
			{
				if (c.Num == 0 && (c.Num = c.Sure(0)) != 0)
					result = simplified = true;
			}
		}
		while (simplified);

		return result;
	}

	/*-----------------------------------------------------------------------*/

	public void Solve()
	{
		if (Solvable() == 0)
		{
			SudokuCell.MaxElement(Cells, SudokuCell.ByLength).Solve();
			mem.Clear();
		}
	}

	public void Generate( Difficulty difficulty = Difficulty.Any )
	{
		if (difficulty != Difficulty.Any)
			Level = difficulty;

		if (Level == Difficulty.Extreme)
		{
			Init(Extreme[Rnd.Next(Extreme.Length)]);
			Shuffle();
		}
		else
		{
			var rnd = new List<SudokuCell>(Cells);
			Rnd.Shuffle(rnd);
			Clear();
			rnd[Rnd.Next(rnd.Count)].Solve();
			foreach (SudokuCell c in rnd)
				c.Generate(Level);
			Accept();
		}
	}

	public bool Undo()
	{
		if (mem.Count > 0)
		{
			var (cell, num) = mem[^1];
			cell.Num = num;
			mem.RemoveAt(mem.Count - 1);
			return false;
		}
		else
		{
			bool changed = Array.Exists(Cells, c => !c.Immutable && c.Num != 0);
			Again();
			if (changed || Rating == 0) // otherwise the accepted layout is already specified
				SpecifyLayout();
			return true;
		}
	}

	/*-----------------------------------------------------------------------*/

	int ParseRating()
	{
		var sure = new List<(SudokuCell Cell, int Num)>();
		foreach (SudokuCell c in Cells)
		{
			if (c.Num == 0)
			{
				int n = c.Sure();
				if (n != 0)
					sure.Add((c, n));
				else
				if (c.Len() < 2) // wrong way
					return 0;
			}
		}

		if (sure.Count > 0)
		{
			int  result  = 0;
			bool success = true;
			foreach (var (c, n) in sure)
				if (!c.Set(n))
					success = false;
			if (success)
				result = ParseRating() + 1;
			foreach (var (c, _) in sure)
				c.Num = 0;
			return result;
		}

		SudokuCell cell = SudokuCell.MinElement(Cells, SudokuCell.ByLength);
		if (cell.Num != 0) // solved!
			return 1;

		int len   = cell.Len();
		int range = cell.Range();
		int rating = 0;
		foreach (SudokuCell c in Cells)
		{
			if (c.Num == 0 && c.Len() == len && c.Range() == range)
			{
				int r = 0;
				foreach (int v in c.Values())
				{
					if (c.Set(v))
					{
						r += ParseRating();
						c.Num = 0;
					}
				}
				if (rating == 0 || r < rating)
					rating = r;
			}
		}

		return rating + 1;
	}

	static uint CalculateCrc32( uint[] data, uint crc = 0 )
	{
		const uint POLY = 0xEDB88320;

		crc = ~crc;
		foreach (uint x in data)
		{
			crc ^= x;
			for (int i = 0; i < 32; i++)
				crc = (crc & 1) != 0 ? (crc >> 1) ^ POLY : (crc >> 1);
		}
		crc = ~crc;

		return crc;
	}

	void CalculateRating( bool estimate = false )
	{
		Rating = Solvable(); if (Rating != 0) return;
		Rating = Correct();  if (Rating != 0) return;

		if (estimate || Level == Difficulty.Extreme) return;

		int msb = 0;
		int result = ParseRating();
		for (int i = Count(0); result > 0; Rating += i--, result >>= 1)
			msb = (result & 1) != 0 ? msb + 1 : 0;
		Rating += msb - 1;
	}

	void CalculateLevel()
	{
		if (Level == Difficulty.Easy)    { return; }
		if (Level == Difficulty.Extreme) { return; }
		if (Rating < 0)   { Level = Difficulty.Medium; return; }
		if (Solved())     { Level = Difficulty.Medium; return; }
		if (!Simplify())  { Level = Difficulty.Expert; return; }
		if (!Solved())    { Level = Difficulty.Hard;           }
		else              { Level = Difficulty.Medium;         }
		Again();
	}

	void CalculateSignature( bool estimate = false )
	{
		if (estimate)
		{
			Signature = 0;
			return;
		}

		var v = new uint[10];
		var l = new uint[81];
		var r = new uint[81];

		foreach (SudokuCell c in Cells)
		{
			v[c.Num]++;
			l[c.Pos] = (uint)c.Len();
			r[c.Pos] = (uint)c.Range();
		}

		Array.Sort(v);
		Array.Sort(l);
		Array.Sort(r);

		Signature = CalculateCrc32(v);
		Signature = CalculateCrc32(l, Signature);
		Signature = CalculateCrc32(r, Signature);
	}

	void SpecifyLayout( bool estimate = false )
	{
		CalculateRating(estimate);
		CalculateLevel();      // must be after CalculateRating (depends on the rating)
		CalculateSignature(estimate);
	}

	/*-----------------------------------------------------------------------*/

	// equivalent of std::stoi: leading spaces, sign and digits; 0 if there is no number
	static int ParseInt( string s )
	{
		int i = 0;
		while (i < s.Length && char.IsWhiteSpace(s[i])) i++;
		int start = i;
		if (i < s.Length && (s[i] == '+' || s[i] == '-')) i++;
		while (i < s.Length && s[i] >= '0' && s[i] <= '9') i++;
		return int.TryParse(s.AsSpan(start, i - start), out int result) ? result : 0;
	}

	public void Read( string line )
	{
		Level = Difficulty.Medium;
		if (line.Length > 82)
		{
			int l = line[82] - '0';
			Level = l > (int)Difficulty.Any && l <= (int)Difficulty.Extreme ? (Difficulty)l : Difficulty.Medium;
		}
		if (line.Length > 100)
			Duration = ParseInt(line[100..]);
		if (line.Length > 0)
			Init(line[..Math.Min(81, line.Length)]);
	}

	public override string ToString()
	{
		var sb = new StringBuilder(110);
		foreach (SudokuCell c in Cells)
			sb.Append(c.ToString());
		sb.Append('|').Append((int)Level).Append(':')
		  .Append($"{Len(),2}").Append(':')
		  .Append($"{Rating,3}").Append(':')
		  .Append($"{Signature,8:x}").Append(':')
		  .Append(Duration);
		return sb.ToString();
	}

	public bool Load( out int gametime, string filename = "sudoku.board" )
	{
		gametime = 0;

		string? line;
		try
		{
			using var file = new StreamReader(filename);
			line = file.ReadLine();
		}
		catch (Exception e) when (e is IOException || e is UnauthorizedAccessException)
		{
			return false;
		}

		var tmp = new Backup(this);

		if (line != null)
			Read(line);
		gametime = Duration;

		return tmp.Changed();
	}

	public void Save( string filename = "sudoku.board", int gametime = 0 )
	{
		try
		{
			Duration = gametime;
			File.WriteAllText(filename, ToString() + "\n");

			var puzzle = new StringBuilder(Html);
			int pos = 0;
			foreach (SudokuCell c in Cells)
			{
				pos = Html.IndexOf(" </td>", pos + 1, StringComparison.Ordinal);
				puzzle[pos] = " 123456789"[c.Num];
			}

			File.WriteAllText(filename + ".html", puzzle.ToString());
		}
		catch (Exception e) when (e is IOException || e is UnauthorizedAccessException)
		{
		}
	}
}
