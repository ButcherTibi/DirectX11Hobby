#pragma once

#include "FileIO.hpp"
#include "TextureAtlas.hpp"


namespace nui {

	class CharacterAtlas;


	struct Character {
		uint32_t unicode;

		int32_t bitmap_left;
		int32_t bitmap_top;

		int32_t hori_bearing_X;
		int32_t hori_bearing_Y;

		int32_t advance_X;
		int32_t advance_Y;

		AtlasRegion* zone;
		uint32_t vertex_start_idx;  // location in the vertex buffer where to find vertices
		uint32_t index_start_idx;  // location in the index buffer where to find indexes
	};

	struct FontSize {
		uint32_t size;

		uint32_t ascender;
		uint32_t descender;
		uint32_t line_spacing;

		std::vector<Character> chars;
	};

	class Font {
	public:
		CharacterAtlas* atlas;
		std::vector<uint8_t> ttf_file;
		void* face_ft;	

		// props
		std::string family_name;
		std::string style_name;

		// cache
		std::vector<uint8_t> bitmap;

		std::vector<FontSize> sizes;

	public:
		ErrStack addSize(uint32_t size);
	};

	class CharacterAtlas {
	public:
		TextureAtlas atlas;

		void* free_type_ft = nullptr;

		std::vector<Font> fonts;

	public:
		ErrStack addFont(std::string path, Font*& r_font);
	};
}