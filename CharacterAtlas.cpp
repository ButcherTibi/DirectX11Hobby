
// Header
#include "UserInterface.h"

bool TextureAtlas::isRectEmpty(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height)
{
	for (uint32_t row = y0; row < y0 + height; row++) {
		for (uint32_t col = x0; col < x0 + width; col++) {

			if (pixel_usage[row * width + col]) {
				return false;
			}
		}
	}
	return true;
}

void TextureAtlas::copyPixels(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height, 
	std::vector<uint8_t>& new_pixels)
{
	for (uint32_t row = 0; row < height; row++) {
		for (uint32_t col = 0; col < width; col++) {
			
			uint32_t idx = (y0 + row) * this->width + x0 + col;
			uint8_t& old_pix = pixel_colors[idx];
			uint8_t& new_pix = new_pixels[row * width + col];

			old_pix = new_pix;
			pixel_usage[idx] = true;
		}
	}
}

bool TextureAtlas::addGlyph(uint32_t unicode, uint32_t font_size, GlyphBitmap& new_bitmap)
{
	for (uint32_t row = 0; row < width; row++) {
		for (uint32_t col = 0; col < height; col++) {

			if (isRectEmpty(col, row, new_bitmap.width, new_bitmap.height)) {

				copyPixels(col, row, new_bitmap.width, new_bitmap.height,
					new_bitmap.pixels);

				Zone zone;
				zone.unicode = unicode;
				zone.font_size = font_size;
				zone.x0 = col;
				zone.x1 = col + new_bitmap.width;
				zone.y0 = row;
				zone.y1 = row + new_bitmap.height;

				zones.push_front(zone);
				return true;
			}
		}
	}

	return false;
}

bool TextureAtlas::findGlyph(uint32_t unicode, uint32_t font_size, Zone& r_zone)
{
	for (Zone& z : zones) {
		if (z.unicode == unicode && z.font_size == font_size) {
			r_zone = z;
			return true;
		}
	}
	return false;
}
