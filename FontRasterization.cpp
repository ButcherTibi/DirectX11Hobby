

// FreeType font rasterization
#include "ft2build.h"
#include FT_FREETYPE_H

// Header
#include "UserInterface.h"


void GlyphBitmap::debugPrint()
{
	for (size_t row = 0; row < height; row++) {
		for (size_t col = 0; col < width; col++) {

			uint8_t color = pixels[row * width + col];

			if (color == 0) {
				putchar('0');
			}
			else if (color == 1) {
				putchar('#');
			}
			else {
				putchar('1');
			}
		}
		printf("\n");
	}
}

ErrStack createFontBitmaps(std::vector<uint8_t>& font_raw, uint32_t size,
	std::vector<GlyphBitmap>& char_bitmaps)
{
	FT_Library free_type;



	char_bitmaps.resize(1);

	char_bitmaps[0].pixels.push_back(255);
	char_bitmaps[0].pixels.push_back(0);
	char_bitmaps[0].pixels.push_back(255);

	char_bitmaps[0].pixels.push_back(0);
	char_bitmaps[0].pixels.push_back(0);
	char_bitmaps[0].pixels.push_back(0);

	char_bitmaps[0].pixels.push_back(255);
	char_bitmaps[0].pixels.push_back(0);
	char_bitmaps[0].pixels.push_back(255);

	char_bitmaps[0].width = 3;
	char_bitmaps[0].height = 3;

	return ErrStack();
}
