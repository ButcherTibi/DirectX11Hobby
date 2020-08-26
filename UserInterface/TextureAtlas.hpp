#pragma once

#include "pch.h"


namespace nui {

	template<typename T>
	struct BoundingBox2D {
		T x0, y0;  // Top Left Corner
		T x1, y1;  // Bot Right Corner

		glm::vec2 getTopLeft() {
			return glm::vec2(x0, y0);
		}

		glm::vec2 getTopRight() {
			return glm::vec2(x1, y0);
		}

		glm::vec2 getBotRight() {
			return glm::vec2(x1, y1);
		}

		glm::vec2 getBotLeft() {
			return glm::vec2(x0, y1);
		}

		T getWidth() {
			return x1 - x0;
		}

		T getHeight() {
			return y1 - y0;
		}
	};


	struct AtlasRegion {
		BoundingBox2D<uint32_t> bb_pix;
		BoundingBox2D<float> bb_uv;
	};

	class TextureAtlas {
	public:
		std::vector<uint8_t> colors;

		uint32_t tex_size;

		uint32_t pen_x = 0;
		uint32_t pen_y = 0;
		uint32_t next_pen_y = 0;
		std::list<AtlasRegion> zones;

	private:
		void copyPixels(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height,
			std::vector<uint8_t>& new_pixels);

	public:
		void create(uint32_t tex_size);

		bool addBitmap(std::vector<uint8_t>& colors, uint32_t width, uint32_t height, AtlasRegion*& r_zone_used);

		void debugPrint();
	};
}
