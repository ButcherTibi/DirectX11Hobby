#pragma once
#include "FilePath.hpp"
#include "TextureAtlas.hpp"
#include "GPU_ShaderTypes.hpp"


namespace nui {

	class CharacterAtlas;


	struct Character {
		uint32_t unicode;

		int32_t bitmap_left;
		int32_t bitmap_top;  // the height of the character above the baseline

		int32_t hori_bearing_X;
		int32_t hori_bearing_Y;

		int32_t advance_X;
		int32_t advance_Y;

		AtlasRegion* zone;

		// Rendering
		std::vector<GPU_CharacterVertex> verts;
		uint32_t vertex_start_idx;  // location in the vertex buffer where to find vertices
		uint32_t index_start_idx;  // location in the index buffer where to find indexes
	};

	struct FontSize {
		uint32_t size;

		uint32_t ascender;
		uint32_t descender;
		uint32_t line_spacing;

		std::vector<Character> chars;

		// Rendering
		bool loaded_as_vertices;

		Character* findCharacter(uint32_t unicode);
	};

	class Font {
	public:
		std::vector<uint8_t> ttf_file;
		void* face_ft;

		// Properties
		std::string family_name;
		std::string style_name;

		std::vector<FontSize> sizes;
	};

	class CharacterAtlas {
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
}