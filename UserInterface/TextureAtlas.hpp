#pragma once

// Standard
#include <list>


namespace nui {

	template<typename T>
	struct BoundingBox2D {
		T x0, y0;  // Top Left Corner
		T x1, y1;  // Bot Right Corner

	public:
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

		T getSize(uint32_t axis) {
			if (axis) {
				return getHeight();
			}

			return getWidth();
		}

		bool isInside(T x, T y) {
			return (x0 <= x && x < x1) &&
				(y0 <= y && y < y1);
		}

	public:
		void set(uint32_t width, uint32_t height)
		{
			x0 = 0;
			x1 = width;
			y0 = 0;
			y1 = height;
		}
	};


	struct AtlasRegion {
		BoundingBox2D<uint32_t> bb_pix;
		BoundingBox2D<float> bb_uv;
	};

	class TextureAtlas {
	public:
		std::vector<uint8_t> colors;  // TODO: add support for multi channel

		uint32_t tex_size = 0;

		uint32_t pen_x = 0;
		uint32_t pen_y = 0;
		uint32_t next_pen_y = 0;
		std::list<AtlasRegion> zones;

	private:
		bool addRegion(AtlasRegion& bitmap_zone,
			std::vector<uint8_t>& bitmap, uint32_t bitmap_width,
			AtlasRegion& r_zone);

	public:
		bool addBitmap(std::vector<uint8_t>& bitmap, uint32_t width, uint32_t height, AtlasRegion*& r_zone_used);

		void debugPrint();
	};
}
