
// Header
#include "TextureAtlas.h"


void TextureAtlas::recreate(uint32_t tex_size, uint32_t channels, uint32_t zone_width, uint32_t zone_height)
{
	this->colors.resize(tex_size * tex_size * channels);

	this->tex_size = tex_size;
	this->zone_width = zone_width;
	this->zone_height = zone_height;

	this->zones_rows = tex_size / zone_height;
	this->zones_cols = tex_size / zone_width;
	this->zones.resize(zones_rows * zones_cols);

	this->zones_used = 0;
	this->mem_size = sizeof(uint8_t) * colors.size();
}

// TODO: add support for multi channels
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

// TODO: enable suupport for adding on 2x2, 3x3, 4x4 etc instead of just 1x1 zones
// ie return a zone with the bb of multiple adjacent zones
bool TextureAtlas::addBitmap(BasicBitmap& bitmap, Zone*& r_zone_used)
{
	for (uint32_t row = 0; row < zones_rows; row++) {
		for (uint32_t col = 0; col < zones_cols; col++) {

			Zone& zone = zones[row * zones_cols + col];

			if (!zone.used) {

				this->zones_used++;

				zone.used = true;

				zone.bb_pix.x0 = col * zone_width;
				zone.bb_pix.y0 = row * zone_height;
				zone.bb_pix.x1 = zone.bb_pix.x0 + bitmap.width;
				zone.bb_pix.y1 = zone.bb_pix.y0 + bitmap.height;

				zone.bb_uv.x0 = (float)(zone.bb_pix.x0) / tex_size;
				zone.bb_uv.y0 = (float)(tex_size - zone.bb_pix.y0) / tex_size;
				zone.bb_uv.x1 = (float)(zone.bb_pix.x1) / tex_size;
				zone.bb_uv.y1 = (float)(tex_size - zone.bb_pix.y1) / tex_size;

				copyPixels(col * zone_width, row * zone_height,
					bitmap.width, bitmap.height,
					bitmap.colors);

				r_zone_used = &zone;

				return true;
			}
		}
	}

	return false;
}

void TextureAtlas::debugPrint()
{
	printf("Pixel Colors: \n");
	for (size_t row = 0; row < tex_size; row++) {
		for (size_t col = 0; col < tex_size; col++) {

			uint8_t color = colors[row * tex_size + col];
			
			if (color == 0) {
				putchar('0');
			}
			else {
				putchar('1');
			}
		}
		printf("\n");
	}

	printf("\n");
}

void TextureAtlas::debugPrintZoneUsage()
{
	for (uint32_t row = 0; row < zones_rows; row++) {
		for (uint32_t col = 0; col < zones_cols; col++) {

			Zone& zone = zones[row * zones_rows + col];

			if (zone.used) {

				printf("1");
			}
			else {
				printf("0");
			}
		}
		printf("\n");
	}
}
