/******************************************************************************

   @file    Program.cs
   @author  Rajmund Szymanski
   @date    12.10.2026
   @brief   Sudoku game, solver and generator: application and main window

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

using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Styling;
using Avalonia.Themes.Fluent;

namespace SudokuGame;

sealed class MainWindow : Window
{
	public MainWindow()
	{
		Title = Layout.Title;
		Width = Layout.WIN.Width;
		Height = Layout.WIN.Height;
		WindowStartupLocation = WindowStartupLocation.CenterScreen;
		Background = new SolidColorBrush(Layout.Background);

		// the game is drawn in its own size and scaled to the size of the window
		var view = new GameView();
		Content = new Viewbox { Stretch = Stretch.Uniform, Child = view };

		// tunnel: the keys (e.g. Tab) are handled before the keyboard navigation
		AddHandler(KeyDownEvent, ( _, e ) =>
		{
			if (view.Keyboard(e.Key))
				e.Handled = true;
		}, RoutingStrategies.Tunnel);
	}
}

sealed class App : Application
{
	public override void Initialize()
	{
		RequestedThemeVariant = ThemeVariant.Light;
		Styles.Add(new FluentTheme());
	}

	public override void OnFrameworkInitializationCompleted()
	{
		if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
			desktop.MainWindow = new MainWindow();

		base.OnFrameworkInitializationCompleted();
	}
}

static class Program
{
	[STAThread]
	public static void Main( string[] args ) => BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);

	public static AppBuilder BuildAvaloniaApp() => AppBuilder.Configure<App>().UsePlatformDetect().WithInterFont();
}
