/******************************************************************************

   @file    graphics.hpp
   @author  Rajmund Szymanski
   @date    18.11.2020
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
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <vector>
#include <tchar.h>

class Graphics
{
	ID2D1Factory *factory;
	ID2D1HwndRenderTarget *target;
	IDWriteFactory *writer;
	ID2D1SolidColorBrush *brush;
	std::vector<IDWriteTextFormat *> fnt;

	void done()
	{
		for (auto f: fnt) f->Release();

		if (brush   != NULL) { brush->Release();   brush   = NULL; }
		if (writer  != NULL) { writer->Release();  writer  = NULL; }
		if (target  != NULL) { target->Release();  target  = NULL; }
		if (factory != NULL) { factory->Release(); factory = NULL; }
	}

public:

	using Color = D2D1::ColorF::Enum;
	using Font  = IDWriteTextFormat;

	enum Alignment: DWORD
	{
		TopLeft     = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_NEAR,   DWRITE_TEXT_ALIGNMENT_LEADING),
		TopRight    = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_NEAR,   DWRITE_TEXT_ALIGNMENT_TRAILING),
		Top         = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_NEAR,   DWRITE_TEXT_ALIGNMENT_CENTER),
		TopWide     = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_NEAR,   DWRITE_TEXT_ALIGNMENT_JUSTIFIED),
		BottomLeft  = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_FAR,    DWRITE_TEXT_ALIGNMENT_LEADING),
		BottomRight = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_FAR,    DWRITE_TEXT_ALIGNMENT_TRAILING),
		Bottom      = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_FAR,    DWRITE_TEXT_ALIGNMENT_CENTER),
		BottomWide  = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_FAR,    DWRITE_TEXT_ALIGNMENT_JUSTIFIED),
		Left        = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_LEADING),
		Right       = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_TRAILING),
		Center      = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_CENTER),
		Wide        = MAKELONG(DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_JUSTIFIED),
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

		operator D2D1_RECT_F() const
		{
			return { left, top, right, bottom };
		}

		operator D2D1_RECT_U() const
		{
			return { static_cast<UINT32>(std::round(left)),
			         static_cast<UINT32>(std::round(top)),
			         static_cast<UINT32>(std::round(right)),
			         static_cast<UINT32>(std::round(bottom)) };
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

		bool contains( const D2D1_POINT_2F _p ) const
		{
			return contains(_p.x, _p.y);
		}
	};

	Graphics(): factory{NULL}, target{NULL}, writer{NULL}, brush{NULL}, fnt{}
	{
	}

	~Graphics()
	{
		done();
	}

	bool init( HWND hWnd )
	{
		HRESULT hr;
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
		if (FAILED(hr)) return false;

		RECT rc;
		GetClientRect(hWnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &target);
		if (FAILED(hr)) return false;
//		target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
//		target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&writer));
		if (FAILED(hr)) return false;

		hr = target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Enum::Black), &brush);
		return SUCCEEDED(hr);
    }

	Font *font( const FLOAT h, const DWRITE_FONT_WEIGHT w, DWRITE_FONT_STRETCH s, const TCHAR *f )
	{
		Font *format = NULL;
		HRESULT hr = writer->CreateTextFormat(f, NULL, w, DWRITE_FONT_STYLE_NORMAL, s, h, _T(""), &format);
		if (SUCCEEDED(hr))
		{
			format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			fnt.push_back(format);
		}
		return format;
	}

	void quit()
	{
		DestroyWindow(target->GetHwnd());
	}

	void begin( Color c )
	{
		target->BeginDraw();
		target->SetTransform(D2D1::Matrix3x2F::Identity());
		target->Clear(D2D1::ColorF(c));
	}

	void end()
	{
		target->EndDraw();
	}

	void draw_line( const D2D1_POINT_2F &p1, D2D1_POINT_2F &p2, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		brush->SetColor(c);

		target->DrawLine(p1, p2, brush, s);
	}

	void draw_line( const D2D1_RECT_F &r, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		auto p1 = D2D1::Point2F(r.left, r.top);
		auto p2 = D2D1::Point2F(r.right, r.bottom);
		draw_line(p1, p2, c, s);
	}

	void draw_rect( const D2D1_RECT_F &r, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		brush->SetColor(c);

		target->DrawRectangle(&r, brush, s);
	}

	void draw_rounded( const D2D1_ROUNDED_RECT &r, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		brush->SetColor(c);

		target->DrawRoundedRectangle(&r, brush, s);
	}

	void draw_rounded( const D2D1_RECT_F &r, const int rr, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		auto rc = D2D1::RoundedRect(r, rr, rr);
		draw_rounded(rc, c, s);
	}

	void draw_ellipse( const D2D1_ELLIPSE &e, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		brush->SetColor(c);

		target->DrawEllipse(&e, brush, s);
	}

	void draw_ellipse( const D2D1_RECT_F &r, const D2D1::ColorF &c, FLOAT s = 1.0f )
	{
		auto p = D2D1::Point2F((r.right + r.left) / 2, (r.bottom + r.top) / 2);
		auto e = D2D1::Ellipse(p, (r.right - r.left) / 2, (r.bottom - r.top) / 2);
		draw_ellipse(e, c, s);
	}

	void fill_rect( const D2D1_RECT_F &r, const D2D1::ColorF &c )
	{
		brush->SetColor(c);

		target->FillRectangle(&r, brush);
		target->DrawRectangle(&r, brush);
	}

	void fill_rounded( const D2D1_ROUNDED_RECT &r, const D2D1::ColorF &c )
	{
		brush->SetColor(c);

		target->FillRoundedRectangle(&r, brush);
		target->DrawRoundedRectangle(&r, brush);
	}

	void fill_rounded( const D2D1_RECT_F &r, const int rr, const D2D1::ColorF &c )
	{
		auto rc = D2D1::RoundedRect(r, rr, rr);
		fill_rounded(rc, c);
	}

	void fill_ellipse( const D2D1_ELLIPSE &e, const D2D1::ColorF &c )
	{
		brush->SetColor(c);

		target->FillEllipse(&e, brush);
		target->DrawEllipse(&e, brush);
	}

	void fill_ellipse( const D2D1_RECT_F &r, const D2D1::ColorF &c )
	{
		auto p = D2D1::Point2F((r.right + r.left) / 2, (r.bottom + r.top) / 2);
		auto e = D2D1::Ellipse(p, (r.right - r.left) / 2, (r.bottom - r.top) / 2);
		fill_ellipse(e, c);
	}

	void draw_layout( const D2D1_RECT_F &r, Font *f, const D2D1::ColorF &c, Alignment a, const TCHAR *t, size_t s )
	{
		f->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(LOWORD(a)));
		f->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(HIWORD(a)));
		brush->SetColor(c);

		target->DrawText(t, s, f, &r, brush, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP, DWRITE_MEASURING_MODE_NATURAL);
	}

	void draw_char( const D2D1_RECT_F &r, Font *f, const D2D1::ColorF &c, Alignment a, const TCHAR t )
	{
		draw_layout(r, f, c, a, &t, 1);
	}

	void draw_text( const D2D1_RECT_F &r, Font *f, const D2D1::ColorF &c, Alignment a, const TCHAR *t )
	{
		draw_layout(r, f, c, a, t, _tcslen(t));
	}
};
