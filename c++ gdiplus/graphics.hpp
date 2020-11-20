/******************************************************************************

   @file    graphics.hpp
   @author  Rajmund Szymanski
   @date    20.11.2020
   @brief   graphics class

*******************************************************************************

   Copyright (c) 2018-2020 Rajmund Szymanski. All rights reserved.

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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <tchar.h>

class Graphics
{
	HWND                         wnd;
	ULONG_PTR                    token;
	Gdiplus::Bitmap             *bm;
	Gdiplus::Graphics           *gr;
	Gdiplus::Pen                *pen;
	Gdiplus::SolidBrush         *brush;
	std::vector<Gdiplus::Font *> fnt;

public:

	using RectF = Gdiplus::RectF;
	using Color = Gdiplus::Color;
	using Font  = Gdiplus::Font;

	enum Alignment: DWORD
	{
		TopLeft     = MAKELONG(Gdiplus::StringAlignment::StringAlignmentNear,   Gdiplus::StringAlignment::StringAlignmentNear),
		TopRight    = MAKELONG(Gdiplus::StringAlignment::StringAlignmentNear,   Gdiplus::StringAlignment::StringAlignmentFar),
		Top         = MAKELONG(Gdiplus::StringAlignment::StringAlignmentNear,   Gdiplus::StringAlignment::StringAlignmentCenter),
		BottomLeft  = MAKELONG(Gdiplus::StringAlignment::StringAlignmentFar,    Gdiplus::StringAlignment::StringAlignmentNear),
		BottomRight = MAKELONG(Gdiplus::StringAlignment::StringAlignmentFar,    Gdiplus::StringAlignment::StringAlignmentFar),
		Bottom      = MAKELONG(Gdiplus::StringAlignment::StringAlignmentFar,    Gdiplus::StringAlignment::StringAlignmentCenter),
		Left        = MAKELONG(Gdiplus::StringAlignment::StringAlignmentCenter, Gdiplus::StringAlignment::StringAlignmentNear),
		Right       = MAKELONG(Gdiplus::StringAlignment::StringAlignmentCenter, Gdiplus::StringAlignment::StringAlignmentFar),
		Center      = MAKELONG(Gdiplus::StringAlignment::StringAlignmentCenter, Gdiplus::StringAlignment::StringAlignmentCenter),
	};

	struct Rect
	{
		FLOAT x, y, width, height, left, top, right, bottom, center, middle;

		Rect( const auto _x, const auto _y, const auto _width, const auto _height ):
			x     {static_cast<FLOAT>(_x)},
			y     {static_cast<FLOAT>(_y)},
			width {static_cast<FLOAT>(_width)},
			height{static_cast<FLOAT>(_height)},
			left  {x},
			top   {y},
			right {x + width},
			bottom{y + height},
			center{(left + right) / 2},
			middle{(top + bottom) / 2}
			{}

		Rect( const RECT _r ):
			x     {static_cast<FLOAT>(_r.left)},
			y     {static_cast<FLOAT>(_r.top)},
			width {static_cast<FLOAT>(_r.right - _r.left)},
			height{static_cast<FLOAT>(_r.bottom - _r.top)},
			left  {x},
			top   {y},
			right {x + width},
			bottom{y + height},
			center{(left + right) / 2},
			middle{(top + bottom) / 2}
			{}

		operator RECT() const
		{
			return { static_cast<LONG>(std::round(left)),
			         static_cast<LONG>(std::round(top)),
			         static_cast<LONG>(std::round(right)),
			         static_cast<LONG>(std::round(bottom)) };
		}

		operator RectF() const
		{
			return { left, top, width, height };
		}

		void inflate( const auto _d )
		{
			x      -= static_cast<FLOAT>(_d);
			y      -= static_cast<FLOAT>(_d);
			width  += static_cast<FLOAT>(_d * 2);
			height += static_cast<FLOAT>(_d * 2);
		}

		void inflate( const auto _dx, const auto _dy )
		{
			x      -= static_cast<FLOAT>(_dx);
			y      -= static_cast<FLOAT>(_dy);
			width  += static_cast<FLOAT>(_dx * 2);
			height += static_cast<FLOAT>(_dy * 2);
		}

		void inflate( const auto _dx, const auto _dy, const auto _dw, const auto _dh )
		{
			x      -= static_cast<FLOAT>(_dx);
			y      -= static_cast<FLOAT>(_dy);
			width  += static_cast<FLOAT>(_dx + _dw);
			height += static_cast<FLOAT>(_dy + _dh);
		}

		static
		Rect inflate( const Rect &r, const auto _d )
		{
			return { r.x - _d, r.y - _d, r.width + _d * 2, r.height + _d * 2 };
		}

		static
		Rect inflate( const Rect &r, const auto _dx, const auto _dy )
		{
			return { r.x - _dx, r.y - _dy, r.width + _dx * 2, r.height + _dy * 2 };
		}

		static
		Rect inflate( const Rect &r, const auto _dx, const auto _dy, const auto _dw, const auto _dh )
		{
			return { r.x - _dx, r.y - _dy, r.width + _dx + _dw, r.height + _dy + _dh };
		}

		static
		Rect deflate( const Rect &r, const auto _d )
		{
			return { r.x + _d, r.y + _d, r.width - _d * 2, r.height - _d * 2 };
		}

		static
		Rect deflate( const Rect &r, const auto _dx, const auto _dy )
		{
			return { r.x + _dx, r.y + _dy, r.width - _dx * 2, r.height - _dy * 2 };
		}

		static
		Rect deflate( const Rect &r, const auto _dx, const auto _dy, const auto _dw, const auto _dh )
		{
			return { r.x + _dx, r.y + _dy, r.width - _dx - _dw, r.height - _dy - _dh };
		}

		bool contains( const auto _x, const auto _y ) const
		{
			return static_cast<FLOAT>(_x) >= left  &&
			       static_cast<FLOAT>(_x) <  right &&
			       static_cast<FLOAT>(_y) >= top   &&
			       static_cast<FLOAT>(_y) <  bottom;
		}

		bool contains( const POINT _p ) const
		{
			return contains(_p.x, _p.y);
		}
	};

	Graphics(): wnd{NULL}, token{}, bm{nullptr}, gr{nullptr}, pen{nullptr}, brush{nullptr}, fnt{}
	{
	}

	bool init( HWND hWnd )
	{
		wnd = hWnd;
		Gdiplus::GdiplusStartupInput si = {};
		auto status = Gdiplus::GdiplusStartup(&token, &si, NULL);
		if (status != Gdiplus::Status::Ok) return false;

		pen = new Gdiplus::Pen(Color::Black);
		if (pen == nullptr) return false;

		brush = new Gdiplus::SolidBrush(Color::Black);
		if (brush == nullptr) return false;

		return true;
	}

	~Graphics()
	{
		for (auto f: fnt) delete f;

		if (brush != nullptr) delete brush;
		if (pen   != nullptr) delete pen;

		Gdiplus::GdiplusShutdown(token);
	}

	Font *font( const FLOAT h, const INT s, const TCHAR *f )
	{
		Font *font = new Gdiplus::Font(f, h * 0.6f, s);

		if (font != nullptr)
			fnt.push_back(font);

		return font;
	}

	void quit()
	{
		DestroyWindow(wnd);
	}

	void begin( const Color &c )
	{
		RECT rc;
		GetClientRect(wnd, &rc);
		bm = new Gdiplus::Bitmap(rc.right - rc.left, rc.bottom - rc.top);
		gr = Gdiplus::Graphics::FromImage(bm);
		gr->Clear(c);
	}

	void end()
	{
	//	PAINTSTRUCT ps;
		HDC hdc = GetDC(wnd); // BeginPaint(wnd, &ps);
		auto g = Gdiplus::Graphics(hdc);
		g.DrawImage(bm, 0, 0);
		delete gr;
		delete bm;
		ReleaseDC(wnd, hdc); // EndPaint(wnd, &ps);
	}

	void draw_rect( const RectF &r, const Color &c, const FLOAT s = 1 )
	{
		pen->SetColor(c);
		pen->SetWidth(s);

		gr->DrawRectangle(pen, r);
	}

	void fill_rect( const RectF &r, const Color &c )
	{
		brush->SetColor(c);

		gr->FillRectangle(brush, r);
		draw_rect(r, c);
	}

	void draw_text( const RectF &r, Font *f, const Color &c, DWORD a, const TCHAR *t )
	{
		Gdiplus::StringFormat format{};
		format.SetLineAlignment(static_cast<Gdiplus::StringAlignment>(LOWORD(a)));
		format.SetAlignment(static_cast<Gdiplus::StringAlignment>(HIWORD(a)));
		brush->SetColor(c);

		gr->DrawString(t, -1, f, r, &format, brush);
	}

	void draw_char( const RectF &r, Font *f, const Color &c, DWORD a, const TCHAR t )
	{
		Gdiplus::StringFormat format{};
		format.SetLineAlignment(static_cast<Gdiplus::StringAlignment>(LOWORD(a)));
		format.SetAlignment(static_cast<Gdiplus::StringAlignment>(HIWORD(a)));
		brush->SetColor(c);

		gr->DrawString(&t, 1, f, r, &format, brush);
	}
};
