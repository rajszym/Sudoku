# Sudoku
Sudoku game, solver and generator using C++, C# and Python.

### Features
- difficulty level: easy, medium / hard / expert, extreme.
- assistance level: none, view only the current digit, view also available moves, view also sure moves, full assistance.
- new: generate or load a new layout.
- solve: solve the current layout.
- undo: undo last move or return to the accepted layout.
- clear: clear the board.
- edit: start editing the current layout.
- accept: accept the current layout and finish editing.
- save: save the current layout to the file "*sudoku.board*".
- load: load layout from the file "*sudoku.board*".
- quit: quit the game.

### Python version
The Python version (folder "*python*") requires [pygame](https://www.pygame.org):
```
pip install pygame
python sudoku.py
```

### C# version (Avalonia)
The C# version (folder "*avalonia*") requires the [.NET 10 SDK](https://dotnet.microsoft.com/download); the [Avalonia](https://avaloniaui.net) packages are downloaded automatically. It runs on Windows, Linux and macOS:
```
dotnet run
```
A single executable file that does not need .NET installed:
```
dotnet publish -c Release -r win-x64 --self-contained -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -p:DebugType=none
```
(use `linux-x64` or `osx-arm64` instead of `win-x64` for the other systems)

### [License](https://github.com/rajszym/Sudoku/blob/master/LICENSE)
The project is licensed under the MIT license.
