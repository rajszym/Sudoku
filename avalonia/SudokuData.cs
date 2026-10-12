/******************************************************************************

   @file    SudokuData.cs
   @author  Rajmund Szymanski
   @date    12.10.2026
   @brief   sudoku class: extreme layouts and html template

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

namespace SudokuGame;

public sealed partial class Sudoku
{
	public static readonly string[] Extreme =
	{
		".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..|4:21:702:9c071975",
		"4.....8.5.3..........7......2.....6.....8.4...4..1.......6.3.7.5.32.1...1.4......|4:20:666:10144aa6",
		"52...6.........7.131..........4..8..6......5...........418.........3..28.387.....|4:20:666:f54090de",
		".9............15...68........2.5.4.....8...9...1....5..2.6...85....75...4.....96.|4:21:654:ae498c1b",
		"12....3.8.6.4..............2.3.1...........758.7......97.5...6..31.8.2...........|4:21:654:ea3fbd5f",
		"7.48..............328...16....2....15.......8....93........6.....63..5...351.2...|4:22:642:16cb5f8e",
		"52.....8...........1....7.575694......467...............8.1..29.6...24.......9..8|4:23:630:346fd02a",
		"6.....8.3.4.7.................5.4.7.3.42.1...1.6.......2.....5.....8.6...6..1....|4:20:617:d9865cd1",
		"2.....31..9.3.......35.64..721.........1.3.7....7.4....18.....5....3.6..........8|4:23:584:c05d7af0",
		"...2.8.1..4.3.18............94.2...56.7.5..8.1........7.6...35......7..44........|4:23:584:e8d23d52",
		"..7.........9....384..1..2..7....2..36....7.......7.8.......94.18..4...2.....216.|4:23:583:027907b4",
		"4.....3.8...8.2...8..7.....2..1...8734.......6........5.4.6.8......184...82......|4:23:583:2ab93c5f",
		".56....82..........28...1.6....56.....5..13....14.........1...8.....2..7.7.59.4..|4:23:583:2e45ac45",
		".9............15...68........2.5.4...5.8...9........5....649185...1.....4.....967|4:23:583:f6415629",
		"48.3............7112.......7.5....6....2..8.............1.76...3.....4......53...|4:19:576:0f8412ed",
		".923.........8.1...........1.7.4...........658.6.......6.5.2...4.....7.....9.4...|4:19:575:abdc6334",
		".68.......52..7..........845..3...9..7...5...1..............5.78........3..4..2.8|4:20:567:d8cd3478",
		"458....3....8.1....9.....8.....5.39.2..7......4........1..48.........7.2...6.....|4:20:565:ed9dbd09",
		"4.....8.5.3........5.7......2.....6.....5.4......1.......693.71..32.1...1.9......|4:21:560:ad7de212",
		".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....|4:21:556:33afc54f",
		"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..|4:21:555:078b18c4",
		".1..6..9...795.......32..4.....42.3...9...8.............8..6..1.2..3.7..4........|4:21:555:0bc328c9",
		"..1..4.......6.3.5...9.....8.....7.3.......285...7.6..3...8...6..92......4...1...|4:21:555:2fde6b62",
		"6....5....9....4.87..2............1..1....764....1.8.9.....2....4.6.....38.5.....|4:21:555:495d82fa",
		"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1|4:21:555:f8bffe1a",
		"....14....3....2...7..........9...3.6.1.............8.2.....1.4....5.6.8...7.8...|4:18:533:102bebdc",
		"3...8.......7....51.......3......36...2..4....7...........6.13..452...........85.|4:19:522:88f2a7a1",
		"......5..........39..64......8.7......3.....2....6..4.67.....9......58..48...6...|4:19:522:95d90963",
		"...5.1....9....8...6.......4.1..........7..9........3.8.....1.5...21.4.3.1.36....|4:20:518:21d22039",
		".7...15..63..4...........8......7.3...5....4......96.....8..9..2...6...1....5...8|4:20:513:0a6a7338",
		"49.....7..8..6..2......5..3.2.....4...8..2........1...3...7.1....1...3.5.6.......|4:20:513:399a75f8",
		".26.........6....3.74.8.........3..2.8..4..1.6..5.........1.78.5....9..........4.|4:20:513:5ff0c9e4",
		".98.1....2......6.............3.2.5..84.........6.4.......4.8.93..5.....8.....1.5|4:20:513:ebd7fdce",
	};

	static readonly string Html =
		"<!doctype html><html><head><title>sudoku</title><style>table{margin-left:auto;margin-right:auto;font-family:Tahoma,Verdana,sans-serif;font-weight:bold;}table,td{border:1px solid black;border-collapse:collapse;}td{width:80px;height:80px;font-size:64px;text-align:center;vertical-align:middle}#top-left{border-top-width:5px;border-left-width:5px;}#top{border-top-width:5px;}#top-right{border-top-width:5px;border-right-width:5px;}#left{border-left-width:5px;}#center{}#right{border-right-width:5px;}#bottom-left{border-bottom-width:5px;border-left-width:5px;}#bottom{border-bottom-width:5px;}#bottom-right{border-bottom-width:5px;border-right-width:5px;}</style></head><body><table><caption><h1>SUDOKU</h1></caption><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr></table></body></html>";
}
