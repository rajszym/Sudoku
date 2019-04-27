'''****************************************************************************

   @file    sudoku.py
   @author  Rajmund Szymanski
   @date    27.04.2019
   @brief   Sudoku game

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

****************************************************************************'''

import os, random, pygame
from pygame.locals import *

White  = 0xFF, 0xFF, 0xFF
Gray   = 0xC0, 0xC0, 0xC0
Red    = 0xFF, 0x00, 0x00
Yellow = 0xFF, 0xFF, 0x00
Green  = 0x00, 0x80, 0x00
Azure  = 0x40, 0x80, 0xFF
Blue   = 0x00, 0x00, 0x80
Dark   = 0x40, 0x40, 0x40
Black  = 0x00, 0x00, 0x00

class Surface(pygame.Surface):

	def drawCell(self, margin, color, fill = True):

		pygame.draw.rect(self, color, (margin, margin, CellWidth - 2 * margin, CellHeight - 2 * margin), not fill)

	def drawMenu(self, margin, color, fill = True):

		pygame.draw.rect(self, color, (margin, margin, MenuWidth - 2 * margin, MenuHeight - 2 * margin), not fill)
	
class Cell:

	def __init__(self, p, n = 0):

		self.pos = p
		self.num = n
		self.lst = []
		self.row = []
		self.col = []
		self.seg = []
		self.immutable = False

	def link(self, c):

		if self != c:

			x = self.pos % 9
			y = self.pos // 9
			z = (y // 3) * 3 + (x // 3)

			cx = c.pos % 9
			cy = c.pos // 9
			cz = (cy // 3) * 3 + (cx // 3)
			
			if x == cx or y == cy or z == cz:
				self.lst.append(c)
				if x == cx: self.col.append(c)
				if y == cy: self.row.append(c)
				if z == cz: self.seg.append(c)

	def fit(self):

		lst = list(range(10))
		lst[self.num] = 0
		for c in self.lst: lst[c.num] = 0
		return [n for n in lst if n > 0]

	def len(self):

		return len(self.fit())

	def slen(self):

		return len(self.fit()) if self.num == 0 else 10

	def glen(self):

		return len(self.fit()) if self.num != 0 else 10

	def set(self, n):

		if self.num == 0 and n in self.fit():
			self.num = n
			return True
		else:
			return False

	def sure(self, n):

		if n == 0:
			for x in self.fit():
				if self.sure(x):
					return True

		if n in self.fit():
			if self.len() == 1:
				return True
		else:
			return False

		num = self.num

		success = True
		self.num = 0
		for c in self.col:
			if c.num == 0 and n in c.fit():
				success = False
				break
		self.num = num
		if success:
			return True

		success = True
		self.num = 0
		for c in self.row:
			if c.num == 0 and n in c.fit():
				success = False
				break
		self.num = num
		if success:
			return True

		success = True
		self.num = 0
		for c in self.seg:
			if c.num == 0 and n in c.fit():
				success = False
				break
		self.num = num
		if success:
			return True

		return False

	def update(self, screen, n, h):

		if   self.num != 0:                 i = self.num if self.num != n else self.num + 10
		elif h == 2 and self.sure(n):       i = 20
		elif h == 0 or not n in self.fit(): i =  0
		elif h == 1 or not self.sure(n):    i = 10
		else:                               i = 20
		x = self.pos % 9
		y = self.pos // 9
		screen.blit(digits[i], (x * CellWidth, y * CellHeight, CellWidth, CellHeight))

class Sudoku(list):

	def __init__(self):

		self.over   = False
		self.button = 0 # current digit
		self.focus  = 0 # focused digit
		self.level  = 0 # dificulty level
		self.help   = 0 # help level
		self.menu   = 0 # focused menu
		self.index  = 0 # extreme index
		self.wait   = False
		self.demo   = False
		for i in range(81):
			self.append(Cell(i))
			for j in range(i):
				self[i].link(self[j])
				self[j].link(self[i])

	def len(self):

		return len([c for c in self if c.num != 0])

	def solved(self):

		return self.len() == 81

	def copy(self):

		temp = Sudoku()
		for i in range(81): temp[i].num = self[i].num
		return temp

	def smin(self, lst):

		p = min(lst, key = lambda x: x.slen())
		return p if p.num == 0 else None

	def clear(self):

		for c in self:
			c.num = 0
			c.immutable = False

	def again(self):

		for c in self:
			if not c.immutable:
				c.num = 0

	def confirm(self):

		for c in self:
			c.immutable = True if c.num != 0 else False

	def solve_next(self, lst):

		if self.demo: self.update()
		pygame.event.pump()
		c = self.smin(lst)
		if c == None: c = self.smin(self)
		if c == None: return True
		lst2 = c.fit()
		random.shuffle(lst2)
		for i in lst2:
			c.num = i
			if self.solve_next(c.lst): return True
		c.num = 0
		return False

	def solve(self):

		self.solve_next(self)

	def solve_test(self, lst):

		c = self.smin(lst)
		if c == None: c = self.smin(self)
		if c == None: return True
		lst2 = c.fit()
		random.shuffle(lst2)
		for i in lst2:
			c.num = i
			if self.solve_test(c.lst): c.num = 0; return True
		c.num = 0
		return False

	def generate_test(self, c):

		num = c.num
		if self.level == 0:
			c.num = 0
			if not c.sure(num):
				c.num = num
				return False
		else:
			for n in c.fit():
				c.num = n
				if self.solve_test(c.lst):
					c.num = num
					return False
			c.num = 0
		return True

	def generate_next(self):

		if self.demo: self.update()
		pygame.event.pump()
		lst = [x for x in self if x.num != 0]
		random.shuffle(lst)
		lst.sort(reverse = True, key = lambda x: len([c for c in x.lst if c.num != 0]))
		for c in lst:
			if self.generate_test(c):
				self.generate_next()
				return

	def random_vertical(self, c1):

		c2 = 3 * (c1 // 3) + (((c1 % 3) + random.randrange(1, 3)) % 3)
		for r in range(0, 81, 9):
			self[r + c1].num, self[r + c2].num = self[r + c2].num, self[r + c1].num

	def random_horizontal(self, r1):

		r2 = 3 * (r1 // 3) + (((r1 % 3) + random.randrange(1, 3)) % 3)
		r1 *= 9; r2 *= 9
		for c in range(9):
			self[r1 + c].num, self[r2 + c].num = self[r2 + c].num, self[r1 + c].num

	def random_column(self, c1):

		c2 = (c1 + random.randrange(1, 3)) % 3
		c1 *= 3; c2 *= 3
		for c in range(3):
			for r in range(c, 81, 9):
				self[r + c1].num, self[r + c2].num = self[r + c2].num, self[r + c1].num

	def random_row(self, r1):

		r2 = (r1 + random.randrange(1, 3)) % 3
		r1 *= 27; r2 *= 27
		for r in range(0, 27, 9):
			for c in range(r, r + 9):
				self[r1 + c].num, self[r2 + c].num = self[r2 + c].num, self[r1 + c].num

	def load_extreme(self):

		self.index = (self.index + random.randrange(1, len(extreme))) % len(extreme)
		self.init(extreme[self.index])
		for r in range(81):
			if self.demo: self.update()
			pygame.event.pump()
			p = random.randrange(24)
			if   p <  9: self.random_vertical(p)
			elif p < 18: self.random_horizontal(p - 9)
			elif p < 21: self.random_column(p - 18)
			else:        self.random_row(p - 21)
		self.confirm()

	def generate(self):

		if self.level == 3:
			self.load_extreme()
		else:
			self.clear()
			self.update()
			self.solve_next(self)
			self.generate_next()
			self.confirm()
			self.calc_level()

	def init(self, txt):

		self.clear()
		txt = txt.replace(".", "0")
		for i in range(81):
			self[i].num = int(txt[i])
		self.confirm()
		self.calc_level()

	def calc_level(self):

		if self.level > 0 and self.level < 3:
			temp = self.copy()
			next = True
			while next:
				next = False
				for c in temp:
					if c.num == 0:
						for n in c.fit():
							if c.sure(n):
								c.num = n
								next = True
								break
			self.level = 1 if temp.len() == 81 else 2

	def onmousebuttonleft(self, x, y):

		x = x // CellWidth
		y = y // CellHeight

		if x < 9 and self.button != 0:
			num = self[y * 9 + x].num
			if num == self.button and self.help == 2:
				lst = [c for c in self if c.num == 0 and num in c.fit() and c.sure(num)]
				for c in lst: c.num = num
			else:
				self[y * 9 + x].set(self.button)
			if self.solved():
				self.button = 0
		elif x == 9:
			num = y + 1
			if self.button != num:
				if not self.solved():
					self.button = num
			elif self.help == 2:
				lst = [c for c in self if c.num == 0 and num in c.fit() and c.sure(num)]
				for c in lst: c.num = num
				if self.solved():
					self.button = 0
		elif x > 9 and self.menu != 0:
			self.button = 0
			self.wait = True
			self.update()
			if   self.menu == 1: self.level = [1, 3, 3, 0][self.level]; self.generate()
			elif self.menu == 2: self.help = (self.help + 1) % 3
			elif self.menu == 3: self.demo = not self.demo
			elif self.menu == 4: self.generate()
			elif self.menu == 5: self.clear()
			elif self.menu == 6: self.confirm()
			elif self.menu == 7: self.again()
			elif self.menu == 8: self.solve()
			elif self.menu == 9: return True
			self.wait = False

		return False

	def onmousebuttonright(self, x, y):

		x = x // CellWidth
		y = y // CellHeight

		if x == 9:
			self.button = 0
		elif x < 9 and not self[y * 9 + x].immutable:
			self[y * 9 + x].num = 0

	def onmousewheelup(self):

		if not self.solved():
			self.button = 9 if self.button == 0 or self.button == 1 else self.button - 1

	def onmousewheeldown(self):

		if not self.solved():
			self.button = 1 if self.button == 0 or self.button == 9 else self.button + 1

	def onmousemotion(self, x, y):

		x = x // CellWidth
		y = y // CellHeight

		if x < 9:
			self.focus = 0
			self.menu = 0
		elif x == 9:
			self.focus = y + 1
			self.menu = 0
		elif x > 9:
			self.focus = 0
			self.menu = y + 1

	def update(self):

		screen = pygame.display.get_surface()
		for c in self:
			c.update(screen, self.button, self.help)
		for p in range(9):
			for m in range(2):
				rect = (p % 3) * 3 * CellWidth + m, (p // 3) * 3 * CellHeight + m, 3 * CellWidth - 2 * m, 3 * CellHeight - 2 * m
				pygame.draw.rect(screen, White, rect, True)
		for p in range(1, 10):
			i = p + (20 if p == self.button else 10 if p == self.focus else 0)
			rect = Width * CellWidth, (p - 1) * CellHeight, CellWidth, CellHeight
			screen.blit(buttons[i], rect)
		for p in range(1, 10):
			if   p == 1:         i = 0 + self.level
			elif p == 2:         i = 4 + self.help
			elif p == 3:         i = 7 if not self.demo else 8
			else:                i = p + 6
			if   p == self.menu: i = 9 if self.wait else i + 16
			rect = (Width + 1) * CellWidth, (p - 1) * CellHeight, MenuWidth, MenuHeight
			screen.blit(menu[i], rect)
		pygame.display.update()

def create_digits():

	global digits

	digits = []
	font = pygame.font.Font(None, CellHeight)
	for i in range(21):
		digits.append(Surface(CellSize))
		digits[i].fill(Dark)
		if   i == 0:
			digits[i].drawCell(2, Blue)
		elif i < 10:
			digits[i].drawCell(2, Green)
			text = font.render(str(i), 1, White)
			rect = text.get_rect()
			rect.center = CellWidth // 2, CellHeight // 2
			digits[i].blit(text, rect)
		elif i == 10:
			digits[i].drawCell(2, Azure)
		elif i < 20:
			digits[i].drawCell(2, Azure)
			text = font.render(str(i - 10), 1, Black)
			rect = text.get_rect()
			rect.center = CellWidth // 2, CellHeight // 2
			digits[i].blit(text, rect)
		elif i == 20:
			digits[i].drawCell(2, Azure)
			digits[i].drawCell(CellWidth // 2 - 4, Black)

def create_buttons():

	global buttons

	buttons = []
	font = pygame.font.Font(None, CellHeight)
	for i in range(10):
		buttons.append(Surface(CellSize))
		buttons[i].fill(White)
		if i > 0:
			text = font.render(str(i), 1, Black)
			rect = text.get_rect()
			rect.center = CellWidth // 2, CellHeight // 2
			buttons[i].blit(text, rect)
	for i in range(10, 20):
		buttons.append(Surface(CellSize))
		buttons[i].fill(White)
		if i > 10:
			buttons[i].drawCell(1, Gray)
			text = font.render(str(i - 10), 1, Black)
			rect = text.get_rect()
			rect.center = CellWidth // 2, CellHeight // 2
			buttons[i].blit(text, rect)
	for i in range(20, 30):
		buttons.append(Surface(CellSize))
		buttons[i].fill(White)
		if i > 10:
			buttons[i].drawCell(1, Red)
			text = font.render(str(i - 20), 1, White)
			rect = text.get_rect()
			rect.center = CellWidth // 2, CellHeight // 2
			buttons[i].blit(text, rect)

def create_menu():

	global menu

	items = ["( EASY )", "( MEDIUM )", "( HARD )", "( EXTREME )",
	         "( CURRENT )", "( AVAILABLE )", "( TIPS )", "( SILENT )", "( LIVE )",
	         "WAIT", "NEXT", "CLEAR", "CONFIRM", "BACK", "SOLVE", "EXIT"]
	menu = []
	font = pygame.font.Font(None, CellHeight // 2)
	for i in range(2 * len(items)):
		menu.append(Surface(MenuSize))
		menu[i].fill(Dark)
		if i == 9:              menu[i].drawMenu(1, Red)
		if i >= len(items): menu[i].drawMenu(1, Gray)
		text = font.render(items[i % len(items)], 1, White if i < len(items) else Dark)
		rect = text.get_rect()
		rect.center = MenuWidth // 2, MenuHeight // 2
		menu[i].blit(text, rect)

def main():

	random.seed()
	create_digits()
	create_buttons()
	create_menu()
	sudoku = Sudoku()
	sudoku.generate()

	while True:

		e = pygame.event.wait()
		if   e.type == QUIT: return
		elif e.type == KEYDOWN:
			if   e.key == K_ESCAPE: return
		elif e.type == MOUSEBUTTONDOWN:
			if   e.button == 1 and sudoku.onmousebuttonleft(*e.pos): return
			elif e.button == 3:    sudoku.onmousebuttonright(*e.pos)
			elif e.button == 4:    sudoku.onmousewheelup()
			elif e.button == 5:    sudoku.onmousewheeldown()
		elif e.type == MOUSEMOTION:
			if   sudoku.over:      sudoku.onmousemotion(*e.pos)
		elif e.type == ACTIVEEVENT:
			sudoku.over = e.gain;
			sudoku.menu = 0
			sudoku.focus = 0;
			
		sudoku.update()

Size       = Width, Height = 9, 9
CellSize   = CellWidth, CellHeight = 64, 64
MenuSize   = MenuWidth, MenuHeight = 3 * CellWidth, CellHeight
ScreenSize = ScreenWidth, ScreenHeight = Width * CellWidth + CellWidth + MenuWidth, Height * CellHeight

extreme = [
"8..........36......7..9.2...5...7.......457.....1...3...1....68..85...1..9....4..", # rating = 3.48
".2.4.37.........32........4.4.2...7.8...5.........1...5.....9...3.9....7..1..86..", # rating = 3.38
"1.......2.9.4...5...6...7...5.9.3.......7.......85..4.7.....6...3...9.8...2.....1", # rating = 3.33
".......39.....1..5..3.5.8....8.9...6.7...2...1..4.......9.8..5..2....6..4..7.....", # rating = 3.33
"4.16...7....4....83..5.........8.....4....1.6..7....32.9...3.......5.49.2...6...5", # rating = 3.09
".47.......3...98.....2..7......6.1..4....3..6.16..4...5..38..7.8.24...5....9.....", # rating = 3.00
"5..8......69...1.4........7725.....6..........14...2......5.7.....62.4.13..9....2", # rating = 3.00
".2.48.............65...1........8.6..1..53.9....2.47...6.....1.5......43...9.28..", # rating = 3.00
"...1..23..1.5..8.....6.4.5...9.......8..1.94..718..3.....2.....6.5..........73...", # rating = 3.00
"....3.6..6....7..4......72.27..91....5..........3..9.1....5..98.3.7..4..5..4.....", # rating = 3.00
".....96..3.........458...1..1..9.26..2..619.....7.......734.....3....5..4...7....", # rating = 3.00
"..6.4.75..5..269......58...7....4........53...2...9...19....68........1.83......4", # rating = 2.96
"...6...3..39...7.24........2.31...5.1............28..4........99.25.68....8..4..7", # rating = 2.96
"7...3.9....2..4....9.....566....72....4......5...1..4....5..3..1......8..362...9.", # rating = 2.96
"..1......83........9..83.......91.53....5.2....46.29..1825........9....2......76.", # rating = 2.92
".....3..5.....627......1.9.4.5........6...8...1..2.3...9..4....1..7..9..34.8....6", # rating = 2.91
"85.2.......27..1..9........5........61..3..8.7.84..513.....975....8.4...........6", # rating = 2.88
"86..1.........78....96...2...8.4..1.73....4....2..3...1..43.5..3...25.9..........", # rating = 2.83
"65.......4...9..16..2...83......79..........4.1..52.7...6...75...9.15..2....6....", # rating = 2.83
"..9.37.6...2........71.598...............93..3.1......7..48.5..2.4...7.....7.6.1.", # rating = 2.83
"..78134...1375..6......9...24..78......5...............7....31....1..64..8.9.....", # rating = 2.83
".6.13....1..6.9.74..9..8...93...1.........8.5..5.62.....7..6....9.....4...4...1.7", # rating = 2.80
"7...5.......8....9.25.1..6.....7.9....32.1...6.....18...4...6........72.83.6...9.", # rating = 2.79
".3....19.....3..2.7..5.8....1....4..5...4......8.61.3....2...5...4.....9..6..97.2", # rating = 2.79
"....2...7.1..4..6....1....8..1...7..8...9....46...3..963...5......4.7.9...5.3.8..", # rating = 2.75
"3....6.2.7.1...6....9.......3......61.7.....4..5..389.....487..4..3.95.....1....8", # rating = 2.64
"....8.4.5.....371.9..2..6..8.614....1...25..............7.......9....86.5839...4.", # rating = 2.64
"....2...375...3......769...8......1.1......97.6..9.5.2....741.9.8....2..6...3....", # rating = 2.64
"...4......4..7526.57..16.4.......3.....5...9.3.5..8..21.......6.5.3....84.7..2...", # rating = 2.62
".3...1...9625........4...2..8...29........67.6.73...1..76..5.8.....1....5.1..8..4", # rating = 2.54
]

if __name__ == "__main__":

	os.environ["SDL_VIDEO_CENTERED"] = "1"
	pygame.init()
	pygame.display.set_mode(ScreenSize)
	pygame.display.set_caption("SUDOKU")
	main()
	pygame.quit()
