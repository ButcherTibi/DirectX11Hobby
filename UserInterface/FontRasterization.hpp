#pragma once

#include "pch.h"

#include "FileIO.hpp"
#include "TextureAtlas.hpp"


namespace nui {


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

		uint32_t line_spacing;

		std::vector<Character> chars;
	};

	struct Font {
		std::string family_name;
		std::string style_name;

		std::vector<FontSize> sizes;
	};

	class CharacterAtlas {
	public:
		TextureAtlas atlas;

		std::vector<Font> fonts;

	public:
		ErrStack addFont(FilePath& path, std::vector<uint32_t>& sizes, Font*& r_font);
	};
}