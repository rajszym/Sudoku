/******************************************************************************

   @file    graphics.hpp
   @author  Rajmund Szymanski
   @date    03.11.2020
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
#include <d3dx9.h>
#include <vector>
#include <tchar.h>

class Graphics
{
	class Vertex
	{
		D3DXVECTOR4 v;
		D3DCOLOR    c;

	public:

		Vertex():                                          v{ 0,  0, 0, 1}, c{Color::Black} {}
		Vertex(FLOAT _x, FLOAT _y):                        v{_x, _y, 0, 1}, c{Color::Black} {}
		Vertex(FLOAT _x, FLOAT _y, D3DCOLOR _c):           v{_x, _y, 0, 1}, c{_c}           {}
		Vertex(FLOAT _x, FLOAT _y, D3DCOLOR _c, DWORD _a): v{_x, _y, 0, 1}, c{_c^(~_a<<24)} {}
	};

	HWND                    wnd;
	LPDIRECT3D9             d3d;
	LPDIRECT3DDEVICE9       dev;
	std::vector<ID3DXFont *> fnt;

	void done()
	{
		for (ID3DXFont *f: fnt) f->Release();
		fnt.clear();

		if (dev != NULL) { dev->Release(); dev = NULL; }
		if (d3d != NULL) { d3d->Release(); d3d = NULL; }
	}

public:

	using Font  = ID3DXFont;

	enum Color: D3DCOLOR
	{
		Maroon               = D3DCOLOR_XRGB(0x80, 0x00, 0x00),
		DarkRed              = D3DCOLOR_XRGB(0x8b, 0x00, 0x00),
		Brown                = D3DCOLOR_XRGB(0xa5, 0x2a, 0x2a),
		Firebrick            = D3DCOLOR_XRGB(0xb2, 0x22, 0x22),
		Crimson              = D3DCOLOR_XRGB(0xdc, 0x14, 0x3c),
		Red                  = D3DCOLOR_XRGB(0xff, 0x00, 0x00),
		Tomato               = D3DCOLOR_XRGB(0xff, 0x63, 0x47),
		Coral                = D3DCOLOR_XRGB(0xff, 0x7f, 0x50),
		IndianRed            = D3DCOLOR_XRGB(0xcd, 0x5c, 0x5c),
		LightCoral           = D3DCOLOR_XRGB(0xf0, 0x80, 0x80),
		DarkSalmon           = D3DCOLOR_XRGB(0xe9, 0x96, 0x7a),
		Salmon               = D3DCOLOR_XRGB(0xfa, 0x80, 0x72),
		LightSalmon          = D3DCOLOR_XRGB(0xff, 0xa0, 0x7a),
		OrangeRed            = D3DCOLOR_XRGB(0xff, 0x45, 0x00),
		DarkOrange           = D3DCOLOR_XRGB(0xff, 0x8c, 0x00),
		Orange               = D3DCOLOR_XRGB(0xff, 0xa5, 0x00),
		Gold                 = D3DCOLOR_XRGB(0xff, 0xd7, 0x00),
		DarkGoldenRod        = D3DCOLOR_XRGB(0xb8, 0x86, 0x0b),
		GoldenRod            = D3DCOLOR_XRGB(0xda, 0xa5, 0x20),
		PaleGoldenRod        = D3DCOLOR_XRGB(0xee, 0xe8, 0xaa),
		DarkKhaki            = D3DCOLOR_XRGB(0xbd, 0xb7, 0x6b),
		Khaki                = D3DCOLOR_XRGB(0xf0, 0xe6, 0x8c),
		Olive                = D3DCOLOR_XRGB(0x80, 0x80, 0x00),
		Yellow               = D3DCOLOR_XRGB(0xff, 0xff, 0x00),
		YellowGreen          = D3DCOLOR_XRGB(0x9a, 0xcd, 0x32),
		DarkOliveGreen       = D3DCOLOR_XRGB(0x55, 0x6b, 0x2f),
		OliveDrab            = D3DCOLOR_XRGB(0x6b, 0x8e, 0x23),
		LawnGreen            = D3DCOLOR_XRGB(0x7c, 0xfc, 0x00),
		ChartReuse           = D3DCOLOR_XRGB(0x7f, 0xff, 0x00),
		GreenYellow          = D3DCOLOR_XRGB(0xad, 0xff, 0x2f),
		DarkGreen            = D3DCOLOR_XRGB(0x00, 0x64, 0x00),
		Green                = D3DCOLOR_XRGB(0x00, 0x80, 0x00),
		ForestGreen          = D3DCOLOR_XRGB(0x22, 0x8b, 0x22),
		Lime                 = D3DCOLOR_XRGB(0x00, 0xff, 0x00),
		LimeGreen            = D3DCOLOR_XRGB(0x32, 0xcd, 0x32),
		LightGreen           = D3DCOLOR_XRGB(0x90, 0xee, 0x90),
		PaleGreen            = D3DCOLOR_XRGB(0x98, 0xfb, 0x98),
		DarkSeaGreen         = D3DCOLOR_XRGB(0x8f, 0xbc, 0x8f),
		MediumSpringGreen    = D3DCOLOR_XRGB(0x00, 0xfa, 0x9a),
		SpringGreen          = D3DCOLOR_XRGB(0x00, 0xff, 0x7f),
		SeaGreen             = D3DCOLOR_XRGB(0x2e, 0x8b, 0x57),
		MediumAquaMarine     = D3DCOLOR_XRGB(0x66, 0xcd, 0xaa),
		MediumSeaGreen       = D3DCOLOR_XRGB(0x3c, 0xb3, 0x71),
		LightSeaGreen        = D3DCOLOR_XRGB(0x20, 0xb2, 0xaa),
		DarkSlateGray        = D3DCOLOR_XRGB(0x2f, 0x4f, 0x4f),
		Teal                 = D3DCOLOR_XRGB(0x00, 0x80, 0x80),
		DarkCyan             = D3DCOLOR_XRGB(0x00, 0x8b, 0x8b),
		Aqua                 = D3DCOLOR_XRGB(0x00, 0xff, 0xff),
		Cyan                 = D3DCOLOR_XRGB(0x00, 0xff, 0xff),
		LightCyan            = D3DCOLOR_XRGB(0xe0, 0xff, 0xff),
		DarkTurquoise        = D3DCOLOR_XRGB(0x00, 0xce, 0xd1),
		Turquoise            = D3DCOLOR_XRGB(0x40, 0xe0, 0xd0),
		MediumTurquoise      = D3DCOLOR_XRGB(0x48, 0xd1, 0xcc),
		PaleTurquoise        = D3DCOLOR_XRGB(0xaf, 0xee, 0xee),
		AquaMarine           = D3DCOLOR_XRGB(0x7f, 0xff, 0xd4),
		PowderBlue           = D3DCOLOR_XRGB(0xb0, 0xe0, 0xe6),
		CadetBlue            = D3DCOLOR_XRGB(0x5f, 0x9e, 0xa0),
		SteelBlue            = D3DCOLOR_XRGB(0x46, 0x82, 0xb4),
		CornFlowerBlue       = D3DCOLOR_XRGB(0x64, 0x95, 0xed),
		DeepSkyBlue          = D3DCOLOR_XRGB(0x00, 0xbf, 0xff),
		DodgerBlue           = D3DCOLOR_XRGB(0x1e, 0x90, 0xff),
		LightBlue            = D3DCOLOR_XRGB(0xad, 0xd8, 0xe6),
		SkyBlue              = D3DCOLOR_XRGB(0x87, 0xce, 0xeb),
		LightSkyBlue         = D3DCOLOR_XRGB(0x87, 0xce, 0xfa),
		MidnightBlue         = D3DCOLOR_XRGB(0x19, 0x19, 0x70),
		Navy                 = D3DCOLOR_XRGB(0x00, 0x00, 0x80),
		DarkBlue             = D3DCOLOR_XRGB(0x00, 0x00, 0x8b),
		MediumBlue           = D3DCOLOR_XRGB(0x00, 0x00, 0xcd),
		Blue                 = D3DCOLOR_XRGB(0x00, 0x00, 0xff),
		RoyalBlue            = D3DCOLOR_XRGB(0x41, 0x69, 0xe1),
		BlueViolet           = D3DCOLOR_XRGB(0x8a, 0x2b, 0xe2),
		Indigo               = D3DCOLOR_XRGB(0x4b, 0x00, 0x82),
		DarkSlateBlue        = D3DCOLOR_XRGB(0x48, 0x3d, 0x8b),
		SlateBlue            = D3DCOLOR_XRGB(0x6a, 0x5a, 0xcd),
		MediumSlateBlue      = D3DCOLOR_XRGB(0x7b, 0x68, 0xee),
		MediumPurple         = D3DCOLOR_XRGB(0x93, 0x70, 0xdb),
		DarkMagenta          = D3DCOLOR_XRGB(0x8b, 0x00, 0x8b),
		DarkViolet           = D3DCOLOR_XRGB(0x94, 0x00, 0xd3),
		DarkOrchid           = D3DCOLOR_XRGB(0x99, 0x32, 0xcc),
		MediumOrchid         = D3DCOLOR_XRGB(0xba, 0x55, 0xd3),
		Purple               = D3DCOLOR_XRGB(0x80, 0x00, 0x80),
		Thistle              = D3DCOLOR_XRGB(0xd8, 0xbf, 0xd8),
		Plum                 = D3DCOLOR_XRGB(0xdd, 0xa0, 0xdd),
		Violet               = D3DCOLOR_XRGB(0xee, 0x82, 0xee),
		Magenta              = D3DCOLOR_XRGB(0xff, 0x00, 0xff),
		Fuchsia              = D3DCOLOR_XRGB(0xff, 0x00, 0xff),
		Orchid               = D3DCOLOR_XRGB(0xda, 0x70, 0xd6),
		MediumVioletRed      = D3DCOLOR_XRGB(0xc7, 0x15, 0x85),
		PaleVioletRed        = D3DCOLOR_XRGB(0xdb, 0x70, 0x93),
		DeepPink             = D3DCOLOR_XRGB(0xff, 0x14, 0x93),
		HotPink              = D3DCOLOR_XRGB(0xff, 0x69, 0xb4),
		LightPink            = D3DCOLOR_XRGB(0xff, 0xb6, 0xc1),
		Pink                 = D3DCOLOR_XRGB(0xff, 0xc0, 0xcb),
		AntiqueWhite         = D3DCOLOR_XRGB(0xfa, 0xeb, 0xd7),
		Beige                = D3DCOLOR_XRGB(0xf5, 0xf5, 0xdc),
		Bisque               = D3DCOLOR_XRGB(0xff, 0xe4, 0xc4),
		BlanchedAlmond       = D3DCOLOR_XRGB(0xff, 0xeb, 0xcd),
		Wheat                = D3DCOLOR_XRGB(0xf5, 0xde, 0xb3),
		CornSilk             = D3DCOLOR_XRGB(0xff, 0xf8, 0xdc),
		LemonChiffon         = D3DCOLOR_XRGB(0xff, 0xfa, 0xcd),
		LightGoldenRodYellow = D3DCOLOR_XRGB(0xfa, 0xfa, 0xd2),
		LightYellow          = D3DCOLOR_XRGB(0xff, 0xff, 0xe0),
		SaddleBrown          = D3DCOLOR_XRGB(0x8b, 0x45, 0x13),
		Sienna               = D3DCOLOR_XRGB(0xa0, 0x52, 0x2d),
		Chocolate            = D3DCOLOR_XRGB(0xd2, 0x69, 0x1e),
		Peru                 = D3DCOLOR_XRGB(0xcd, 0x85, 0x3f),
		SandyBrown           = D3DCOLOR_XRGB(0xf4, 0xa4, 0x60),
		BurlyWood            = D3DCOLOR_XRGB(0xde, 0xb8, 0x87),
		Tan                  = D3DCOLOR_XRGB(0xd2, 0xb4, 0x8c),
		RosyBrown            = D3DCOLOR_XRGB(0xbc, 0x8f, 0x8f),
		Moccasin             = D3DCOLOR_XRGB(0xff, 0xe4, 0xb5),
		NavajoWhite          = D3DCOLOR_XRGB(0xff, 0xde, 0xad),
		PeachPuff            = D3DCOLOR_XRGB(0xff, 0xda, 0xb9),
		MistyRose            = D3DCOLOR_XRGB(0xff, 0xe4, 0xe1),
		LavenderBlush        = D3DCOLOR_XRGB(0xff, 0xf0, 0xf5),
		Linen                = D3DCOLOR_XRGB(0xfa, 0xf0, 0xe6),
		OldLace              = D3DCOLOR_XRGB(0xfd, 0xf5, 0xe6),
		PapayaWhip           = D3DCOLOR_XRGB(0xff, 0xef, 0xd5),
		SeaShell             = D3DCOLOR_XRGB(0xff, 0xf5, 0xee),
		MintCream            = D3DCOLOR_XRGB(0xf5, 0xff, 0xfa),
		SlateGray            = D3DCOLOR_XRGB(0x70, 0x80, 0x90),
		LightSlateGray       = D3DCOLOR_XRGB(0x77, 0x88, 0x99),
		LightSteelBlue       = D3DCOLOR_XRGB(0xb0, 0xc4, 0xde),
		Lavender             = D3DCOLOR_XRGB(0xe6, 0xe6, 0xfa),
		FloralWhite          = D3DCOLOR_XRGB(0xff, 0xfa, 0xf0),
		AliceBlue            = D3DCOLOR_XRGB(0xf0, 0xf8, 0xff),
		GhostWhite           = D3DCOLOR_XRGB(0xf8, 0xf8, 0xff),
		Honeydew             = D3DCOLOR_XRGB(0xf0, 0xff, 0xf0),
		Ivory                = D3DCOLOR_XRGB(0xff, 0xff, 0xf0),
		Azure                = D3DCOLOR_XRGB(0xf0, 0xff, 0xff),
		Snow                 = D3DCOLOR_XRGB(0xff, 0xfa, 0xfa),
		Black                = D3DCOLOR_XRGB(0x00, 0x00, 0x00),
		DimGray              = D3DCOLOR_XRGB(0x69, 0x69, 0x69),
		Gray                 = D3DCOLOR_XRGB(0x80, 0x80, 0x80),
		DarkGray             = D3DCOLOR_XRGB(0xa9, 0xa9, 0xa9),
		Silver               = D3DCOLOR_XRGB(0xc0, 0xc0, 0xc0),
		LightGray            = D3DCOLOR_XRGB(0xd3, 0xd3, 0xd3),
		Gainsboro            = D3DCOLOR_XRGB(0xdc, 0xdc, 0xdc),
		WhiteSmoke           = D3DCOLOR_XRGB(0xf5, 0xf5, 0xf5),
		White                = D3DCOLOR_XRGB(0xff, 0xff, 0xff),
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
		const int left, top, right, bottom, x, y, width, height, center, middle;

		Rect(int _x, int _y, int _w, int _h):
			left(_x), top(_y), right(_x + _w), bottom(_y + _h),
			x(_x), y(_y), width(_w), height(_h),
			center(_x + _w / 2), middle(_y + _h / 2) {}

		operator RECT() const
		{
			return { left, top, right, bottom };
		}

		bool contains( const int _x, const int _y ) const
		{
			return _x >= left && _x < right && _y >= top && _y < bottom;
		}
	};

	Graphics(): wnd{NULL}, d3d{NULL}, dev{NULL}, fnt{}
	{
	}

	~Graphics()
	{
		done();
	}

	bool init( HWND hWnd )
	{
		wnd = hWnd;
		d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (d3d == NULL) { return false; }

		D3DPRESENT_PARAMETERS d3dpp = {};
		d3dpp.Windowed         = TRUE;
		d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow    = wnd;
	    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

		HRESULT hr;
		hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dev);
		if (FAILED(hr)) { done(); return false; }

		dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
		dev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		return true;
	}

	Font *font( const INT h, const UINT w, const BYTE p, const TCHAR *f )
	{
		D3DXFONT_DESC desc = {};
		desc.Height          = h;
		desc.Width           = 0;
		desc.Weight          = w;
		desc.MipLevels       = 0;
		desc.Italic          = FALSE;
		desc.CharSet         = DEFAULT_CHARSET;
		desc.OutputPrecision = OUT_TT_PRECIS;
		desc.Quality         = CLEARTYPE_QUALITY;
		desc.PitchAndFamily  = p;
		_tcscpy(desc.FaceName, f);

		Font *font;
		HRESULT hr = D3DXCreateFontIndirect(dev, &desc, &font);
		if (FAILED(hr))
			return NULL;

		fnt.push_back(font);
		return font;
	}

	void quit()
	{
		DestroyWindow(wnd);
	}

	void begin( const D3DCOLOR c )
	{
		dev->Clear(0, NULL, D3DCLEAR_TARGET, c, 1.0f, 0);
		dev->BeginScene();
	}

	void end()
	{
		dev->EndScene();
		dev->Present(NULL, NULL, NULL, NULL);
	}

	void draw_line( const int x, const int y, const int w, const int h, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		Vertex v[] =
		{
			Vertex(x,         y,         c, a),
			Vertex(x + w - 1, y + h - 1, c, a),
		};

		dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, v, sizeof(Vertex));
	}

	void draw_line( const RECT &r, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		draw_line(r.left, r.right, r.right - r.left, r.bottom - r.top, c, a);
	}

	void draw_rect( const int x, const int y, const int w, const int h, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		Vertex v[] =
		{
			Vertex(x,         y,         c, a),
			Vertex(x + w - 1, y,         c, a),
			Vertex(x + w - 1, y + h - 1, c, a),
			Vertex(x,         y + h - 1, c, a),
			Vertex(x,         y,         c, a),
		};

		dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, v, sizeof(Vertex));
	}

	void draw_rect( const RECT &r, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		draw_rect(r.left, r.top, r.right - r.left, r.bottom - r.top, c, a);
	}

	void draw_rect( const RECT &r, const int m, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		draw_rect(r.left + m, r.top + m, r.right - r.left - m * 2, r.bottom - r.top - m * 2, c, a);
	}

	void fill_rect( const int x, const int y, const int w, const int h, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		Vertex v[] =
		{
			Vertex(x,     y,     c, a),
			Vertex(x + w, y,     c, a),
			Vertex(x + w, y + h, c, a),
			Vertex(x,     y + h, c, a),
		};

		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(Vertex));
	}

	void fill_rect( const RECT &r, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		fill_rect(r.left, r.top, r.right - r.left, r.bottom - r.top, c, a);
	}

	void fill_rect( const RECT &r, const int m, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		fill_rect(r.left + m, r.top + m, r.right - r.left - m * 2, r.bottom - r.top - m * 2, c, a);
	}

	void draw_left( const RECT &r, const int h, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		int y = (r.bottom + r.top) / 2;
		int x = r.left + h / 6;
		int d = h * 4 / 6;

		Vertex v[] =
		{
			Vertex(x,     y,         c, a),
			Vertex(x + d, y - d / 2, c, a),
			Vertex(x + d, y + d / 2, c, a),
		};

		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 1, v, sizeof(Vertex));
	}

	void draw_right( const RECT &r, const int h, const D3DCOLOR c, const DWORD a = 0xFF )
	{
		int y = (r.bottom + r.top) / 2;
		int x = r.right - h / 6;
		int d = h * 4 / 6;

		Vertex v[] =
		{
			Vertex(x - d, y - d / 2, c, a),
			Vertex(x,     y,         c, a),
			Vertex(x - d, y + d / 2, c, a),
		};

		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 1, v, sizeof(Vertex));
	}

	void draw_char( const RECT &r, Font *f, const D3DCOLOR c, DWORD a, const TCHAR t )
	{
		RECT rc = r;
		f->DrawText(NULL, &t, 1, &rc, a | DT_NOCLIP, c);
	}

	void draw_char( const RECT &r, const int m, Font *f, const D3DCOLOR c, DWORD a, const TCHAR t )
	{
		RECT rc = { r.left + m, r.top + m, r.right - m, r.bottom - m };
		draw_char(rc, f, c, a, t);
	}

	void draw_text( const RECT &r, Font *f, const D3DCOLOR c, DWORD a, const TCHAR *t )
	{
		RECT rc = r;
		f->DrawText(NULL, t, -1, &rc, a | DT_NOCLIP, c);
	}

	void draw_text( const RECT &r, const int m, Font *f, const D3DCOLOR c, DWORD a, const TCHAR *t )
	{
		RECT rc = { r.left + m, r.top + m, r.right - m, r.bottom - m };
		draw_text(rc, f, c, a, t);
	}
};
