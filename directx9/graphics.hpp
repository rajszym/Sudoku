/******************************************************************************

   @file    graphics.hpp
   @author  Rajmund Szymanski
   @date    02.12.2020
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
#include <d3d9.h>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <tchar.h>

class Graphics
{
	class Vertex
	{
		FLOAT    v[4];
		D3DCOLOR c;

	public:

		static constexpr DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

		Vertex():                                          v{ 0,  0, 0, 1}, c{Color::Black} {}
		Vertex(FLOAT _x, FLOAT _y):                        v{_x, _y, 0, 1}, c{Color::Black} {}
		Vertex(FLOAT _x, FLOAT _y, D3DCOLOR _c):           v{_x, _y, 0, 1}, c{_c}           {}
		Vertex(FLOAT _x, FLOAT _y, D3DCOLOR _c, DWORD _a): v{_x, _y, 0, 1}, c{_c^(~_a<<24)} {}
	};

	class TexVertex
	{
		FLOAT    v[4];
		D3DCOLOR c;
		FLOAT    t[2];

	public:

		static constexpr DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		TexVertex(FLOAT _x, FLOAT _y, D3DCOLOR _c, FLOAT _u, FLOAT _v): v{_x, _y, 0, 1}, c{_c}, t{_u, _v} {}
	};

public:

	class Font
	{
		friend class Graphics;

		HFONT handle;

		Font( HFONT h ): handle{h} {}

	public:

		~Font() { DeleteObject(handle); }
	};

private:

	// text rendered by GDI into the texture (alpha channel = glyph coverage)
	struct Text
	{
		IDirect3DTexture9 *tex;
		LONG pad, width, height; // size of the text bitmap (with padding)
		UINT tw, th;             // size of the texture
	};

	using TextKey = std::pair<Font *, std::basic_string<TCHAR>>;

	static
	D3DCOLOR Alpha( const D3DCOLOR c, const DWORD a )
	{
		return c ^ (~a << 24);
	}

	static
	D3DCOLOR Alpha( const D3DCOLOR c, const FLOAT a )
	{
		return Alpha(c, static_cast<DWORD>(std::round(0xFF * a)));
	}

	HWND                     wnd;
	LPDIRECT3D9              d3d;
	LPDIRECT3DDEVICE9        dev;
	std::vector<Font *>      fnt;
	std::map<TextKey, Text>  txt;

	static UINT pow2( LONG n )
	{
		UINT p = 1;
		while (p < static_cast<UINT>(n)) p <<= 1;
		return p;
	}

	void release_texts()
	{
		for (auto &t: txt) t.second.tex->Release();
		txt.clear();
	}

	Text *text( Font *f, const std::basic_string<TCHAR> &s )
	{
		auto key = std::make_pair(f, s);
		auto it = txt.find(key);
		if (it != txt.end())
			return &it->second;

		if (txt.size() >= 256) // the timer creates a new text every second
			release_texts();

		Text t = {};
		HDC dc = CreateCompatibleDC(NULL);
		if (dc == NULL)
			return nullptr;
		HGDIOBJ old_font = SelectObject(dc, f->handle);

		SIZE size = {};
		TEXTMETRIC tm = {};
		GetTextExtentPoint32(dc, s.c_str(), static_cast<int>(s.size()), &size);
		GetTextMetrics(dc, &tm);
		t.pad    = tm.tmHeight / 8 + 1; // room for the glyph overhangs
		t.width  = size.cx + t.pad * 2;
		t.height = tm.tmHeight + t.pad * 2;
		t.tw     = pow2(t.width);
		t.th     = pow2(t.height);

		BITMAPINFO bi = {};
		bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
		bi.bmiHeader.biWidth       = t.width;
		bi.bmiHeader.biHeight      = -t.height; // top-down
		bi.bmiHeader.biPlanes      = 1;
		bi.bmiHeader.biBitCount    = 32;
		bi.bmiHeader.biCompression = BI_RGB;

		void *bits = nullptr;
		HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
		bool success = false;
		if (bmp != NULL && bits != nullptr)
		{
			HGDIOBJ old_bmp = SelectObject(dc, bmp);
			ZeroMemory(bits, static_cast<size_t>(t.width) * static_cast<size_t>(t.height) * 4);
			SetBkMode(dc, TRANSPARENT);
			SetTextColor(dc, RGB(0xFF, 0xFF, 0xFF));
			TextOut(dc, t.pad, t.pad, s.c_str(), static_cast<int>(s.size()));
			GdiFlush();

			HRESULT hr = dev->CreateTexture(t.tw, t.th, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &t.tex, NULL);
			D3DLOCKED_RECT lr;
			if (SUCCEEDED(hr) && SUCCEEDED(t.tex->LockRect(0, &lr, NULL, 0)))
			{
				const DWORD *src = static_cast<const DWORD *>(bits);
				for (UINT y = 0; y < t.th; ++y)
				{
					DWORD *dst = reinterpret_cast<DWORD *>(static_cast<BYTE *>(lr.pBits) + y * static_cast<UINT>(lr.Pitch));
					for (UINT x = 0; x < t.tw; ++x)
					{
						DWORD a = 0;
						if (x < static_cast<UINT>(t.width) && y < static_cast<UINT>(t.height))
						{
							DWORD p = src[y * static_cast<UINT>(t.width) + x]; // grayscale antialiasing: R == G == B
							DWORD r = (p >> 16) & 0xFF, g = (p >> 8) & 0xFF, b = p & 0xFF;
							a = r > g ? (r > b ? r : b) : (g > b ? g : b);
						}
						dst[x] = (a << 24) | 0x00FFFFFF;
					}
				}
				t.tex->UnlockRect(0);
				success = true;
			}
			else
			if (t.tex != NULL)
			{
				t.tex->Release();
			}

			SelectObject(dc, old_bmp);
			DeleteObject(bmp);
		}

		SelectObject(dc, old_font);
		DeleteDC(dc);

		if (!success)
			return nullptr;

		return &txt.emplace(std::move(key), t).first->second;
	}

public:

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
			return { r.x - static_cast<FLOAT>(_d),
			         r.y - static_cast<FLOAT>(_d),
			         r.width + static_cast<FLOAT>(_d * 2),
			         r.height + static_cast<FLOAT>(_d * 2) };
		}

		static
		Rect inflate( const Rect &r, const auto _dx, const auto _dy )
		{
			return { r.x - static_cast<FLOAT>(_dx),
			         r.y - static_cast<FLOAT>(_dy),
			         r.width + static_cast<FLOAT>(_dx * 2),
			         r.height + static_cast<FLOAT>(_dy * 2) };
		}

		static
		Rect inflate( const Rect &r, const auto _dx, const auto _dy, const auto _dw, const auto _dh )
		{
			return { r.x - static_cast<FLOAT>(_dx),
			         r.y - static_cast<FLOAT>(_dy),
			         r.width + static_cast<FLOAT>(_dx) + static_cast<FLOAT>(_dw),
			         r.height + static_cast<FLOAT>(_dy) + static_cast<FLOAT>(_dh) };
		}

		static
		Rect deflate( const Rect &r, const auto _d )
		{
			return { r.x + static_cast<FLOAT>(_d),
			         r.y + static_cast<FLOAT>(_d),
			         r.width - static_cast<FLOAT>(_d * 2),
			         r.height - static_cast<FLOAT>(_d * 2) };
		}

		static
		Rect deflate( const Rect &r, const auto _dx, const auto _dy )
		{
			return { r.x + static_cast<FLOAT>(_dx),
			         r.y + static_cast<FLOAT>(_dy),
			         r.width - static_cast<FLOAT>(_dx * 2),
			         r.height - static_cast<FLOAT>(_dy * 2) };
		}

		static
		Rect deflate( const Rect &r, const auto _dx, const auto _dy, const auto _dw, const auto _dh )
		{
			return { r.x + static_cast<FLOAT>(_dx),
			         r.y + static_cast<FLOAT>(_dy),
			         r.width - static_cast<FLOAT>(_dx) - static_cast<FLOAT>(_dw),
			         r.height - static_cast<FLOAT>(_dy) - static_cast<FLOAT>(_dh) };
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

	Graphics(): wnd{NULL}, d3d{NULL}, dev{NULL}, fnt{}
	{
	}

	bool init( HWND hWnd )
	{
		wnd = hWnd;
		d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (d3d == NULL) return false;

		D3DPRESENT_PARAMETERS d3dpp = {};
		d3dpp.Windowed         = TRUE;
		d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow    = wnd;
	    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

		HRESULT hr;
		hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dev);
		if (FAILED(hr)) return false;

		dev->SetFVF(Vertex::FVF);
		dev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dev->SetRenderState(D3DRS_LIGHTING, FALSE);

		// color from the vertex; alpha from the vertex (shapes) or from the texture (texts)
		dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
		dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

		return true;
	}

	~Graphics()
	{
		release_texts();
		for (Font *f: fnt) delete f;

		if (dev != NULL) dev->Release();
		if (d3d != NULL) d3d->Release();
	}

	Font *font( const INT h, const UINT w, const BYTE p, const TCHAR *f )
	{
		HFONT handle = CreateFont(h, 0, 0, 0, static_cast<int>(w), FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                          OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, p, f);
		if (handle == NULL)
			return nullptr;

		Font *font = new Font(handle);
		fnt.push_back(font);

		return font;
	}

	void quit()
	{
		DestroyWindow(wnd);
	}

	void begin( const Color &c )
	{
		dev->Clear(0, NULL, D3DCLEAR_TARGET, c, 1.0f, 0);
		dev->BeginScene();
	}

	void end()
	{
		dev->EndScene();
		dev->Present(NULL, NULL, NULL, NULL);
	}

	void draw_line( const Rect &r, const Color &c )
	{
		Vertex v[] =
		{
			Vertex(r.left,  r.top,    c),
			Vertex(r.right, r.bottom, c),
		};

		dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, v, sizeof(Vertex));
	}

	void draw_rect( const Rect &r, const Color &c, const int s = 1 )
	{
		for (int i = (0 - s) / 2; i < (1 + s) / 2; ++i)
		{
			Vertex v[] =
			{
				Vertex(r.left  + (FLOAT)i, r.top    + (FLOAT)i, c),
				Vertex(r.right - (FLOAT)i, r.top    + (FLOAT)i, c),
				Vertex(r.right - (FLOAT)i, r.bottom - (FLOAT)i, c),
				Vertex(r.left  + (FLOAT)i, r.bottom - (FLOAT)i, c),
				Vertex(r.left  + (FLOAT)i, r.top    + (FLOAT)i, c),
			};

			dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, v, sizeof(Vertex));
		}
	}

	void fill_rect( const Rect &r, const Color &c )
	{
		Vertex v[] =
		{
			Vertex(r.left,  r.top,    c),
			Vertex(r.right, r.top,    c),
			Vertex(r.right, r.bottom, c),
			Vertex(r.left,  r.bottom, c),
			Vertex(r.left,  r.top,    c),
		};

		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(Vertex));
		dev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, v, sizeof(Vertex));
	}

	void draw_char( const Rect &r, Font *f, const Color &c, DWORD a, const TCHAR t )
	{
		draw_text(r, f, c, a, std::basic_string<TCHAR>(1, t));
	}

	void draw_text( const Rect &r, Font *f, const Color &c, DWORD a, const TCHAR *t )
	{
		draw_text(r, f, c, a, std::basic_string<TCHAR>(t));
	}

	void draw_text( const Rect &r, Font *f, const Color &c, DWORD a, const std::basic_string<TCHAR> &s )
	{
		if (f == nullptr || s.empty())
			return;

		Text *t = text(f, s);
		if (t == nullptr)
			return;

		RECT rc = r;
		const LONG w = t->width  - t->pad * 2;
		const LONG h = t->height - t->pad * 2;
		LONG x = (a & DT_RIGHT)   ? rc.right - w : (a & DT_CENTER)  ? (rc.left + rc.right - w) / 2 : rc.left;
		LONG y = (a & DT_BOTTOM)  ? rc.bottom - h : (a & DT_VCENTER) ? (rc.top + rc.bottom - h) / 2 : rc.top;

		// -0.5 maps the texels directly to the pixels
		const FLOAT x1 = static_cast<FLOAT>(x - t->pad) - 0.5f;
		const FLOAT y1 = static_cast<FLOAT>(y - t->pad) - 0.5f;
		const FLOAT x2 = x1 + static_cast<FLOAT>(t->width);
		const FLOAT y2 = y1 + static_cast<FLOAT>(t->height);
		const FLOAT u  = static_cast<FLOAT>(t->width)  / static_cast<FLOAT>(t->tw);
		const FLOAT v  = static_cast<FLOAT>(t->height) / static_cast<FLOAT>(t->th);

		TexVertex vtx[] =
		{
			TexVertex(x1, y1, c, 0, 0),
			TexVertex(x2, y1, c, u, 0),
			TexVertex(x2, y2, c, u, v),
			TexVertex(x1, y2, c, 0, v),
		};

		dev->SetFVF(TexVertex::FVF);
		dev->SetTexture(0, t->tex);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vtx, sizeof(TexVertex));
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
		dev->SetTexture(0, NULL);
		dev->SetFVF(Vertex::FVF);
	}
};
