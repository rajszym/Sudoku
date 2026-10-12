/******************************************************************************

   @file    Game.cs
   @author  Rajmund Szymanski
   @date    12.10.2026
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

using System.Globalization;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Media.Immutable;
using Avalonia.Threading;

namespace SudokuGame;

/*---------------------------------------------------------------------------*/
/*                                 GRAPHICS                                  */
/*---------------------------------------------------------------------------*/

enum Alignment
{
	Left,
	Center,
	Right,
}

sealed class Font
{
	public readonly Typeface Typeface;
	public readonly double   Size;

	// the font with the given line height (like the cell height of a GDI font)
	public Font( double height, FontWeight weight, string family )
	{
		Typeface = new Typeface(new FontFamily(family), FontStyle.Normal, weight);
		var text = new FormattedText("0", CultureInfo.InvariantCulture, FlowDirection.LeftToRight, Typeface, height, null);
		Size = text.Height > 0 ? height * height / text.Height : height;
	}
}

sealed class Graphics
{
	public DrawingContext Dc = null!;

	readonly Dictionary<Color, ImmutableSolidColorBrush> brushes = new();
	readonly Dictionary<Color, ImmutablePen>             pens    = new();
	readonly Dictionary<(Font, string, Color), FormattedText> texts = new();

	ImmutableSolidColorBrush Brush( Color c )
	{
		if (!brushes.TryGetValue(c, out var b))
			brushes[c] = b = new ImmutableSolidColorBrush(c);
		return b;
	}

	ImmutablePen Pen( Color c )
	{
		if (!pens.TryGetValue(c, out var p))
			pens[c] = p = new ImmutablePen(Brush(c), 1);
		return p;
	}

	public static Rect Deflate( Rect r, double d ) => new(r.X + d, r.Y + d, r.Width - d * 2, r.Height - d * 2);

	public static bool Contains( Rect r, double x, double y ) => x >= r.Left && x < r.Right && y >= r.Top && y < r.Bottom;

	public void FillRect( Rect r, Color c )
	{
		Dc.FillRectangle(Brush(c), new Rect(r.X, r.Y, r.Width + 1, r.Height + 1));
	}

	public void DrawRect( Rect r, Color c )
	{
		// the pen is centered on the path: shift it by half a pixel to cover whole pixels
		Dc.DrawRectangle(null, Pen(c), new Rect(r.X + 0.5, r.Y + 0.5, r.Width, r.Height));
	}

	public void FillPolygon( Point[] points, Color c )
	{
		var geometry = new StreamGeometry();
		using (var ctx = geometry.Open())
		{
			ctx.BeginFigure(points[0], true);
			for (int i = 1; i < points.Length; i++)
				ctx.LineTo(points[i]);
			ctx.EndFigure(true);
		}
		Dc.DrawGeometry(Brush(c), null, geometry);
	}

	public void DrawText( Rect r, Font f, Color c, Alignment a, string t )
	{
		var key = (f, t, c);
		if (!texts.TryGetValue(key, out var text))
		{
			if (texts.Count > 512) // the timer creates a new text every second
				texts.Clear();
			text = new FormattedText(t, CultureInfo.InvariantCulture, FlowDirection.LeftToRight, f.Typeface, f.Size, Brush(c));
			texts[key] = text;
		}

		double x = a switch
		{
			Alignment.Left  => r.Left,
			Alignment.Right => r.Right - text.Width,
			_               => r.Center.X - text.Width / 2,
		};
		double y = r.Center.Y - text.Height / 2;

		Dc.DrawText(text, new Point(Math.Round(x), Math.Round(y)));
	}
}

/*---------------------------------------------------------------------------*/
/*                                  LAYOUT                                   */
/*---------------------------------------------------------------------------*/

static class Layout
{
	public const string Title = "Sudoku";

	public const double Frame    =  0;
	public const double Margin   =  4;
	public const double CellSize = 64;
	public const double SegSize  = CellSize * 3 + Margin * 2;
	public const double TabSize  = SegSize  * 3 + Margin * 8;
	public const double MnuSize  = 12;

	public static readonly Rect TAB = new(Frame + Margin * 2, Frame + CellSize + Margin * 2, TabSize, TabSize);
	public static readonly Rect MNU = new(TAB.Right + Margin, TAB.Top, SegSize, TAB.Height);
	public static readonly Rect HDR = new(Frame, Frame, TAB.Left + MNU.Right, TAB.Top - TAB.Left);
	public static readonly Rect FTR = new(HDR.Left, TAB.Bottom, HDR.Width, CellSize / 2);
	public static readonly Rect WIN = new(0, 0, HDR.Left + HDR.Right + 1, FTR.Bottom + Frame + 1);

	public static readonly Color Background = Colors.Moccasin;
	public static readonly Color Lighted    = Colors.OldLace;

	public const string BigFont   = "Tahoma";
	public const string SmallFont = "Arial";

	public static double Round( double x ) => Math.Round(x, MidpointRounding.AwayFromZero);
}

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
}

enum Command
{
	NoCmd = -1,
	Button0Cmd,
	Button1Cmd,
	Button2Cmd,
	Button3Cmd,
	Button4Cmd,
	Button5Cmd,
	Button6Cmd,
	Button7Cmd,
	Button8Cmd,
	Button9Cmd,
	ClearCellCmd,
	SetCellCmd,
	SetSureCmd,
	PrevHelpCmd,
	NextHelpCmd,
	PrevLevelCmd,
	NextLevelCmd,
	GenerateCmd,
	HighLightCmd,
	TimerCmd,
	SolveCmd,
	UndoCmd,
	ClearCmd,
	EditCmd,
	AcceptCmd,
	SaveCmd,
	LoadCmd,
	QuitCmd,
}

/*---------------------------------------------------------------------------*/

sealed class GameHeader
{
	static Font? font;
	static Font? tiny;

	public void Update( Graphics gr, string? info, int time, int len, int rating )
	{
		font ??= new Font(Layout.HDR.Height,     FontWeight.Medium, Layout.BigFont);
		tiny ??= new Font(Layout.HDR.Height / 2, FontWeight.Normal, Layout.SmallFont);

		var rc = Graphics.Deflate(Layout.HDR, Layout.TAB.Left);
		gr.DrawText(rc, font, Colors.White, Alignment.Left, Layout.Title);

		if (info != null)
			gr.DrawText(rc, tiny, Colors.White, Alignment.Center, info);

		gr.DrawText(rc, font, Colors.Silver, Alignment.Right, Text(time, len, rating));
	}

	public static string Text( int time, int len, int rating )
	{
		if (time >= 0)
			return $"{time / 3600}:{time / 60 % 60:00}:{time % 60:00}";

		// like "%03d" in C: the sign is included in the width
		return $"{len}:" + (rating < 0 ? "-" + (-rating).ToString("00") : rating.ToString("000"));
	}

	public Command MouseLButton( double x, double y )
	{
		if (Graphics.Contains(Layout.HDR, x, y))
			return Command.TimerCmd;

		return Command.NoCmd;
	}
}

/*---------------------------------------------------------------------------*/

sealed class GameCell
{
	readonly Rect       r;
	readonly SudokuCell cell;

	bool focused;

	// the assistance calculations are repeated only after a change of the state
	long state = -1;
	bool allowed;
	int  sure;
	int  full;

	static Font? font;
	static Font? tiny;

	public GameCell( double x, double y, SudokuCell c )
	{
		r = new Rect(x, y, Layout.CellSize, Layout.CellSize);
		cell = c;
	}

	void Prepare( long version, int number, Assistance help )
	{
		if (state == version)
			return;

		state   = version;
		allowed = Allowed(number, help);
		sure    = allowed && help >= Assistance.Sure ? cell.Sure(number) : 0;
		full    = !allowed && help == Assistance.Full ? cell.Sure(number) : 0;
	}

	public void Update( Graphics gr, long version, int number, Assistance help, SudokuCell? focus, bool light )
	{
		font ??= new Font(Layout.CellSize,     FontWeight.Black, Layout.BigFont);
		tiny ??= new Font(Layout.CellSize / 3, FontWeight.Black, Layout.BigFont);

		if (focused || (light && cell.Linked(focus)))
			gr.FillRect(Graphics.Deflate(r, Layout.Margin), Layout.Lighted);

		Prepare(version, number, help);

		if (cell.Num != 0)
		{
			var f = help >= Assistance.Current && cell.Equal(number) ? Colors.Red   :
			        cell.Immutable                                    ? Colors.Black :
			                                                            Colors.Navy;

			gr.DrawText(r, font, f, Alignment.Center, cell.Num.ToString());
		}
		else
		if ((focused || help > Assistance.Current) && allowed)
		{
			var f = help >= Assistance.Sure && sure != 0 ? Colors.Green :
			        help <= Assistance.Current           ? Colors.Gray  :
			                                               Colors.Orange;

			gr.DrawText(r, tiny, f, Alignment.Center, number.ToString());
		}
		else
		if (full != 0)
		{
			gr.DrawText(r, tiny, Colors.Green, Alignment.Center, full.ToString());
		}

		gr.DrawRect(r, Colors.Black);
	}

	public void MouseMove( double x, double y ) => focused = Graphics.Contains(r, x, y);

	public void MouseLeave() => focused = false;

	public Command MouseLButton( int number, Assistance help )
	{
		if (focused)
		{
			if (Allowed(number, help))
				return Command.SetCellCmd;
			else
			if (help == Assistance.Full && number == 0 && cell.Sure() != 0)
				return Command.SetSureCmd;
			else
			if (help != Assistance.None || cell.Num != number)
				return Command.Button0Cmd + cell.Num;
		}

		return Command.NoCmd;
	}

	public Command MouseRButton()
	{
		if (focused && cell.Num != 0)
			return Command.ClearCellCmd;

		return Command.NoCmd;
	}

	public SudokuCell? GetCell() => focused ? cell : null;

	bool Allowed( int number, Assistance help )
	{
		if (number == 0 || cell.Num != 0)
			return false;

		if (help <= Assistance.Current)
			return true;

		if (help != Assistance.Full && cell.Passable(number))
			return true;

		if (cell.Allowed(number))
			return true;

		return false;
	}
}

/*---------------------------------------------------------------------------*/

sealed class GameTable : List<GameCell>
{
	public GameTable( Sudoku s )
	{
		foreach (SudokuCell c in s.Cells)
		{
			double x = Layout.TAB.X + c.Pos % 9 * (Layout.CellSize + Layout.Margin) + c.Pos % 9 / 3 * Layout.Margin * 3;
			double y = Layout.TAB.Y + c.Pos / 9 * (Layout.CellSize + Layout.Margin) + c.Pos / 9 / 3 * Layout.Margin * 3;

			Add(new GameCell(x, y, c));
		}
	}

	public void Update( Graphics gr, long version, int number, Assistance help, SudokuCell? focus, bool light )
	{
		foreach (GameCell c in this)
			c.Update(gr, version, number, help, focus, light);
	}

	public void MouseMove( double x, double y ) { foreach (GameCell c in this) c.MouseMove(x, y); }

	public void MouseLeave() { foreach (GameCell c in this) c.MouseLeave(); }

	public Command MouseLButton( int number, Assistance help )
	{
		foreach (GameCell c in this)
		{
			Command cmd = c.MouseLButton(number, help);
			if (cmd != Command.NoCmd)
				return cmd;
		}

		return Command.NoCmd;
	}

	public Command MouseRButton()
	{
		foreach (GameCell c in this)
		{
			Command cmd = c.MouseRButton();
			if (cmd != Command.NoCmd)
				return cmd;
		}

		return Command.NoCmd;
	}

	public SudokuCell? GetCell()
	{
		foreach (GameCell c in this)
		{
			SudokuCell? cell = c.GetCell();
			if (cell != null)
				return cell;
		}

		return null;
	}
}

/*---------------------------------------------------------------------------*/

sealed class MenuItem : List<string>
{
	readonly Rect      r;
	readonly string    info;
	readonly int       num;
	readonly GameTimer timer = new(100);

	int  idx;
	bool focused;

	static Font? font;

	public MenuItem( int n, double y, double h, string i )
	{
		r = new Rect(Layout.MNU.X, y, Layout.MNU.Width, h);
		info = i;
		num = n;
	}

	// the highlight of the item grows after a click
	public bool Animated => focused && timer.Until((int)Layout.Margin * 4) > 0;

	public void Update( Graphics gr, double x )
	{
		double h = Layout.Round(Layout.TabSize / 1.8 / Layout.MnuSize);

		font ??= new Font(h, FontWeight.Normal, Layout.SmallFont);

		if (focused)
			gr.FillRect(Graphics.Deflate(r, timer.Until((int)Layout.Margin * 4)), Layout.Lighted);

		if (Count > 1)
		{
			var cl = !focused ? Layout.Lighted : (x < Layout.MNU.Center.X || Count == 2) ? Colors.Black : Layout.Background;
			var cr = !focused ? Layout.Lighted : (x < Layout.MNU.Center.X && Count != 2) ? Layout.Background : Colors.Black;

			double a = h / 4;
			double left  = r.Left  + Layout.Margin * 2;
			double right = r.Right - Layout.Margin * 2;
			double mid   = r.Center.Y;
			gr.FillPolygon(new[] { new Point(left,  mid), new Point(left  + a * 2, mid - a), new Point(left  + a * 2, mid + a) }, cl);
			gr.FillPolygon(new[] { new Point(right, mid), new Point(right - a * 2, mid - a), new Point(right - a * 2, mid + a) }, cr);
		}

		gr.DrawText(r, font, Colors.Black, Alignment.Center, this[idx]);
	}

	public void MouseMove( double x, double y ) => focused = Graphics.Contains(r, x, y);

	public void MouseLeave() => focused = false;

	public Command MouseLButton( double x )
	{
		if (focused)
		{
			timer.Restart();

			switch (num)
			{
			case  0: return x < Layout.MNU.Center.X ? Command.PrevLevelCmd : Command.NextLevelCmd;
			case  1: return x < Layout.MNU.Center.X ? Command.PrevHelpCmd  : Command.NextHelpCmd;
			case  2: return Command.HighLightCmd;
			case  3: return Command.GenerateCmd;
			case  4: return Command.SolveCmd;
			case  5: return Command.UndoCmd;
			case  6: return Command.ClearCmd;
			case  7: return Command.EditCmd;
			case  8: return Command.AcceptCmd;
			case  9: return Command.SaveCmd;
			case 10: return Command.LoadCmd;
			case 11: return Command.QuitCmd;
			}
		}

		return Command.NoCmd;
	}

	public string? GetInfo() => focused ? info : null;

	public void SetIndex( int i ) => idx = i;

	public int Prev()
	{
		int i = idx;
		int s = Count;

		if (num == 0)
			idx = i == 0 ? s - 1 : i == s - 1 ? 1 : 0;
		else
			idx = (i + s - 1) % s;

		return idx;
	}

	public int Next()
	{
		int i = idx;
		int s = Count;

		if (num == 0)
			idx = i == s - 1 ? 0 : i == 0 ? 1 : s - 1;
		else
			idx = (i + 1) % s;

		return idx;
	}
}

/*---------------------------------------------------------------------------*/

sealed class GameMenu : List<MenuItem>
{
	static readonly (string Info, string[] Names)[] items =
	{
		("Change the difficulty level: easy, medium / hard / expert, extreme (keyboard shortcuts: D, PgUp, PgDn)",
			new[] { "easy", "medium", "hard", "expert", "extreme" }),
		("Change the assistance level: none, current, available, sure, full (keyboard shortcuts: A, Left, Right)",
			new[] { "none", "current", "available", "sure", "full" }),
		("Display the highlight of the entire group of cells: on / off (keyboard shortcut: H)",
			new[] { "highlight off", "highlight on" }),
		("Generate or load a new layout (keyboard shortcuts: N, Tab)",
			new[] { "new" }),
		("Solve the current layout (keyboard shortcuts: S, Enter)",
			new[] { "solve" }),
		("Undo last move or restore the accepted layout (keyboard shortcuts: U, Backspace)",
			new[] { "undo" }),
		("Clear the board (keyboard shortcuts: C, Delete)",
			new[] { "clear" }),
		("Start editing the current layout (keyboard shortcuts: E, Home)",
			new[] { "edit" }),
		("Accept the current layout and finish editing (keyboard shortcuts: T, End)",
			new[] { "accept" }),
		("Save the current layout to the file (keyboard shortcuts: V, Insert)",
			new[] { "save" }),
		("Load layout from the file (keyboard shortcut: L)",
			new[] { "load" }),
		("Quit the game (keyboard shortcuts: Q, Esc)",
			new[] { "quit" }),
	};

	public GameMenu()
	{
		double h = Layout.Round(Layout.TabSize / 1.2 / Layout.MnuSize);
		double Pos( int i ) => Layout.MNU.Y + Layout.Round(i * (Layout.MNU.Height - h) / (Layout.MnuSize - 1));

		for (int i = 0; i < items.Length; i++)
		{
			var item = new MenuItem(i, Pos(i), h, items[i].Info);
			item.AddRange(items[i].Names);
			Add(item);
		}
	}

	public bool Animated => Exists(m => m.Animated);

	public void Update( Graphics gr, double x )
	{
		foreach (MenuItem m in this)
			m.Update(gr, x);

		gr.DrawRect(Layout.MNU, Colors.Black);
	}

	public void MouseMove( double x, double y ) { foreach (MenuItem m in this) m.MouseMove(x, y); }

	public void MouseLeave() { foreach (MenuItem m in this) m.MouseLeave(); }

	public Command MouseLButton( double x )
	{
		foreach (MenuItem m in this)
		{
			Command cmd = m.MouseLButton(x);
			if (cmd != Command.NoCmd)
				return cmd;
		}

		return Command.NoCmd;
	}

	public string? GetInfo()
	{
		foreach (MenuItem m in this)
		{
			string? info = m.GetInfo();
			if (info != null)
				return info;
		}

		return null;
	}
}

/*---------------------------------------------------------------------------*/

sealed class GameFooter
{
	static Font? font;

	public void Update( Graphics gr, string? info )
	{
		font ??= new Font(Math.Floor(Math.Min(Layout.FTR.Height, Layout.WIN.Width / 40)), FontWeight.Normal, Layout.SmallFont);

		gr.DrawText(Layout.FTR, font, Colors.Gray, Alignment.Center, info ?? "Sudoku game, solver and generator");
	}
}

/*---------------------------------------------------------------------------*/
/*                                   GAME                                    */
/*---------------------------------------------------------------------------*/

sealed class GameView : Control
{
	static readonly Color[] colors =
	{
		Colors.Navy,
		Colors.Green,
		Colors.Olive,
		Colors.Maroon,
		Colors.Maroon,
	};

	readonly Sudoku     sudoku = new();
	readonly Graphics   gr     = new();
	readonly GameHeader hdr    = new();
	readonly GameTable  tab;
	readonly GameMenu   mnu    = new();
	readonly GameFooter ftr    = new();
	readonly GameTimer  timer  = new();

	readonly DispatcherTimer ticker;

	int  number;
	bool timerF = true;
	bool lightF;
	bool busy;
	double cursorX;
	string header = "";

	// the version of the state for the assistance calculations
	long version;
	int[] lastNums = new int[81];
	int lastNumber = -1;
	Assistance lastHelp = (Assistance)(-1);

	Assistance help = Assistance.None;

	public GameView()
	{
		Width  = Layout.WIN.Width;
		Height = Layout.WIN.Height;
		Focusable = true;

		tab = new GameTable(sudoku);

		sudoku.Generate();
		timer.Start();

		mnu[0].SetIndex((int)sudoku.Level);
		mnu[1].SetIndex((int)help);
		mnu[2].SetIndex(lightF ? 1 : 0);

		// redraw only when the displayed time changes or the menu is animated
		ticker = new DispatcherTimer(TimeSpan.FromMilliseconds(20), DispatcherPriority.Background, (_, _) =>
		{
			if (mnu.Animated || GameHeader.Text(timerF ? timer.Now() : -1, sudoku.Len(), sudoku.Rating) != header)
				InvalidateVisual();
		});
		ticker.Start();
	}

	long Version()
	{
		bool changed = number != lastNumber || help != lastHelp;
		for (int i = 0; i < 81; i++)
		{
			if (sudoku[i].Num != lastNums[i])
			{
				lastNums[i] = sudoku[i].Num;
				changed = true;
			}
		}

		lastNumber = number;
		lastHelp = help;
		if (changed)
			version++;

		return version;
	}

	public override void Render( DrawingContext dc )
	{
		gr.Dc = dc;

		int time = timerF ? timer.Now() : -1;
		string? info = sudoku.Len() < 81 ? (sudoku.Rating == -2 ? "unsolvable" : sudoku.Rating == -1 ? "ambiguous" : null)
		                                 : (sudoku.Corrupt() ? "corrupt" : "solved");
		header = GameHeader.Text(time, sudoku.Len(), sudoku.Rating);

		gr.FillRect(new Rect(0, 0, Layout.WIN.Width - 1, Layout.WIN.Height - 1), Layout.Background);

		var T = Layout.TAB;
		double C = Layout.CellSize, M = Layout.Margin;
		gr.FillRect(new Rect(T.X + C * 3 + M * 3, T.Y, M * 2, T.Height), Colors.DimGray);
		gr.FillRect(new Rect(T.X + C * 6 + M * 9, T.Y, M * 2, T.Height), Colors.DimGray);
		gr.FillRect(new Rect(T.X, T.Y + C * 3 + M * 3, T.Width, M * 2), Colors.DimGray);
		gr.FillRect(new Rect(T.X, T.Y + C * 6 + M * 9, T.Width, M * 2), Colors.DimGray);

		gr.FillRect(Layout.HDR, colors[(int)sudoku.Level]);

		hdr.Update(gr, info, time, sudoku.Len(), sudoku.Rating);
		tab.Update(gr, Version(), number, help, tab.GetCell(), lightF);
		mnu.Update(gr, cursorX);
		ftr.Update(gr, mnu.GetInfo());
	}

	/*-----------------------------------------------------------------------*/

	protected override void OnPointerMoved( PointerEventArgs e )
	{
		base.OnPointerMoved(e);
		var p = e.GetPosition(this);
		cursorX = p.X;
		tab.MouseMove(p.X, p.Y);
		mnu.MouseMove(p.X, p.Y);
		InvalidateVisual();
	}

	protected override void OnPointerExited( PointerEventArgs e )
	{
		base.OnPointerExited(e);
		tab.MouseLeave();
		mnu.MouseLeave();
		InvalidateVisual();
	}

	protected override void OnPointerPressed( PointerPressedEventArgs e )
	{
		base.OnPointerPressed(e);
		var p = e.GetPosition(this);
		var props = e.GetCurrentPoint(this).Properties;

		if (props.IsLeftButtonPressed)
		{
			Execute(hdr.MouseLButton(p.X, p.Y));
			Execute(tab.MouseLButton(number, help));
			Execute(mnu.MouseLButton(p.X));
		}
		else
		if (props.IsRightButtonPressed)
		{
			Execute(tab.MouseRButton());
		}

		e.Handled = true;
	}

	protected override void OnPointerWheelChanged( PointerWheelEventArgs e )
	{
		base.OnPointerWheelChanged(e);
		var p = e.GetPosition(this);
		var cell = tab.GetCell();

		if ((help != Assistance.None && Graphics.Contains(Layout.TAB, p.X, p.Y)) || (cell != null && cell.Empty()))
		{
			int n = e.Delta.Y < 0 ? (number == 0 ? 1 : 1 + (number + 0) % 9)
			                      : (number == 0 ? 9 : 1 + (number + 7) % 9);
			Execute(Command.Button0Cmd + n);
		}

		e.Handled = true;
	}

	public bool Keyboard( Key key )
	{
		Command cmd = key switch
		{
			Key.D0       => Command.Button0Cmd,
			Key.D1       => Command.Button1Cmd,
			Key.D2       => Command.Button2Cmd,
			Key.D3       => Command.Button3Cmd,
			Key.D4       => Command.Button4Cmd,
			Key.D5       => Command.Button5Cmd,
			Key.D6       => Command.Button6Cmd,
			Key.D7       => Command.Button7Cmd,
			Key.D8       => Command.Button8Cmd,
			Key.D9       => Command.Button9Cmd,
			Key.Left     => Command.PrevHelpCmd,
			Key.Right    => Command.NextHelpCmd,
			Key.A        => Command.NextHelpCmd,
			Key.PageDown => Command.PrevLevelCmd,
			Key.PageUp   => Command.NextLevelCmd,
			Key.D        => Command.NextLevelCmd,
			Key.Tab      => Command.GenerateCmd,
			Key.N        => Command.GenerateCmd,
			Key.H        => Command.HighLightCmd,
			Key.Enter    => Command.SolveCmd,
			Key.S        => Command.SolveCmd,
			Key.Back     => Command.UndoCmd,
			Key.U        => Command.UndoCmd,
			Key.Delete   => Command.ClearCmd,
			Key.C        => Command.ClearCmd,
			Key.Home     => Command.EditCmd,
			Key.E        => Command.EditCmd,
			Key.End      => Command.AcceptCmd,
			Key.T        => Command.AcceptCmd,
			Key.Insert   => Command.SaveCmd,
			Key.V        => Command.SaveCmd,
			Key.L        => Command.LoadCmd,
			Key.Escape   => Command.QuitCmd,
			Key.Q        => Command.QuitCmd,
			_            => Command.NoCmd,
		};

		Execute(cmd);
		return cmd != Command.NoCmd;
	}

	/*-----------------------------------------------------------------------*/

	bool SetNumber( int num = 0 )
	{
		if (num == 0)
			num = number;

		return num == 0                  ? false :
		       help == Assistance.None ? sudoku.Set(tab.GetCell(), num, Force.Direct) :
		       help != Assistance.Full ? sudoku.Set(tab.GetCell(), num, Force.Careful) :
		                                 sudoku.Set(tab.GetCell(), num, Force.Safe);
	}

	static bool Slow( Command c ) => c is Command.PrevLevelCmd or Command.NextLevelCmd or Command.GenerateCmd
	                                   or Command.SolveCmd or Command.AcceptCmd or Command.LoadCmd;

	void Execute( Command c )
	{
		if (c == Command.NoCmd || busy)
			return;

		if (Slow(c))
		{
			// show the busy cursor, then run the time-consuming command
			busy = true;
			Cursor = new Cursor(StandardCursorType.Wait);
			Dispatcher.UIThread.Post(() =>
			{
				Run(c);
				Cursor = Cursor.Default;
				busy = false;
				InvalidateVisual();
			}, DispatcherPriority.Background);
		}
		else
		{
			Run(c);
			InvalidateVisual();
		}
	}

	void Run( Command c )
	{
		switch (c)
		{
		case Command.NoCmd:        break;
		case Command.Button0Cmd:   number = 0;
		                           break;
		case >= Command.Button1Cmd and <= Command.Button9Cmd:
		                           number = number == c - Command.Button0Cmd ? 0 : c - Command.Button0Cmd;
		                           break;
		case Command.ClearCellCmd: number = tab.GetCell()!.Num;
		                           sudoku.Set(tab.GetCell(), 0);
		                           break;
		case Command.SetCellCmd:   SetNumber();
		                           break;
		case Command.SetSureCmd:   SetNumber(tab.GetCell()!.Sure());
		                           break;
		case Command.PrevHelpCmd:  help = (Assistance)mnu[1].Prev();
		                           break;
		case Command.NextHelpCmd:  help = (Assistance)mnu[1].Next();
		                           break;
		case Command.PrevLevelCmd: sudoku.Level = (Difficulty)mnu[0].Prev();
		                           sudoku.Generate(); number = 0; timer.Start();
		                           mnu[0].SetIndex((int)sudoku.Level);
		                           break;
		case Command.NextLevelCmd: sudoku.Level = (Difficulty)mnu[0].Next();
		                           goto case Command.GenerateCmd;
		case Command.GenerateCmd:  sudoku.Generate(); number = 0; timer.Start();
		                           mnu[0].SetIndex((int)sudoku.Level);
		                           break;
		case Command.HighLightCmd: lightF = !lightF; mnu[2].SetIndex(lightF ? 1 : 0);
		                           break;
		case Command.TimerCmd:     timerF = !timerF;
		                           break;
		case Command.SolveCmd:     sudoku.Solve(); number = 0;
		                           if (sudoku.Len() < 81) sudoku.Rating = -2;
		                           break;
		case Command.UndoCmd:      sudoku.Undo();
		                           break;
		case Command.ClearCmd:     sudoku.Clear(); number = 0; timer.Reset();
		                           break;
		case Command.EditCmd:      sudoku.Discard();           timer.Reset();
		                           break;
		case Command.AcceptCmd:    sudoku.Accept();
		                           if (timer.Now() == 0) timer.Start();
		                           break;
		case Command.SaveCmd:      sudoku.Save(gametime: timer.Now());
		                           break;
		case Command.LoadCmd:      if (sudoku.Load(out int seconds))
		                           {
		                               number = 0;
		                               timer.ContinueIf(sudoku.Rating >= 0, seconds);
		                               mnu[0].SetIndex((int)sudoku.Level);
		                           }
		                           break;
		case Command.QuitCmd:      (TopLevel.GetTopLevel(this) as Window)?.Close();
		                           break;
		}

		if (sudoku.Len() == 81)
		{
			number = 0;
			if (sudoku.Solved())
				timer.Stop();
		}
	}
}
