all: sudoku.cpp sudoku.hpp console.hpp gametimer.hpp
	@g++ -std=gnu++2a -O2 -Wall -Wextra -Wshadow -Wpedantic sudoku.cpp -o sudoku
unicode: sudoku.cpp sudoku.hpp console.hpp gametimer.hpp
	@g++ -std=gnu++2a -O2 -municode -DUNICODE -D_UNICODE -Wall -Wextra -Wshadow -Wpedantic sudoku.cpp -ld3d9 -ld3dx9 -o sudoku
clean:
	@rm -f sudoku.exe
