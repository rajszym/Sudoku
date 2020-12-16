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
#include <vector>
#include <tchar.h>

class Graphics
{
	HWND               wnd;
	PAINTSTRUCT        ps;
	HDC                hdc;
	HDC                cdc;
	HBITMAP            bmp;
	std::vector<HFONT> fnt;

public:

	using Font = HFONT;

	enum Color: COLORREF
	{
		Maroon               = RGB(0x80, 0x00, 0x00),
		DarkRed              = RGB(0x8b, 0x00, 0x00),
		Brown                = RGB(0xa5, 0x2a, 0x2a),
		Firebrick            = RGB(0xb2, 0x22, 0x22),
		Crimson              = RGB(0xdc, 0x14, 0x3c),
		Red                  = RGB(0xff, 0x00, 0x00),
		Tomato               = RGB(0xff, 0x63, 0x47),
		Coral                = RGB(0xff, 0x7f, 0x50),
		IndianRed            = RGB(0xcd, 0x5c, 0x5c),
		LightCoral           = RGB(0xf0, 0x80, 0x80),
		DarkSalmon           = RGB(0xe9, 0x96, 0x7a),
		Salmon               = RGB(0xfa, 0x80, 0x72),
		LightSalmon          = RGB(0xff, 0xa0, 0x7a),
		OrangeRed            = RGB(0xff, 0x45, 0x00),
		DarkOrange           = RGB(0xff, 0x8c, 0x00),
		Orange               = RGB(0xff, 0xa5, 0x00),
		Gold                 = RGB(0xff, 0xd7, 0x00),
		DarkGoldenRod        = RGB(0xb8, 0x86, 0x0b),
		GoldenRod            = RGB(0xda, 0xa5, 0x20),
		PaleGoldenRod        = RGB(0xee, 0xe8, 0xaa),
		DarkKhaki            = RGB(0xbd, 0xb7, 0x6b),
		Khaki                = RGB(0xf0, 0xe6, 0x8c),
		Olive                = RGB(0x80, 0x80, 0x00),
		Yellow               = RGB(0xff, 0xff, 0x00),
		YellowGreen          = RGB(0x9a, 0xcd, 0x32),
		DarkOliveGreen       = RGB(0x55, 0x6b, 0x2f),
		OliveDrab            = RGB(0x6b, 0x8e, 0x23),
		LawnGreen            = RGB(0x7c, 0xfc, 0x00),
		ChartReuse           = RGB(0x7f, 0xff, 0x00),
		GreenYellow          = RGB(0xad, 0xff, 0x2f),
		DarkGreen            = RGB(0x00, 0x64, 0x00),
		Green                = RGB(0x00, 0x80, 0x00),
		ForestGreen          = RGB(0x22, 0x8b, 0x22),
		Lime                 = RGB(0x00, 0xff, 0x00),
		LimeGreen            = RGB(0x32, 0xcd, 0x32),
		LightGreen           = RGB(0x90, 0xee, 0x90),
		PaleGreen            = RGB(0x98, 0xfb, 0x98),
		DarkSeaGreen         = RGB(0x8f, 0xbc, 0x8f),
		MediumSpringGreen    = RGB(0x00, 0xfa, 0x9a),
		SpringGreen          = RGB(0x00, 0xff, 0x7f),
		SeaGreen             = RGB(0x2e, 0x8b, 0x57),
		MediumAquaMarine     = RGB(0x66, 0xcd, 0xaa),
		MediumSeaGreen       = RGB(0x3c, 0xb3, 0x71),
		LightSeaGreen        = RGB(0x20, 0xb2, 0xaa),
		DarkSlateGray        = RGB(0x2f, 0x4f, 0x4f),
		Teal                 = RGB(0x00, 0x80, 0x80),
		DarkCyan             = RGB(0x00, 0x8b, 0x8b),
		Aqua                 = RGB(0x00, 0xff, 0xff),
		Cyan                 = RGB(0x00, 0xff, 0xff),
		LightCyan            = RGB(0xe0, 0xff, 0xff),
		DarkTurquoise        = RGB(0x00, 0xce, 0xd1),
		Turquoise            = RGB(0x40, 0xe0, 0xd0),
		MediumTurquoise      = RGB(0x48, 0xd1, 0xcc),
		PaleTurquoise        = RGB(0xaf, 0xee, 0xee),
		AquaMarine           = RGB(0x7f, 0xff, 0xd4),
		PowderBlue           = RGB(0xb0, 0xe0, 0xe6),
		CadetBlue            = RGB(0x5f, 0x9e, 0xa0),
		SteelBlue            = RGB(0x46, 0x82, 0xb4),
		CornFlowerBlue       = RGB(0x64, 0x95, 0xed),
		DeepSkyBlue          = RGB(0x00, 0xbf, 0xff),
		DodgerBlue           = RGB(0x1e, 0x90, 0xff),
		LightBlue            = RGB(0xad, 0xd8, 0xe6),
		SkyBlue              = RGB(0x87, 0xce, 0xeb),
		LightSkyBlue         = RGB(0x87, 0xce, 0xfa),
		MidnightBlue         = RGB(0x19, 0x19, 0x70),
		Navy                 = RGB(0x00, 0x00, 0x80),
		DarkBlue             = RGB(0x00, 0x00, 0x8b),
		MediumBlue           = RGB(0x00, 0x00, 0xcd),
		Blue                 = RGB(0x00, 0x00, 0xff),
		RoyalBlue            = RGB(0x41, 0x69, 0xe1),
		BlueViolet           = RGB(0x8a, 0x2b, 0xe2),
		Indigo               = RGB(0x4b, 0x00, 0x82),
		DarkSlateBlue        = RGB(0x48, 0x3d, 0x8b),
		SlateBlue            = RGB(0x6a, 0x5a, 0xcd),
		MediumSlateBlue      = RGB(0x7b, 0x68, 0xee),
		MediumPurple         = RGB(0x93, 0x70, 0xdb),
		DarkMagenta          = RGB(0x8b, 0x00, 0x8b),
		DarkViolet           = RGB(0x94, 0x00, 0xd3),
		DarkOrchid           = RGB(0x99, 0x32, 0xcc),
		MediumOrchid         = RGB(0xba, 0x55, 0xd3),
		Purple               = RGB(0x80, 0x00, 0x80),
		Thistle              = RGB(0xd8, 0xbf, 0xd8),
		Plum                 = RGB(0xdd, 0xa0, 0xdd),
		Violet               = RGB(0xee, 0x82, 0xee),
		Magenta              = RGB(0xff, 0x00, 0xff),
		Fuchsia              = RGB(0xff, 0x00, 0xff),
		Orchid               = RGB(0xda, 0x70, 0xd6),
		MediumVioletRed      = RGB(0xc7, 0x15, 0x85),
		PaleVioletRed        = RGB(0xdb, 0x70, 0x93),
		DeepPink             = RGB(0xff, 0x14, 0x93),
		HotPink              = RGB(0xff, 0x69, 0xb4),
		LightPink            = RGB(0xff, 0xb6, 0xc1),
		Pink                 = RGB(0xff, 0xc0, 0xcb),
		AntiqueWhite         = RGB(0xfa, 0xeb, 0xd7),
		Beige                = RGB(0xf5, 0xf5, 0xdc),
		Bisque               = RGB(0xff, 0xe4, 0xc4),
		BlanchedAlmond       = RGB(0xff, 0xeb, 0xcd),
		Wheat                = RGB(0xf5, 0xde, 0xb3),
		CornSilk             = RGB(0xff, 0xf8, 0xdc),
		LemonChiffon         = RGB(0xff, 0xfa, 0xcd),
		LightGoldenRodYellow = RGB(0xfa, 0xfa, 0xd2),
		LightYellow          = RGB(0xff, 0xff, 0xe0),
		SaddleBrown          = RGB(0x8b, 0x45, 0x13),
		Sienna               = RGB(0xa0, 0x52, 0x2d),
		Chocolate            = RGB(0xd2, 0x69, 0x1e),
		Peru                 = RGB(0xcd, 0x85, 0x3f),
		SandyBrown           = RGB(0xf4, 0xa4, 0x60),
		BurlyWood            = RGB(0xde, 0xb8, 0x87),
		Tan                  = RGB(0xd2, 0xb4, 0x8c),
		RosyBrown            = RGB(0xbc, 0x8f, 0x8f),
		Moccasin             = RGB(0xff, 0xe4, 0xb5),
		NavajoWhite          = RGB(0xff, 0xde, 0xad),
		PeachPuff            = RGB(0xff, 0xda, 0xb9),
		MistyRose            = RGB(0xff, 0xe4, 0xe1),
		LavenderBlush        = RGB(0xff, 0xf0, 0xf5),
		Linen                = RGB(0xfa, 0xf0, 0xe6),
		OldLace              = RGB(0xfd, 0xf5, 0xe6),
		PapayaWhip           = RGB(0xff, 0xef, 0xd5),
		SeaShell             = RGB(0xff, 0xf5, 0xee),
		MintCream            = RGB(0xf5, 0xff, 0xfa),
		SlateGray            = RGB(0x70, 0x80, 0x90),
		LightSlateGray       = RGB(0x77, 0x88, 0x99),
		LightSteelBlue       = RGB(0xb0, 0xc4, 0xde),
		Lavender             = RGB(0xe6, 0xe6, 0xfa),
		FloralWhite          = RGB(0xff, 0xfa, 0xf0),
		AliceBlue            = RGB(0xf0, 0xf8, 0xff),
		GhostWhite           = RGB(0xf8, 0xf8, 0xff),
		Honeydew             = RGB(0xf0, 0xff, 0xf0),
		Ivory                = RGB(0xff, 0xff, 0xf0),
		Azure                = RGB(0xf0, 0xff, 0xff),
		Snow                 = RGB(0xff, 0xfa, 0xfa),
		Black                = RGB(0x00, 0x00, 0x00),
		DimGray              = RGB(0x69, 0x69, 0x69),
		Gray                 = RGB(0x80, 0x80, 0x80),
		DarkGray             = RGB(0xa9, 0xa9, 0xa9),
		Silver               = RGB(0xc0, 0xc0, 0xc0),
		LightGray            = RGB(0xd3, 0xd3, 0xd3),
		Gainsboro            = RGB(0xdc, 0xdc, 0xdc),
		WhiteSmoke           = RGB(0xf5, 0xf5, 0xf5),
		White                = RGB(0xff, 0xff, 0xff),
	};

	enum Alignment: UINT
	{
		TopLeft     = DT_TOP     | DT_LEFT,
		Top         = DT_TOP     | DT_CENTER,
		TopRight    = DT_TOP     | DT_RIGHT,
		Left        = DT_VCENTER | DT_LEFT,
		Center      = DT_VCENTER | DT_CENTER,
		Right       = DT_VCENTER | DT_RIGHT,
		BottomLeft  = DT_BOTTOM  | DT_LEFT,
		Bottom      = DT_BOTTOM  | DT_CENTER,
		BottomRight = DT_BOTTOM  | DT_RIGHT,
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

	Graphics(): wnd{NULL}, ps{}, hdc{NULL}, cdc{NULL}, bmp{NULL}
	{
	}

	bool init( HWND hWnd )
	{
		wnd = hWnd;

		return true;
	}

	~Graphics()
	{
		for (auto f: fnt) DeleteObject(f);
	}

	HFONT font( const INT h, const INT w, const BYTE p, const TCHAR *f )
	{
		LOGFONT lf = {};
		lf.lfHeight         = h;
		lf.lfWidth          = 0;
		lf.lfEscapement     = 0;
		lf.lfOrientation    = 0;
		lf.lfWeight         = w;
		lf.lfItalic         = FALSE;
		lf.lfUnderline      = FALSE;
		lf.lfStrikeOut      = FALSE;
		lf.lfCharSet        = DEFAULT_CHARSET;
		lf.lfOutPrecision   = OUT_TT_PRECIS;
		lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
		lf.lfQuality        = CLEARTYPE_QUALITY;
		lf.lfPitchAndFamily = p;
		_tcscpy(lf.lfFaceName, f);

		HFONT font = CreateFontIndirect(&lf);
		if (font != NULL)
			fnt.push_back(font);

		return font;
	}

	void quit()
	{
		DestroyWindow(wnd);
	}

	void begin( const Color )
	{
		RECT rc;
		GetClientRect(wnd, &rc);
		hdc = GetDC(wnd); // hdc = BeginPaint(wnd, &ps);
		cdc = CreateCompatibleDC(hdc);
		bmp = CreateCompatibleBitmap(cdc, rc.right - rc.left, rc.bottom - rc.top);
		SelectObject(cdc, reinterpret_cast<HGDIOBJ>(DC_PEN));
		SelectObject(cdc, reinterpret_cast<HGDIOBJ>(DC_BRUSH));
		SelectObject(cdc, bmp);
	}

	void end()
	{
		RECT rc;
		GetClientRect(wnd, &rc);
		BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, cdc, 0, 0, SRCCOPY);
		DeleteObject(bmp);
		DeleteDC(cdc);
		ReleaseDC(wnd, hdc); // EndPaint(wnd, &ps);
	}

	void draw_rect( const Rect &r, const Color c, const INT = 0 )
	{
		SetDCPenColor(cdc, c);
		SelectObject(cdc, reinterpret_cast<HGDIOBJ>(NULL_BRUSH));

		RECT rc = r;
		Rectangle(cdc, rc.left, rc.top, rc.right, rc.bottom);

		SelectObject(cdc, reinterpret_cast<HGDIOBJ>(DC_BRUSH));
	}

	void fill_rect( const Rect &r, const Color c )
	{
		SetDCPenColor(cdc, c);
		SetDCBrushColor(cdc, c);

		RECT rc = r;
		Rectangle(cdc, rc.left, rc.top, rc.right, rc.bottom);
	}

	void draw_text( const Rect &r, Font f, const Color c, UINT a, const TCHAR *t )
	{
		SelectObject(cdc, f);
		SetTextColor(cdc, c);

		RECT rc = r;
		DrawText(cdc, t, -1, &rc, a | DT_NOCLIP);
	}

	void draw_char( const Rect &r, Font f, const Color c, UINT a, const TCHAR t )
	{
		SelectObject(cdc, f);
		SetTextColor(cdc, c);

		RECT rc = r;
		DrawText(cdc, &t, 1, &rc, a | DT_NOCLIP);
	}
};
