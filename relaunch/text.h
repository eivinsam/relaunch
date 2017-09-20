#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "point.h"

struct HFONT__;
typedef HFONT__* HFONT;

namespace oui
{
	class Font
	{
		HFONT _font;
		unsigned _tex;
		int _size;
		int _next_offset;
		struct GlyphInfo
		{
			float offset;
			float width;
		};
		std::unordered_map<int, GlyphInfo> _glyphs;

		const GlyphInfo& _init_glyph(int ch);

		float _draw_glyph(int ch, Point p, float height);
	public:
		Font(const std::string& name, int size);
		Font(const Font&) = delete;
		~Font();

		Point drawText(std::string_view, const Rectangle&, float height = 0);
	};
}