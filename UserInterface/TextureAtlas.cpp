
#include "pch.h"

// Header
#include "TextureAtlas.hpp"


using namespace nui;


void TextureAtlas::create(uint32_t new_tex_size)
{
	this->colors.resize(new_tex_size * new_tex_size);
	this->tex_size = new_tex_size;
}

void TextureAtlas::copyPixels(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height,
	std::vector<uint8_t>& new_pixels)
{
	for (uint32_t row = 0; row < height; row++) {
		for (uint32_t col = 0; col < width; col++) {

			uint32_t idx = (y0 + row) * this->tex_size + x0 + col;
			uint8_t& old_pix = colors[idx];
			uint8_t& new_pix = new_pixels[row * width + col];

			old_pix = new_pix;
		}
	}
}

bool TextureAtlas::addBitmap(std::vector<uint8_t>& bitmap, uint32_t width, uint32_t height, AtlasRegion*& r_zone_used)
{
	while (tex_size - pen_y >= height) {

		if (tex_size - pen_x < width) {
			pen_x = 0;
			pen_y = next_pen_y;
			next_pen_y = 0;
			continue;
		}

		AtlasRegion& zone = zones.emplace_back();
		zone.bb_pix.x0 = pen_x;
		zone.bb_pix.y0 = pen_y;
		zone.bb_pix.x1 = zone.bb_pix.x0 + width;
		zone.bb_pix.y1 = zone.bb_pix.y0 + height;

		if (!pen_x) {
			zone.bb_uv.x0 = 0.0f;
		}
		else {
			zone.bb_uv.x0 = (float)(zone.bb_pix.x0) / tex_size;
		}

		if (!pen_y) {
			zone.bb_uv.y0 = 0.0f;
		}
		else {
			zone.bb_uv.y0 = (float)(zone.bb_pix.y0) / tex_size;
		}
		
		zone.bb_uv.x1 = (float)(zone.bb_pix.x1) / tex_size;
		zone.bb_uv.y1 = (float)(zone.bb_pix.y1) / tex_size;

		copyPixels(pen_x, pen_y,
			width, height,
			bitmap);

		pen_x += width;

		uint32_t new_next_pen_y = pen_y + height;
		if (new_next_pen_y > next_pen_y) {
			next_pen_y = new_next_pen_y;
		}

		r_zone_used = &zone;
		return true;
	}

	return false;
}

void TextureAtlas::debugPrint()
{
	for (size_t row = 0; row < tex_size; row++) {
		for (size_t col = 0; col < tex_size; col++) {

			uint8_t color = colors[row * tex_size + col];
			
			if (color == 0) {
				putchar(' ');
			}
			else if (color < 255) {
				putchar('.');
			}
			else {
				putchar('0');
			}
		}
		printf("\n");
	}
}
