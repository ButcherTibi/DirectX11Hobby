
#include "pch.h"

// Header
#include "FontRasterization.hpp"


using namespace nui;


ErrStack CharacterAtlas::addFont(FilePath& path, std::vector<uint32_t>& sizes, Font*& r_font)
{
	ErrStack err_stack;
	FT_Library free_type;

	FT_Error err = FT_Init_FreeType(&free_type);
	if (err) {
		return ErrStack(code_location, "failed to initialize FreeType library");
	}

	FT_Face face;

	std::vector<uint8_t> ttf_file;
	checkErrStack(path.read(ttf_file), "failed to read font file");

	err = FT_New_Memory_Face(free_type, ttf_file.data(), (uint32_t)ttf_file.size(), 0, &face);
	if (err) {
		return ErrStack(code_location, "failed to create font face");
	}

	Font& font = this->fonts.emplace_back();
	font.family_name = face->family_name;
	font.style_name = face->style_name;

	uint32_t first_unicode = '!';
	uint32_t last_unicode = '~';
	uint32_t unicode_count = last_unicode - first_unicode + 1;
	std::vector<uint8_t> bitmap;

	for (auto size : sizes) {

		err = FT_Set_Pixel_Sizes(face, 0, size);
		if (err) {
			return ErrStack(code_location, "failed to set font face size");
		}

		FontSize& font_size = font.sizes.emplace_back();
		font_size.size = size;
		font_size.line_spacing = face->size->metrics.height / 64;
		font_size.chars.resize(unicode_count + 1);

		if (!atlas.colors.size()) {
			atlas.create(2048);
		}

		uint32_t i = 0;
		for (uint32_t unicode = first_unicode; unicode <= last_unicode; unicode++) {

			uint32_t glyph_idx = FT_Get_Char_Index(face, unicode);

			err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_RENDER);
			if (err) {
				return ErrStack(code_location, "failed to load and render glyph");
			}

			auto& glyph = face->glyph;

			Character& chara = font_size.chars[i++];
			chara.unicode = unicode;
			chara.bitmap_left = glyph->bitmap_left;
			chara.bitmap_top = glyph->bitmap_top;
			chara.hori_bearing_X = glyph->metrics.horiBearingX / 64;
			chara.hori_bearing_Y = glyph->metrics.horiBearingY / 64;
			chara.advance_X = glyph->advance.x / 64;
			chara.advance_Y = glyph->advance.y / 64;

			bitmap.resize(glyph->bitmap.width * glyph->bitmap.rows);
			std::memcpy(bitmap.data(), glyph->bitmap.buffer, bitmap.size());

			if (!atlas.addBitmap(bitmap, glyph->bitmap.width, glyph->bitmap.rows, chara.zone)) {
				return ErrStack(code_location, "failed to find space to store character in atlas");
			}
		}

		// White Space
		uint32_t space_unicode = 0x0020;
		uint32_t glyph_idx = FT_Get_Char_Index(face, space_unicode);

		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_RENDER);
		if (err) {
			return ErrStack(code_location, "failed to load and render glyph");
		}

		auto& glyph = face->glyph;

		Character& chara = font_size.chars[i];
		chara.unicode = space_unicode;
		chara.bitmap_left = glyph->bitmap_left;
		chara.bitmap_top = glyph->bitmap_top;
		chara.hori_bearing_X = glyph->metrics.horiBearingX / 64;
		chara.hori_bearing_Y = glyph->metrics.horiBearingY / 64;
		chara.advance_X = glyph->advance.x / 64;
		chara.advance_Y = glyph->advance.y / 64;

		chara.zone = nullptr;
	}

	r_font = &font;

	return ErrStack();
}
