module;

// FreeType
#include <ft2build.h>
#include <freetype\freetype.h>

#include "FilePath.hpp"

export module CharacterAtlas;


import TextureAtlas;


namespace nui {

	export struct Character {
		uint32_t unicode;

		int32_t bitmap_left;
		int32_t bitmap_top;  // the height of the character above the baseline

		int32_t hori_bearing_X;
		int32_t hori_bearing_Y;

		int32_t advance_X;
		int32_t advance_Y;

		AtlasRegion* zone;
	};


	export struct FontSize {
		uint32_t size;

		uint32_t ascender;
		uint32_t descender;
		uint32_t line_spacing;

		std::vector<Character> chars;

		Character* findCharacter(uint32_t unicode);
	};

	Character* FontSize::findCharacter(uint32_t unicode)
	{
		for (Character& chara : chars) {
			if (chara.unicode == unicode) {
				return &chara;
			}
		}

		return nullptr;
	}


	export struct Font {
		std::vector<uint8_t> ttf_file;
		void* face_ft;

		// Properties
		std::string family_name;
		std::string style_name;

		std::vector<FontSize> sizes;
	};


	export class CharacterAtlas {
	public:
		TextureAtlas atlas;

		void* free_type_ft = nullptr;

		std::vector<Font> fonts;

		// Memory cache
		std::vector<uint8_t> _bitmap;

	public:
		ErrStack addFont(std::string path, Font*& r_font);

		// TODO: specify which characters
		ErrStack addSizeToFont(Font* font, uint32_t size, FontSize*& r_font_size);

		// Finds the font and adds the size if it does not exist, else return existing size.
		// Font must exist.
		bool ensureFontWithSize(std::string font_family, std::string font_style, uint32_t size,
			FontSize*& r_font_size);
	};


	ErrStack CharacterAtlas::addFont(std::string path, Font*& r_font)
	{
		ErrStack err_stack;
		FT_Error err;

		FT_Library free_type = (FT_Library)free_type_ft;

		if (free_type == nullptr) {
			err = FT_Init_FreeType(&free_type);
			if (err) {
				return ErrStack(code_location, "failed to initialize FreeType library");
			}
			this->free_type_ft = free_type;
		}

		Font& font = this->fonts.emplace_back();

		checkErrStack(io::readLocalFile(path, font.ttf_file), "failed to read font file");

		FT_Face face;
		err = FT_New_Memory_Face(free_type, font.ttf_file.data(), (uint32_t)font.ttf_file.size(), 0, &face);
		if (err) {
			return ErrStack(code_location, "failed to create font face");
		}

		font.face_ft = face;
		font.family_name = face->family_name;
		font.style_name = face->style_name;

		r_font = &font;

		return err_stack;
	}

	ErrStack CharacterAtlas::addSizeToFont(Font* font, uint32_t size, FontSize*& r_font_size)
	{
		ErrStack err_stack;
		FT_Error err;

		uint32_t first_unicode = '!';
		uint32_t last_unicode = '~';
		uint32_t unicode_count = last_unicode - first_unicode + 1;

		FT_Face face = (FT_Face)font->face_ft;

		err = FT_Set_Pixel_Sizes(face, 0, size);
		if (err) {
			return ErrStack(code_location, "failed to set font face size");
		}

		FontSize& font_size = font->sizes.emplace_back();
		font_size.size = size;

		FT_Size_Metrics& metrics = face->size->metrics;
		font_size.ascender = metrics.ascender / 64;
		font_size.descender = (-metrics.descender) / 64;
		font_size.line_spacing = metrics.height / 64;
		font_size.chars.resize(unicode_count + 1);

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

			_bitmap.resize(glyph->bitmap.width * glyph->bitmap.rows);
			std::memcpy(_bitmap.data(), glyph->bitmap.buffer, _bitmap.size());

			if (!atlas.addBitmap(_bitmap, glyph->bitmap.width, glyph->bitmap.rows, chara.zone)) {
				return ErrStack(code_location, "failed to find space to store character in atlas");
			}
		}

		// White Space
		{
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

		r_font_size = &font_size;

		return err_stack;
	}

	bool CharacterAtlas::ensureFontWithSize(std::string font_family, std::string font_style, uint32_t size,
		FontSize*& r_font_size)
	{
		for (Font& font : fonts) {
			if (font.family_name == font_family && font.style_name == font_style) {

				for (FontSize& font_size : font.sizes) {
					if (font_size.size == size) {
						r_font_size = &font_size;
						return true;
					}
				}

				addSizeToFont(&font, size, r_font_size);
				return true;
			}
		}

		// Don't throw or error on something so trivial
		r_font_size = &fonts[0].sizes[0];
		return false;
	}
}