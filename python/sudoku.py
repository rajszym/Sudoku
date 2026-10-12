'''****************************************************************************

   @file    sudoku.py
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

****************************************************************************'''

import os, sys, time, random
os.environ.setdefault("PYGAME_HIDE_SUPPORT_PROMPT", "1")
import pygame
from enum import Enum, IntEnum
from pygame.locals import *

sys.setrecursionlimit(10000)

#------------------------------------------------------------------------------
#                            SUDOKU: GENERATOR AND SOLVER
#------------------------------------------------------------------------------

class Difficulty(IntEnum):

	Any     = -1
	Easy    =  0
	Medium  =  1
	Hard    =  2
	Expert  =  3
	Extreme =  4

class Force(Enum):

	Direct  = 0
	Careful = 1
	Safe    = 2

#------------------------------------------------------------------------------

def min_by_length(cells):

	# equivalent of std::min_element(..., SudokuCell::by_length)
	# the range is calculated only when the lengths are equal
	best = None
	best_len = 0
	best_rng = None
	for c in cells:
		if c._num != 0:
			continue
		l = c.len()
		if best is None or l < best_len:
			best, best_len, best_rng = c, l, None
		elif l == best_len:
			if best_rng is None:
				best_rng = best.range()
			r = c.range()
			if r < best_rng:
				best, best_rng = c, r
	return best if best is not None else cells[0]

def max_by_length(cells):

	# equivalent of std::max_element(..., SudokuCell::by_length)
	for c in cells:
		if c._num != 0:
			return c
	return max(cells, key = lambda c: (c.len(), c.range()))

#------------------------------------------------------------------------------

class SudokuCell:

	__slots__ = ('tab', 'pos', '_num', 'immutable', 'row', 'col', 'seg', 'lst', 'cnt', 'free')

	def __init__(self, tab, pos):

		self.tab = tab
		self.pos = pos
		self._num = 0
		self.immutable = False
		self.row = []
		self.col = []
		self.seg = []
		self.lst = []
		self.cnt = [0] * 10 # number of linked cells containing given digit
		self.free = 9       # number of digits not present in linked cells

	@property
	def num(self):

		return self._num

	@num.setter
	def num(self, n):

		o = self._num
		if o != n:
			self._num = n
			for c in self.lst:
				cnt = c.cnt
				cnt[o] -= 1
				if o != 0 and cnt[o] == 0: c.free += 1
				if n != 0 and cnt[n] == 0: c.free -= 1
				cnt[n] += 1

	def values(self, shuffled = False):

		cnt = self.cnt
		result = [v for v in range(1, 10) if cnt[v] == 0 and v != self._num]
		if shuffled:
			random.shuffle(result)
		return result

	def in_row(self, c):

		return self.pos != c.pos and self.pos // 9 == c.pos // 9

	def in_col(self, c):

		return self.pos != c.pos and self.pos % 9 == c.pos % 9

	def in_seg(self, c):

		r1, c1 = divmod(self.pos, 9)
		r2, c2 = divmod(c.pos, 9)
		return self.pos != c.pos and (r1 // 3) * 3 + (c1 // 3) == (r2 // 3) * 3 + (c2 // 3)

	def in_lst(self, c):

		return self.in_row(c) or self.in_col(c) or self.in_seg(c)

	def link(self, c):

		if self.in_row(c): self.row.append(c)
		if self.in_col(c): self.col.append(c)
		if self.in_seg(c): self.seg.append(c)
		if self.in_lst(c): self.lst.append(c)

	def linked(self, c):

		if c is None:
			return False
		return self.in_lst(c)

	def len(self):

		return 0 if self._num != 0 else self.free

	def range(self):

		return sum(c.len() for c in self.lst)

	def solid(self):

		if self._num == 0:
			return 0
		return sum(1 for c in self.lst if c._num != 0) + 1

	def empty(self):

		return self._num == 0

	def equal(self, n):

		return self._num != 0 and self._num == n

	def passable(self, n):

		if self._num != 0:
			return False
		if n == 0:
			return True
		return self.cnt[n] == 0

	def corrupt(self):

		if self._num == 0:
			return self.len() == 0
		return self.cnt[self._num] != 0

	def allowed(self, n):

		# equivalent of: set the number and check that none of the linked cells is corrupt
		if self._num != 0 or n == 0 or self.cnt[n] != 0:
			return False
		for c in self.lst:
			if c._num != 0:
				if c.cnt[c._num] != 0:
					return False
			elif c.free < 2 and (c.free == 0 or c.cnt[n] == 0):
				return False
		return True

	def accept(self, n):

		return self._num == n or self.allowed(n)

	def sure(self, n = 0):

		if self._num == 0 and n == 0:
			for v in self.values():
				if self.sure(v):
					return v
			return 0

		if not self.allowed(n): return 0
		if self.len() == 1:     return n

		for group in (self.row, self.col, self.seg):
			for c in group:
				if c.allowed(n):
					break
			else:
				return n

		return 0

	def clear(self):

		self.num = 0
		self.immutable = False

	def set(self, n):

		if self.immutable or (n != 0 and not self.allowed(n)):
			return False
		self.num = n
		return True

	def solve(self, check = False):

		cell = min_by_length(self.lst)
		if cell._num != 0:
			cell = min_by_length(self.tab)
			if cell._num != 0:
				return True

		for v in cell.values(True):
			cell.num = v
			if cell.solve(check):
				if check:
					cell.num = 0
				return True

		cell.num = 0
		return False

	def generate(self, level, check = False):

		if self._num == 0 or self.immutable:
			return False

		n = self._num

		self.num = 0
		if self.sure(n):
			return True

		self.num = n
		if level == Difficulty.Easy and not check:
			return False

		for v in self.values():
			self.num = v
			if self.solve(True):
				self.num = n
				return False

		self.num = 0
		return True

	def __str__(self):

		return (".123456789" if self.immutable else ".ABCDEFGHI")[self._num]

#------------------------------------------------------------------------------

class Sudoku(list):

	class Backup(list):

		def __init__(self, tab):

			super().__init__((c, c.num, c.immutable) for c in tab)

		def restore(self):

			for c, n, i in self:
				c.num = n
				c.immutable = i

		def changed(self):

			return any(c.num != n for c, n, _ in self)

	class Temp(Backup):

		def __enter__(self):

			return self

		def __exit__(self, *args):

			self.restore()

		def reset(self):

			return all(c.set(n) for c, n, _ in self)

	class Random(list):

		def __init__(self, tab):

			super().__init__(tab)
			random.shuffle(self)

		def __call__(self):

			return random.choice(self)

	def __init__(self, level = Difficulty.Easy):

		super().__init__(SudokuCell(self, p) for p in range(81))
		for c in self:
			for d in self:
				c.link(d)
			c.cnt[0] = len(c.lst)

		self.mem       = []
		self.level     = level
		self.rating    = 0
		self.signature = 0
		self.duration  = 0

	def len(self):

		return sum(1 for c in self if c._num != 0)

	def count(self, n):

		return sum(1 for c in self if c._num == n)

	def empty(self):

		return all(c.empty() for c in self)

	def corrupt(self):

		for i in range(0, 81, 9):
			for n in range(1, 10):
				if not any(c.accept(n) for c in self[i:i + 9]):
					return True

		return any(c.corrupt() for c in self)

	def solved(self):

		return not any(c.empty() or c.corrupt() for c in self)

	def set(self, cell, n, force = Force.Direct):

		if cell is None:
			return False

		t = cell.num

		if t == n:
			return False

		if force == Force.Direct:
			if cell.immutable:
				return False
			cell.num = n
		elif force == Force.Careful:
			if not cell.passable(n):
				return False
			cell.num = n
		elif force == Force.Safe:
			if not cell.set(n):
				return False

		self.mem.append((cell, t))
		self.rating = 0
		return True

	def clear(self, deep = True):

		for c in self:
			c.clear()

		if deep:
			self.rating = 0
			self.signature = 0
			if self.level > Difficulty.Easy and self.level < Difficulty.Extreme:
				self.level = Difficulty.Medium

	def discard(self):

		for c in self:
			c.immutable = False

	def accept(self, estimate = False, difficulty = Difficulty.Any):

		for c in self:
			c.immutable = c.num != 0

		if difficulty != Difficulty.Any:
			self.level = difficulty

		self.specify_layout(estimate)
		self.mem.clear()

	def init(self, txt):

		self.clear()

		for c in self:
			if c.pos < len(txt):
				x = ord(txt[c.pos]) - ord('0')
				c.set(x if 0 <= x <= 9 else 0)

		self.accept()

		for c in self:
			if c.pos < len(txt):
				x = ord(txt[c.pos]) - ord('@')
				c.set(x if 0 <= x <= 9 else 0)

	def again(self):

		for c in self:
			if not c.immutable:
				c.num = 0

		self.mem.clear()

	def swap_cells(self, p1, p2):

		a, b = self[p1], self[p2]
		a.num, b.num = b.num, a.num
		a.immutable, b.immutable = b.immutable, a.immutable

	def swap_rows(self, r1, r2):

		r1 *= 9; r2 *= 9
		for c in range(9):
			self.swap_cells(r1 + c, r2 + c)

	def swap_cols(self, c1, c2):

		for r in range(0, 81, 9):
			self.swap_cells(r + c1, r + c2)

	def shuffle(self):

		v = list(range(10))
		w = v[1:]
		random.shuffle(w)
		v[1:] = w

		for c in self:
			c.num = v[c.num]

		for i in range(81):
			c1 = random.randrange(9)
			c2 = (c1 // 3) * 3 + (c1 + 1) % 3
			self.swap_cols(c1, c2)

			r1 = random.randrange(9)
			r2 = (r1 // 3) * 3 + (r1 + 1) % 3
			self.swap_rows(r1, r2)

			c1 = random.randrange(3)
			c2 = (c1 + 1) % 3
			c1 *= 3; c2 *= 3
			for j in range(3):
				self.swap_cols(c1 + j, c2 + j)

			r1 = random.randrange(3)
			r2 = (r1 + 1) % 3
			r1 *= 3; r2 *= 3
			for j in range(3):
				self.swap_rows(r1 + j, r2 + j)

	def solvable(self):

		if self.corrupt():
			return -2

		with Sudoku.Temp(self) as tmp:
			self.clear(False)
			if not tmp.reset():
				return -2

		return 0

	def correct(self):

		with Sudoku.Temp(self):
			max_by_length(self).solve()
			if not self.solved():
				return -2

			if not all(c.generate(self.level, True) != c.immutable for c in self):
				return -1

		return 0

	def simplify(self, confirm = False):

		result = False

		simplified = True
		while simplified:
			simplified = False
			for c in self:
				if c.num == 0:
					c.num = c.sure(0)
					if c.num != 0:
						if confirm:
							c.immutable = True
						result = simplified = True

		if confirm:
			self.level = Difficulty.Expert
			self.rating = 0
			self.signature = 0

		return result

	def solve(self):

		if self.solvable() == 0:
			max_by_length(self).solve()
			self.mem.clear()

	def generate(self, difficulty = Difficulty.Any):

		if difficulty != Difficulty.Any:
			self.level = difficulty

		if self.level == Difficulty.Extreme:
			self.init(random.choice(Sudoku.extreme))
			self.shuffle()
		else:
			rnd = Sudoku.Random(self)
			self.clear()
			rnd().solve()
			for c in rnd:
				c.generate(self.level)
			self.accept()

	def undo(self):

		if self.mem:
			cell, n = self.mem.pop()
			cell.num = n
			return False
		else:
			self.again()
			self.specify_layout()
			return True

	def parse_rating(self):

		sure = []
		for c in self:
			if c.num == 0:
				n = c.sure()
				if n != 0:
					sure.append((c, n))
				elif c.len() < 2: # wrong way
					return 0

		if sure:
			result  = 0
			success = True
			for c, n in sure:
				if not c.set(n):
					success = False
			if success:
				result = self.parse_rating() + 1
			for c, n in sure:
				c.num = 0
			return result

		cell = min_by_length(self)
		if cell.num != 0: # solved!
			return 1

		length = cell.len()
		range_ = cell.range()
		result = 0
		for c in self:
			if c.num == 0 and c.len() == length and c.range() == range_:
				r = 0
				for v in c.values():
					if c.set(v):
						r += self.parse_rating()
						c.num = 0
				if result == 0 or r < result:
					result = r

		return result + 1

	@staticmethod
	def calculate_crc32(data, crc = 0):

		crc = ~crc & 0xFFFFFFFF
		for x in data:
			crc ^= x
			for i in range(32):
				crc = (crc >> 1) ^ 0xEDB88320 if crc & 1 else crc >> 1
		return ~crc & 0xFFFFFFFF

	def calculate_rating(self, estimate = False):

		self.rating = self.solvable()
		if self.rating != 0: return
		self.rating = self.correct()
		if self.rating != 0: return

		if estimate or self.level == Difficulty.Extreme: return

		msb = 0
		result = self.parse_rating()
		i = self.count(0)
		while result > 0:
			msb = msb + 1 if result & 1 else 0
			self.rating += i
			i -= 1
			result >>= 1
		self.rating += msb - 1

	def calculate_level(self):

		if self.level == Difficulty.Easy:    return
		if self.level == Difficulty.Extreme: return
		if self.rating < 0:     self.level = Difficulty.Medium; return
		if self.solved():       self.level = Difficulty.Medium; return
		if not self.simplify(): self.level = Difficulty.Expert; return
		if not self.solved():   self.level = Difficulty.Hard
		else:                   self.level = Difficulty.Medium
		self.again()

	def calculate_signature(self, estimate = False):

		if estimate:
			self.signature = 0
			return

		v = [0] * 10
		for c in self:
			v[c.num] += 1
		l = sorted(c.len()   for c in self)
		r = sorted(c.range() for c in self)
		v.sort()

		self.signature = Sudoku.calculate_crc32(v)
		self.signature = Sudoku.calculate_crc32(l, self.signature)
		self.signature = Sudoku.calculate_crc32(r, self.signature)

	def specify_layout(self, estimate = False):

		self.calculate_rating(estimate)
		self.calculate_level()      # must be after calculate_rating (depends on the rating)
		self.calculate_signature(estimate)

	def read(self, line):

		self.level = Difficulty.Medium
		if len(line) > 82:
			l = ord(line[82]) - ord('0')
			self.level = Difficulty(l) if Difficulty.Any < l <= Difficulty.Extreme else Difficulty.Medium
		if len(line) > 100:
			try:
				self.duration = int(line[100:])
			except ValueError:
				pass
		if len(line) > 0:
			self.init(line[:81])

	def __str__(self):

		return "".join(str(c) for c in self) + "|{:d}:{:2d}:{:3d}:{:8x}:{:d}".format(
			int(self.level), self.len(), self.rating, self.signature, self.duration)

	def load(self, filename = None):

		name = filename if filename is not None else "sudoku.board"
		try:
			with open(name, "r") as file:
				line = file.readline().rstrip("\r\n")
		except OSError:
			return False

		tmp = Sudoku.Backup(self)
		self.read(line)

		return tmp.changed()

	def save(self, filename = None, gametime = 0):

		name = filename if filename is not None else "sudoku.board"
		try:
			with open(name, "w") as file:
				self.duration = gametime
				file.write(str(self) + "\n")
		except OSError:
			return

		pos = 0
		puzzle = list(Sudoku.html)
		text = Sudoku.html
		for c in self:
			pos = text.find(" </td>", pos + 1)
			puzzle[pos] = " 123456789"[c.num]

		try:
			with open(name + ".html", "w") as file:
				file.write("".join(puzzle))
		except OSError:
			return

#------------------------------------------------------------------------------
#                                 GAME: GRAPHICS
#------------------------------------------------------------------------------

class Rect:

	def __init__(self, x, y, width, height):

		self.x      = float(x)
		self.y      = float(y)
		self.width  = float(width)
		self.height = float(height)
		self.left   = self.x
		self.top    = self.y
		self.right  = self.x + self.width
		self.bottom = self.y + self.height
		self.center = (self.left + self.right) / 2
		self.middle = (self.top + self.bottom) / 2

	@staticmethod
	def deflate(r, d):

		return Rect(r.x + d, r.y + d, r.width - d * 2, r.height - d * 2)

	def contains(self, x, y):

		return self.left <= x < self.right and self.top <= y < self.bottom

	def frame(self):

		l, t = round(self.left), round(self.top)
		return pygame.Rect(l, t, round(self.right) - l + 1, round(self.bottom) - t + 1)

class Color:

	White    = 0xFF, 0xFF, 0xFF
	Silver   = 0xC0, 0xC0, 0xC0
	Gray     = 0x80, 0x80, 0x80
	DimGray  = 0x69, 0x69, 0x69
	Black    = 0x00, 0x00, 0x00
	Red      = 0xFF, 0x00, 0x00
	Maroon   = 0x80, 0x00, 0x00
	Orange   = 0xFF, 0xA5, 0x00
	Olive    = 0x80, 0x80, 0x00
	Green    = 0x00, 0x80, 0x00
	Navy     = 0x00, 0x00, 0x80
	Moccasin = 0xFF, 0xE4, 0xB5
	OldLace  = 0xFD, 0xF5, 0xE6

class Alignment(Enum):

	Left   = 0
	Center = 1
	Right  = 2

class Graphics:

	def __init__(self, surface):

		self.surface = surface
		self.texts = {}

	def font(self, height, bold, names, text = None, width = 0):

		# select the largest font with the line height not exceeding the given height
		# (and optionally with the given text not exceeding the given width)
		height = int(height)
		size = height
		while True:
			f = pygame.font.SysFont(names, size, bold)
			if size <= 1 or (f.get_height() <= height and (text is None or f.size(text)[0] <= width)):
				return f
			size -= 1

	def begin(self, color):

		self.surface.fill(color)

	def fill_rect(self, r, color):

		pygame.draw.rect(self.surface, color, r.frame())

	def draw_rect(self, r, color):

		pygame.draw.rect(self.surface, color, r.frame(), 1)

	def draw_polygon(self, points, color):

		pygame.draw.polygon(self.surface, color, points)

	def draw_text(self, r, font, color, align, text):

		key = id(font), text, color
		surf = self.texts.get(key)
		if surf is None:
			surf = font.render(text, True, color)
			if len(self.texts) > 1024:
				self.texts.clear()
			self.texts[key] = surf

		w, h = surf.get_size()
		y = r.middle - h / 2
		if   align == Alignment.Left:  x = r.left
		elif align == Alignment.Right: x = r.right - w
		else:                          x = r.center - w / 2
		self.surface.blit(surf, (round(x), round(y)))

#------------------------------------------------------------------------------

class GameTimer:

	def __init__(self, scale = 1):

		self.scale = scale # number of units per second
		self.start()

	def elapsed(self):

		return time.monotonic() - self.begin

	def continue_if(self, cond, period):

		self.count = float("inf") if cond else 0.0
		self.begin = time.monotonic() - period / self.scale

	def start(self):

		self.count = float("inf")
		self.restart()

	def restart(self):

		self.begin = time.monotonic()

	def reset(self):

		self.count = 0.0

	def stop(self):

		t = self.elapsed()
		if t < self.count:
			self.count = t

	def until(self, period):

		d = period / self.scale
		t = self.elapsed()
		return 0 if t >= d else int((d - t) * self.scale)

	def now(self):

		t = self.elapsed()
		return int((self.count if t >= self.count else t) * self.scale)

#------------------------------------------------------------------------------
#                                  GAME: LAYOUT
#------------------------------------------------------------------------------

Title      = "Sudoku"

Frame      =  0
Margin     =  4
CellSize   = 64
SegSize    = CellSize * 3 + Margin * 2
TabSize    = SegSize  * 3 + Margin * 8
MnuSize    = 12

TAB = Rect(Frame + Margin * 2, Frame + CellSize + Margin * 2, TabSize, TabSize)
MNU = Rect(TAB.right + Margin, TAB.top, SegSize,  TAB.height)
HDR = Rect(Frame, Frame, TAB.left + MNU.right, TAB.top - TAB.left)
FTR = Rect(HDR.left, TAB.bottom, HDR.width, CellSize / 2)
WIN = Rect(0, 0, HDR.left + HDR.right + 1, FTR.bottom + Frame + 1)

Background = Color.Moccasin
Lighted    = Color.OldLace

BigFont    = "tahoma,dejavusans,verdana,arial"
SmallFont  = "arial,liberationsans,dejavusans"

#------------------------------------------------------------------------------
#                                 GAME: CLASSES
#------------------------------------------------------------------------------

class Assistance(IntEnum):

	None_     = 0
	Current   = 1
	Available = 2
	Sure      = 3
	Full      = 4

class Command(IntEnum):

	NoCmd        = -1
	Button0Cmd   =  0
	Button1Cmd   =  1
	Button2Cmd   =  2
	Button3Cmd   =  3
	Button4Cmd   =  4
	Button5Cmd   =  5
	Button6Cmd   =  6
	Button7Cmd   =  7
	Button8Cmd   =  8
	Button9Cmd   =  9
	ClearCellCmd = 10
	SetCellCmd   = 11
	SetSureCmd   = 12
	PrevHelpCmd  = 13
	NextHelpCmd  = 14
	PrevLevelCmd = 15
	NextLevelCmd = 16
	GenerateCmd  = 17
	HighLightCmd = 18
	TimerCmd     = 19
	SolveCmd     = 20
	UndoCmd      = 21
	ClearCmd     = 22
	EditCmd      = 23
	AcceptCmd    = 24
	SaveCmd      = 25
	LoadCmd      = 26
	QuitCmd      = 27

#------------------------------------------------------------------------------

class GameHeader:

	font = None
	tiny = None

	def update(self, gr, info, gametime, length, rating):

		if GameHeader.font is None:
			GameHeader.font = gr.font(HDR.height, False, BigFont)

		if GameHeader.tiny is None:
			GameHeader.tiny = gr.font(HDR.height / 2, False, SmallFont)

		rc = Rect.deflate(HDR, TAB.left)
		gr.draw_text(rc, GameHeader.font, Color.White, Alignment.Left, Title)

		if info is not None:
			gr.draw_text(rc, GameHeader.tiny, Color.White, Alignment.Center, info)

		if gametime >= 0:
			v = "{:d}:{:02d}:{:02d}".format(gametime // 3600, (gametime // 60) % 60, gametime % 60)
		else:
			v = "{:d}:{:03d}".format(length, rating)
		gr.draw_text(rc, GameHeader.font, Color.Silver, Alignment.Right, v)

	def mouseLButton(self, x, y):

		if HDR.contains(x, y):
			return Command.TimerCmd

		return Command.NoCmd

#------------------------------------------------------------------------------

class GameCell:

	font = None
	tiny = None

	def __init__(self, x, y, cell):

		self.r = Rect(x, y, CellSize, CellSize)
		self.cell = cell
		self.focused = False
		self.state = None
		self.info = None

	def prepare(self, state, number, help):

		# the assistance calculations are expensive, so they are only repeated after a change
		if self.state != state:
			self.state = state
			cell = self.cell
			allowed = self.allowed(number, help)
			sure    = cell.sure(number) if allowed and help >= Assistance.Sure else 0
			full    = cell.sure()       if not allowed and help == Assistance.Full else 0
			self.info = allowed, sure, full
		return self.info

	def update(self, gr, state, number, help, focus, light):

		if GameCell.font is None:
			GameCell.font = gr.font(CellSize, True, BigFont)

		if GameCell.tiny is None:
			GameCell.tiny = gr.font(CellSize / 3, True, BigFont)

		cell = self.cell

		if self.focused or (light and cell.linked(focus)):
			gr.fill_rect(Rect.deflate(self.r, Margin), Lighted)

		allowed, sure, full = self.prepare(state, number, help)

		if cell.num != 0:
			f = Color.Red   if help >= Assistance.Current and cell.equal(number) else \
			    Color.Black if cell.immutable                                    else \
			    Color.Navy

			gr.draw_text(self.r, GameCell.font, f, Alignment.Center, str(cell.num))
		elif (self.focused or help > Assistance.Current) and allowed:
			f = Color.Green if help >= Assistance.Sure and sure != 0 else \
			    Color.Gray  if help <= Assistance.Current            else \
			    Color.Orange

			gr.draw_text(self.r, GameCell.tiny, f, Alignment.Center, str(number))
		elif full != 0:
			gr.draw_text(self.r, GameCell.tiny, Color.Green, Alignment.Center, str(full))

		gr.draw_rect(self.r, Color.Black)

	def mouseMove(self, x, y):

		self.focused = self.r.contains(x, y)

	def mouseLeave(self):

		self.focused = False

	def mouseLButton(self, x, y, number, help):

		if self.focused:
			if self.allowed(number, help):
				return Command.SetCellCmd
			elif help == Assistance.Full and number == 0 and self.cell.sure() != 0:
				return Command.SetSureCmd
			elif help != Assistance.None_ or self.cell.num != number:
				return Command(Command.Button0Cmd + self.cell.num)

		return Command.NoCmd

	def mouseRButton(self, x, y):

		if self.focused and self.cell.num != 0:
			return Command.ClearCellCmd

		return Command.NoCmd

	def getCell(self):

		return self.cell if self.focused else None

	def allowed(self, number, help):

		if number == 0 or self.cell.num != 0:
			return False

		if help <= Assistance.Current:
			return True

		if help != Assistance.Full and self.cell.passable(number):
			return True

		if self.cell.allowed(number):
			return True

		return False

#------------------------------------------------------------------------------

class GameTable(list):

	def __init__(self, sudoku):

		for c in sudoku:
			x = TAB.x + (c.pos % 9) * (CellSize + Margin) + (c.pos % 9 // 3) * Margin * 3
			y = TAB.y + (c.pos // 9) * (CellSize + Margin) + (c.pos // 9 // 3) * Margin * 3
			self.append(GameCell(x, y, c))

	def update(self, gr, state, number, help, focus, light):

		for c in self:
			c.update(gr, state, number, help, focus, light)

	def mouseMove(self, x, y):

		for c in self:
			c.mouseMove(x, y)

	def mouseLeave(self):

		for c in self:
			c.mouseLeave()

	def mouseLButton(self, x, y, number, help):

		for c in self:
			cmd = c.mouseLButton(x, y, number, help)
			if cmd != Command.NoCmd:
				return cmd

		return Command.NoCmd

	def mouseRButton(self, x, y):

		for c in self:
			cmd = c.mouseRButton(x, y)
			if cmd != Command.NoCmd:
				return cmd

		return Command.NoCmd

	def getCell(self):

		for c in self:
			cell = c.getCell()
			if cell is not None:
				return cell

		return None

#------------------------------------------------------------------------------

class MenuItem(list):

	font = None

	def __init__(self, num, y, h, info):

		self.r = Rect(MNU.x, y, MNU.width, h)
		self.info = info
		self.num = num
		self.idx = 0
		self.focused = False
		self.timer = GameTimer(100)

	def update(self, gr, x):

		h = round(TabSize / 1.8 / MnuSize)

		if MenuItem.font is None:
			MenuItem.font = gr.font(h, False, SmallFont)

		if self.focused:
			gr.fill_rect(Rect.deflate(self.r, self.timer.until(Margin * 4)), Lighted)

		if len(self) > 1:
			cl = Lighted if not self.focused else Color.Black if x < MNU.center or len(self) == 2 else Background
			cr = Lighted if not self.focused else Background  if x < MNU.center and len(self) != 2 else Color.Black

			a = h / 4
			l = self.r.left  + Margin * 2
			r = self.r.right - Margin * 2
			m = self.r.middle
			gr.draw_polygon(((l, m), (l + a * 2, m - a), (l + a * 2, m + a)), cl)
			gr.draw_polygon(((r, m), (r - a * 2, m - a), (r - a * 2, m + a)), cr)

		gr.draw_text(self.r, MenuItem.font, Color.Black, Alignment.Center, self[self.idx])

	def mouseMove(self, x, y):

		self.focused = self.r.contains(x, y)

	def mouseLeave(self):

		self.focused = False

	def mouseLButton(self, x, y):

		if self.focused:
			self.timer.restart()

			if self.num ==  0: return Command.PrevLevelCmd if x < MNU.center else Command.NextLevelCmd
			if self.num ==  1: return Command.PrevHelpCmd  if x < MNU.center else Command.NextHelpCmd
			if self.num ==  2: return Command.HighLightCmd
			if self.num ==  3: return Command.GenerateCmd
			if self.num ==  4: return Command.SolveCmd
			if self.num ==  5: return Command.UndoCmd
			if self.num ==  6: return Command.ClearCmd
			if self.num ==  7: return Command.EditCmd
			if self.num ==  8: return Command.AcceptCmd
			if self.num ==  9: return Command.SaveCmd
			if self.num == 10: return Command.LoadCmd
			if self.num == 11: return Command.QuitCmd

		return Command.NoCmd

	def getInfo(self):

		return self.info if self.focused else None

	def setIndex(self, i):

		self.idx = int(i)

	def prev(self):

		i = self.idx
		s = len(self)

		if self.num == 0:
			self.idx = s - 1 if i == 0 else 1 if i == s - 1 else 0
		else:
			self.idx = (i + s - 1) % s

		return self.idx

	def next(self):

		i = self.idx
		s = len(self)

		if self.num == 0:
			self.idx = 0 if i == s - 1 else 1 if i == 0 else s - 1
		else:
			self.idx = (i + 1) % s

		return self.idx

#------------------------------------------------------------------------------

class GameMenu(list):

	items = [
		("Change the difficulty level: easy, medium / hard / expert, extreme (keyboard shortcuts: D, PgUp, PgDn)",
			["easy", "medium", "hard", "expert", "extreme"]),
		("Change the assistance level: none, current, available, sure, full (keyboard shortcuts: A, Left, Right)",
			["none", "current", "available", "sure", "full"]),
		("Display the highlight of the entire group of cells: on / off (keyboard shortcut: H)",
			["highlight off", "highlight on"]),
		("Generate or load a new layout (keyboard shortcuts: N, Tab)",
			["new"]),
		("Solve the current layout (keyboard shortcuts: S, Enter)",
			["solve"]),
		("Undo last move or restore the accepted layout (keyboard shortcuts: U, Backspace)",
			["undo"]),
		("Clear the board (keyboard shortcuts: C, Delete)",
			["clear"]),
		("Start editing the current layout (keyboard shortcuts: E, Home)",
			["edit"]),
		("Accept the current layout and finish editing (keyboard shortcuts: T, End)",
			["accept"]),
		("Save the current layout to the file (keyboard shortcuts: V, Insert)",
			["save"]),
		("Load layout from the file (keyboard shortcut: L)",
			["load"]),
		("Quit the game (keyboard shortcuts: Q, Esc)",
			["quit"]),
	]

	def __init__(self):

		h = round(TabSize / 1.2 / MnuSize)
		pos = lambda i: MNU.y + round(i * (MNU.height - h) / (MnuSize - 1))

		for i, (info, names) in enumerate(GameMenu.items):
			self.append(MenuItem(i, pos(i), h, info))
			self[-1].extend(names)

	def update(self, gr, x):

		for m in self:
			m.update(gr, x)

		gr.draw_rect(MNU, Color.Black)

	def mouseMove(self, x, y):

		for m in self:
			m.mouseMove(x, y)

	def mouseLeave(self):

		for m in self:
			m.mouseLeave()

	def mouseLButton(self, x, y):

		for m in self:
			cmd = m.mouseLButton(x, y)
			if cmd != Command.NoCmd:
				return cmd

		return Command.NoCmd

	def getInfo(self):

		for m in self:
			info = m.getInfo()
			if info is not None:
				return info

		return None

#------------------------------------------------------------------------------

class GameFooter:

	fonts = {}

	def update(self, gr, info):

		if info is None:
			info = "Sudoku game, solver and generator"

		font = GameFooter.fonts.get(info)
		if font is None:
			font = gr.font(min(FTR.height, WIN.width / 40), False, SmallFont, info, FTR.width - Margin * 4)
			GameFooter.fonts[info] = font

		gr.draw_text(FTR, font, Color.Gray, Alignment.Center, info)

#------------------------------------------------------------------------------

class Game(Sudoku):

	colors = [Color.Navy, Color.Green, Color.Olive, Color.Maroon, Color.Maroon]

	def __init__(self, surface):

		super().__init__()

		self.gr  = Graphics(surface)
		self.hdr = GameHeader()
		self.tab = GameTable(self)
		self.mnu = GameMenu()
		self.ftr = GameFooter()

		self.timer    = GameTimer()
		self.number   = 0
		self.timer_f  = True
		self.light_f  = False
		self.running  = True
		self.busy     = False
		self.cursor_x = 0
		self.help     = Assistance.None_

		self.generate()
		self.timer.start()

		self.mnu[0].setIndex(self.level)
		self.mnu[1].setIndex(self.help)
		self.mnu[2].setIndex(self.light_f)

	def update(self):

		gr = self.gr

		gametime = self.timer.now() if self.timer_f else -1
		if self.len() < 81:
			info = "unsolvable" if self.rating == -2 else "ambiguous" if self.rating == -1 else None
		else:
			info = "corrupt" if self.corrupt() else "solved"

		x = self.cursor_x

		gr.begin(Background)

		gr.fill_rect(Rect(TAB.x + CellSize * 3 + Margin * 3, TAB.y, Margin * 2, TAB.height), Color.DimGray)
		gr.fill_rect(Rect(TAB.x + CellSize * 6 + Margin * 9, TAB.y, Margin * 2, TAB.height), Color.DimGray)
		gr.fill_rect(Rect(TAB.x, TAB.y + CellSize * 3 + Margin * 3, TAB.width, Margin * 2),  Color.DimGray)
		gr.fill_rect(Rect(TAB.x, TAB.y + CellSize * 6 + Margin * 9, TAB.width, Margin * 2),  Color.DimGray)

		gr.fill_rect(HDR, Game.colors[self.level])

		state = tuple(c.num for c in self), self.number, self.help

		self.hdr.update(gr, info, gametime, self.len(), self.rating)
		self.tab.update(gr, state, self.number, self.help, self.tab.getCell(), self.light_f)
		self.mnu.update(gr, x)
		self.ftr.update(gr, self.mnu.getInfo())

	def mouseMove(self, x, y):

		self.cursor_x = x
		self.tab.mouseMove(x, y)
		self.mnu.mouseMove(x, y)

	def mouseLeave(self):

		self.tab.mouseLeave()
		self.mnu.mouseLeave()

	def mouseLButton(self, x, y):

		self.command(self.hdr.mouseLButton(x, y))
		self.command(self.tab.mouseLButton(x, y, self.number, self.help))
		self.command(self.mnu.mouseLButton(x, y))

	def mouseRButton(self, x, y):

		self.command(self.tab.mouseRButton(x, y))

	def mouseWheel(self, x, y, d):

		cell = self.tab.getCell()

		if (self.help != Assistance.None_ and TAB.contains(x, y)) or (cell is not None and cell.empty()):
			if d < 0:
				n = 1 if self.number == 0 else 1 + (self.number + 0) % 9
			else:
				n = 9 if self.number == 0 else 1 + (self.number + 7) % 9
			self.command(Command(Command.Button0Cmd + n))

	def keyboard(self, key):

		keys = {
			K_0:         Command.Button0Cmd,
			K_1:         Command.Button1Cmd,
			K_2:         Command.Button2Cmd,
			K_3:         Command.Button3Cmd,
			K_4:         Command.Button4Cmd,
			K_5:         Command.Button5Cmd,
			K_6:         Command.Button6Cmd,
			K_7:         Command.Button7Cmd,
			K_8:         Command.Button8Cmd,
			K_9:         Command.Button9Cmd,
			K_LEFT:      Command.PrevHelpCmd,
			K_RIGHT:     Command.NextHelpCmd,
			K_a:         Command.NextHelpCmd,
			K_PAGEDOWN:  Command.PrevLevelCmd,
			K_PAGEUP:    Command.NextLevelCmd,
			K_d:         Command.NextLevelCmd,
			K_TAB:       Command.GenerateCmd,
			K_n:         Command.GenerateCmd,
			K_h:         Command.HighLightCmd,
			K_RETURN:    Command.SolveCmd,
			K_s:         Command.SolveCmd,
			K_BACKSPACE: Command.UndoCmd,
			K_u:         Command.UndoCmd,
			K_DELETE:    Command.ClearCmd,
			K_c:         Command.ClearCmd,
			K_HOME:      Command.EditCmd,
			K_e:         Command.EditCmd,
			K_END:       Command.AcceptCmd,
			K_t:         Command.AcceptCmd,
			K_INSERT:    Command.SaveCmd,
			K_v:         Command.SaveCmd,
			K_l:         Command.LoadCmd,
			K_ESCAPE:    Command.QuitCmd,
			K_q:         Command.QuitCmd,
		}

		self.command(keys.get(key, Command.NoCmd))

	def set_number(self, num = 0):

		if num == 0:
			num = self.number

		if num == 0:
			return False
		if self.help == Assistance.None_:
			return self.set(self.tab.getCell(), num, Force.Direct)
		if self.help != Assistance.Full:
			return self.set(self.tab.getCell(), num, Force.Careful)
		return self.set(self.tab.getCell(), num, Force.Safe)

	def command(self, c):

		if c == Command.NoCmd:
			pass
		elif c == Command.Button0Cmd:
			self.number = 0
		elif Command.Button1Cmd <= c <= Command.Button9Cmd:
			n = c - Command.Button0Cmd
			self.number = 0 if self.number == n else n
		elif c == Command.ClearCellCmd:
			self.number = self.tab.getCell().num
			self.set(self.tab.getCell(), 0)
		elif c == Command.SetCellCmd:
			self.set_number()
		elif c == Command.SetSureCmd:
			self.set_number(self.tab.getCell().sure())
		elif c == Command.PrevHelpCmd:
			self.help = Assistance(self.mnu[1].prev())
		elif c == Command.NextHelpCmd:
			self.help = Assistance(self.mnu[1].next())
		elif c == Command.PrevLevelCmd or c == Command.NextLevelCmd or c == Command.GenerateCmd:
			if   c == Command.PrevLevelCmd: self.level = Difficulty(self.mnu[0].prev())
			elif c == Command.NextLevelCmd: self.level = Difficulty(self.mnu[0].next())
			self.wait()
			self.generate()
			self.number = 0
			self.timer.start()
			self.mnu[0].setIndex(self.level)
		elif c == Command.HighLightCmd:
			self.light_f = not self.light_f
			self.mnu[2].setIndex(self.light_f)
		elif c == Command.TimerCmd:
			self.timer_f = not self.timer_f
		elif c == Command.SolveCmd:
			self.wait()
			self.solve()
			self.number = 0
			if self.len() < 81:
				self.rating = -2
		elif c == Command.UndoCmd:
			self.undo()
		elif c == Command.ClearCmd:
			self.clear()
			self.number = 0
			self.timer.reset()
		elif c == Command.EditCmd:
			self.discard()
			self.timer.reset()
		elif c == Command.AcceptCmd:
			self.wait()
			self.accept()
			if self.timer.now() == 0:
				self.timer.start()
		elif c == Command.SaveCmd:
			seconds = self.timer.now()
			self.save(None, seconds)
		elif c == Command.LoadCmd:
			self.wait()
			if self.load():
				self.number = 0
				self.timer.continue_if(self.rating >= 0, self.duration)
				self.mnu[0].setIndex(self.level)
		elif c == Command.QuitCmd:
			self.running = False

		if self.len() == 81:
			self.number = 0
			if self.solved():
				self.timer.stop()

	def cursor(self, busy):

		# show the busy cursor during time-consuming operations
		if self.busy != busy:
			self.busy = busy
			try:
				pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_WAIT if busy else pygame.SYSTEM_CURSOR_ARROW)
				pygame.event.pump()
			except pygame.error:
				pass

	def wait(self):

		self.cursor(True)

#------------------------------------------------------------------------------
#                                     MAIN
#------------------------------------------------------------------------------

def main():

	os.environ["SDL_VIDEO_CENTERED"] = "1"
	pygame.init()
	pygame.display.set_caption(Title)
	screen = pygame.display.set_mode((round(WIN.width), round(WIN.height)), RESIZABLE | SCALED)

	game = Game(screen)
	clock = pygame.time.Clock()

	while game.running:

		for e in pygame.event.get():
			if   e.type == QUIT:            game.command(Command.QuitCmd)
			elif e.type == MOUSEMOTION:     game.mouseMove(*e.pos)
			elif e.type == WINDOWLEAVE:     game.mouseLeave()
			elif e.type == MOUSEWHEEL:      game.mouseWheel(*pygame.mouse.get_pos(), e.y)
			elif e.type == KEYDOWN:         game.keyboard(e.key)
			elif e.type == MOUSEBUTTONDOWN:
				if   e.button == 1:         game.mouseLButton(*e.pos)
				elif e.button == 3:         game.mouseRButton(*e.pos)

		game.cursor(False)

		if not game.running:
			break

		game.update()
		pygame.display.flip()
		clock.tick(60)

	pygame.quit()

#------------------------------------------------------------------------------

Sudoku.extreme = [
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
]

Sudoku.html = \
"<!doctype html><html><head><title>sudoku</title><style>table{margin-left:auto;margin-right:auto;font-family:Tahoma,Verdana,sans-serif;font-weight:bold;}table,td{border:1px solid black;border-collapse:collapse;}td{width:80px;height:80px;font-size:64px;text-align:center;vertical-align:middle}#top-left{border-top-width:5px;border-left-width:5px;}#top{border-top-width:5px;}#top-right{border-top-width:5px;border-right-width:5px;}#left{border-left-width:5px;}#center{}#right{border-right-width:5px;}#bottom-left{border-bottom-width:5px;border-left-width:5px;}#bottom{border-bottom-width:5px;}#bottom-right{border-bottom-width:5px;border-right-width:5px;}</style></head><body><table><caption><h1>SUDOKU</h1></caption><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr><tr><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td><td id=top-left> </td><td id=top> </td><td id=top-right> </td></tr><tr><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td><td id=left> </td><td id=center> </td><td id=right> </td></tr><tr><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td><td id=bottom-left> </td><td id=bottom> </td><td id=bottom-right> </td></tr></table></body></html>"

#------------------------------------------------------------------------------

if __name__ == "__main__":

	main()
