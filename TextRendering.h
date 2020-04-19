#pragma once

// Standard
#include <vector>
#include <array>
#include <list>

// GLM
#include <glm\vec3.hpp>

// Mine
#include "ErrorStuff.h"
#include "TextureAtlas.h"
#include "UIComponents.h"


struct CharacterInstance {
	glm::vec2 screen_pos;
	float scale;

	Zone* zone;
};

struct RasterizedCharacter {
	uint32_t raster_size;

	// Metrics
	float width;
	float height;
	float baseline_height;
	float advance;  // width + padding left right

	BasicBitmap bitmap;
	Zone* zone;
};

struct Font {
	std::string font_family;
	std::string font_style;

	float ascender;
	float descender;
};

struct CharacterMesh {
	Font* font;

	uint32_t unicode;
	std::array<glm::vec2, 4> verts;
	std::vector<RasterizedCharacter> raster_sizes;

	std::list<CharacterInstance> instances;

public:
	RasterizedCharacter* findBestFitRaster(uint32_t size);
};

struct FontInfo {
	std::string family_name;
	std::string style_name;

	std::vector<uint32_t> sizes_px;
};

class TextStuff {
public:
	TextureAtlas atlas;

	uint32_t max_char_width;
	uint32_t max_char_height;

	std::vector<Font> fonts;
	std::vector<CharacterMesh> char_meshs;

private:
	CharacterMesh* findCharacter(std::string font_family, std::string font_style, uint32_t unicode);

public:
	// add bitmap and set verts
	ErrStack addFont(std::vector<uint8_t>& font_ttf, FontInfo& info);

	ErrStack rebindToAtlas(uint32_t atlas_size);

	ErrStack addInstances(ui::CharSeq& char_seq, uint32_t screen_width, uint32_t screen_height);
};
