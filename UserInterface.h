#pragma once

// Standard
#include <vector>
#include <forward_list>

// Mine
#include "ErrorStuff.h"


struct GlyphBitmap {
	uint32_t unicode;
	//std::string font;
	uint32_t size;

	std::vector<uint8_t> pixels;  // single channel bitmap
	uint32_t width;
	uint32_t height;

	void debugPrint();
};

ErrStack createFontBitmaps(std::vector<uint8_t>& font_raw, uint32_t size,
	std::vector<GlyphBitmap>& char_bitmaps);


struct Zone {
	// Glyph
	uint32_t unicode;
	uint32_t font_size;

	// Rect
	uint32_t x0, y0;  // top left
	uint32_t x1, y1;  // bottom right
};

class TextureAtlas {
public:
	std::vector<uint8_t> pixel_colors;
	std::vector<bool> pixel_usage;

	uint32_t width;
	uint32_t height;
	uint32_t channels = 1;

	std::forward_list<Zone> zones;

private:
	bool isRectEmpty(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height);
	void copyPixels(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height,
		std::vector<uint8_t>& new_pixels);
public:

	bool addGlyph(uint32_t unicode, uint32_t font_size, GlyphBitmap& char_bitmap);
	bool findGlyph(uint32_t unicode, uint32_t font_size, Zone& r_zone);
};
