
// Mine
#include "CommonTypes.h"

#pragma once

struct Zone {
	bool used = false;

	// Rect of used pixels
	BoundingBox<uint32_t> bb_pix;
	BoundingBox<float> bb_uv;
};

class TextureAtlas {
public:
	std::vector<uint8_t> colors;

	uint32_t tex_size;
	uint32_t zone_width;
	uint32_t zone_height;

	uint32_t zones_rows;
	uint32_t zones_cols;
	std::vector<Zone> zones;

	uint32_t zones_used;
	size_t mem_size;

private:
	void copyPixels(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height,
		std::vector<uint8_t>& new_pixels);

public:
	void recreate(uint32_t tex_size, uint32_t channels, uint32_t zone_width, uint32_t zone_height);

	bool addBitmap(BasicBitmap& bitmap, Zone*& r_zone_used);

	void debugPrint();
	void debugPrintZoneUsage();
};

