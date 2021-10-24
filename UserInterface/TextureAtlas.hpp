#pragma once

// Standard
#include <vector>
#include <list>

// GLM
#include "glm\vec2.hpp"


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


	inline bool TextureAtlas::addRegion(AtlasRegion& bitmap_zone,
		std::vector<uint8_t>& bitmap, uint32_t bitmap_width,
		AtlasRegion& r_zone)
	{
		uint32_t copy_width = bitmap_zone.bb_pix.getWidth();
		uint32_t copy_height = bitmap_zone.bb_pix.getHeight();

		// can fit vertically
		while (tex_size - pen_y >= copy_height) {

			// can fit horizontally in the current row
			if (tex_size - pen_x < copy_width) {
				pen_x = 0;
				pen_y = next_pen_y;
				next_pen_y = 0;
				continue;
			}

			r_zone.bb_pix.x0 = pen_x;
			r_zone.bb_pix.y0 = pen_y;
			r_zone.bb_pix.x1 = r_zone.bb_pix.x0 + copy_width;
			r_zone.bb_pix.y1 = r_zone.bb_pix.y0 + copy_height;

			if (!pen_x) {
				r_zone.bb_uv.x0 = 0.0f;
			}
			else {
				r_zone.bb_uv.x0 = (float)(r_zone.bb_pix.x0) / tex_size;
			}

			if (!pen_y) {
				r_zone.bb_uv.y0 = 0.0f;
			}
			else {
				r_zone.bb_uv.y0 = (float)(r_zone.bb_pix.y0) / tex_size;
			}

			r_zone.bb_uv.x1 = (float)(r_zone.bb_pix.x1) / tex_size;
			r_zone.bb_uv.y1 = (float)(r_zone.bb_pix.y1) / tex_size;

			// Copy Pixels
			{
				uint32_t src_x0 = bitmap_zone.bb_pix.x0;
				uint32_t src_y0 = bitmap_zone.bb_pix.y0;

				for (uint32_t row = 0; row < copy_height; row++) {

					size_t dst_idx = (pen_y + row) * tex_size + pen_x;
					size_t src_idx = (src_y0 + row) * bitmap_width + src_x0;

					for (uint32_t col = 0; col < copy_width; col++) {

						colors[dst_idx + col] = bitmap[src_idx + col];
					}
				}
			}

			// Update pen position for next copy
			pen_x += copy_width;

			uint32_t new_next_pen_y = pen_y + copy_height;
			if (new_next_pen_y > next_pen_y) {
				next_pen_y = new_next_pen_y;
			}

			return true;
		}

		return false;
	}

	inline bool TextureAtlas::addBitmap(std::vector<uint8_t>& bitmap, uint32_t width, uint32_t height, AtlasRegion*& r_zone_used)
	{
		AtlasRegion bitmap_zone;
		bitmap_zone.bb_pix.x0 = 0;
		bitmap_zone.bb_pix.y0 = 0;
		bitmap_zone.bb_pix.x1 = width;
		bitmap_zone.bb_pix.y1 = height;

		if (!colors.size()) {
			this->tex_size = 2048;
			this->colors.resize((size_t)this->tex_size * this->tex_size);
		}

		AtlasRegion& new_zone = zones.emplace_back();
		r_zone_used = &new_zone;

		if (!addRegion(bitmap_zone, bitmap, width, new_zone)) {

			uint32_t old_atlas_size = this->tex_size;
			this->tex_size *= 2;

			while (this->tex_size < width || this->tex_size < height) {
				this->tex_size *= 2;
			}

			if (this->tex_size > 32768) {
				return false;
			}

			this->pen_x = 0;
			this->pen_y = 0;
			this->next_pen_y = 0;

			std::vector<uint8_t> old_atlas((size_t)this->tex_size * this->tex_size);
			colors.swap(old_atlas);  // now old atlas, truly is old atlas

			// copy old zones into new atlas and update them
			for (AtlasRegion& zone : zones) {

				AtlasRegion new_atlas_zone;
				addRegion(zone, old_atlas, old_atlas_size, new_atlas_zone);

				zone = new_atlas_zone;
			}

			// try adding it again
			r_zone_used = &new_zone;
			return addRegion(bitmap_zone, bitmap, width, new_zone);
		}

		return true;
	}

	inline void TextureAtlas::debugPrint()
	{
		for (size_t row = 0; row < tex_size; row++) {
			for (size_t col = 0; col < tex_size; col++) {

				uint8_t color = colors[row * tex_size + col];

				if (color == 0) {
					putchar(' ');
				}
				else if (color < 255) {
					putchar('.');
				}
				else {
					putchar('0');
				}
			}
			printf("\n");
		}
	}
}