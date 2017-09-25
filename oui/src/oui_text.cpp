#include <oui_text.h>

#include <stdexcept>
#include <memory>

#include <GL/glew.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace oui
{
	using uchar = unsigned char;

	static constexpr int font_texture_width = 2048;

	Font::Font(const std::string& name, int size) : _size(size)
	{
		_font = CreateFontA(_size, 0, 0, 0, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, FF_DONTCARE, name.c_str());

		glGenTextures(1, &_tex);
		glBindTexture(GL_TEXTURE_2D, _tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, font_texture_width, _size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	Font::~Font()
	{
		glDeleteTextures(1, &_tex);
		DeleteObject(_font);
	}

	int popCodepoint(std::string_view& text)
	{
		if (text.empty())
			return 0;
		int code = 0;
		const uchar x = uchar(text.front()); text.remove_prefix(1);
		uchar remaining = 0;
		switch (x >> 4)
		{
		case 0b1111: 
			code = x & 0x7;
			remaining = 3;
			break;
		case 0b1110:
			code = x & 0xf;
			remaining = 2;
			break;
		case 0b1100:
		case 0b1101:
			code = x & 0x1f;
			remaining = 1;
			break;
		default:
			if (x & 0x80)
				throw std::runtime_error("unepected continuation byte");
			return x;
		}
		for (; remaining > 0; --remaining)
		{
			if (text.empty())
				return 0;
			const uchar y = uchar(text.front()); text.remove_prefix(1);
			if (y >> 6 != 2)
				throw std::runtime_error("continuation byte expected");
			code = (code << 6) | (y & 0x3f);
		}
		return code;
	}

	Point Font::drawText(const Rectangle & area, std::string_view text, float height, const Color& color)
	{
		if (height == 0)
			height = float(_size);
		Point head = area.min;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _tex);

		glColor4fv(&color.r);
		glBegin(GL_QUADS);
		while (int ch = popCodepoint(text))
			head.x = _draw_glyph(ch, head, height);
		glEnd();

		glDisable(GL_TEXTURE_2D);

		return head;
	}
	float Font::_draw_glyph(int ch, Point p, float height)
	{
		auto& info = _init_glyph(ch);

		const auto s0 = info.offset;
		const auto s1 = info.width + s0;

		const float width = info.width * font_texture_width * (height / _size);
		const float x_1 = p.x + width;
		const float y_1 = p.y + height;

		glTexCoord2f(s0, 0); glVertex2f(p.x, y_1);
		glTexCoord2f(s1, 0); glVertex2f(x_1, y_1);
		glTexCoord2f(s1, 1); glVertex2f(x_1, p.y);
		glTexCoord2f(s0, 1); glVertex2f(p.x, p.y);

		return x_1;
	}

	HBITMAP rgb_bitmap(HDC dc, int width, int height, void** data)
	{
		BITMAPINFO bmpi;
		bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpi.bmiHeader.biWidth = width;
		bmpi.bmiHeader.biHeight = height;
		bmpi.bmiHeader.biPlanes = 1;
		bmpi.bmiHeader.biBitCount = 24;
		bmpi.bmiHeader.biCompression = BI_RGB;
		bmpi.bmiHeader.biSizeImage = 0;
		bmpi.bmiHeader.biXPelsPerMeter = 0;
		bmpi.bmiHeader.biYPelsPerMeter = 0;
		bmpi.bmiHeader.biClrUsed = 0;
		bmpi.bmiHeader.biClrImportant = 0;

		HBITMAP bmp = CreateDIBSection(dc, &bmpi, DIB_RGB_COLORS, data, NULL, 0);
		return bmp;
	}

	const Font::GlyphInfo& Font::_init_glyph(int ch)
	{
		auto info = _glyphs.find(ch);
		if (info == _glyphs.end())
		{
			wchar_t str[3] = { wchar_t(ch), 0, 0 };
			if (ch >= 0x10000)
			{
				const int chm = ch - 0x10000;
				str[0] = wchar_t((chm >> 10) + 0xd800);
				str[1] = wchar_t((chm & ((1 << 10) - 1)) + 0xdc00);
			}
			const int yoff = 0;//MulDiv(height,1,16);
			HDC dc = CreateCompatibleDC(NULL);
			SelectObject(dc, _font);
			RECT r;
			SetRect(&r, 1, 1 + yoff, 256, _size);
			DrawTextW(dc, str, 1, &r, DT_CALCRECT);
			int width = (r.right - r.left);

			int width_ceil = (width + 3) & 0xfffffffc;

			char* data = NULL;
			HBITMAP bmp = rgb_bitmap(dc, width_ceil, _size, (void**)&data);
			SelectObject(dc, bmp);
			SetBkColor(dc, RGB(0, 0, 0));
			SetTextColor(dc, RGB(255, 255, 255));
			//SetRect(&r, 0,0,width,height);
			DrawTextW(dc, str, 1, &r, DT_NOCLIP);
			GdiFlush();

			{
				auto img = std::make_unique<unsigned char[]>(width*_size * 4);

				unsigned char* ii = img.get();
				for (int i = 0; i < _size; ++i)
				{
					unsigned char* di = (unsigned char*)(data + i * width_ceil * 3);
					for (int j = 0; j < width; ++j)
					{
						j; // confuse the compler warning away!
						*ii = 255; ++ii;
						*ii = 255; ++ii;
						*ii = 255; ++ii;
						*ii = *di; ++ii;
						di += 3;
					}
				}
				glEnd();
				glTexSubImage2D(GL_TEXTURE_2D, 0, _next_offset, 0, width, _size, GL_RGBA, GL_UNSIGNED_BYTE, img.get());
				glBegin(GL_QUADS);
			}
			DeleteObject(dc);
			DeleteObject(bmp);

			GlyphInfo new_info;
			new_info.offset = float(_next_offset) / font_texture_width;
			new_info.width = float(width) / font_texture_width;

			_next_offset += width + 1;

			info = _glyphs.emplace(ch, new_info).first;
		}
		return info->second;
	}
}
