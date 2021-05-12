#pragma once

namespace nui {
	float toRadians(float degrees);

	struct Box2D {
		std::array<int32_t, 2> pos;
		std::array<uint32_t, 2> size;

		bool isInside(int32_t x, int32_t y)
		{
			int32_t top = pos[1];
			int32_t bot = top + size[1];
			int32_t left = pos[0];
			int32_t right = left + size[0];

			if (top < y && y <= bot &&
				left < x && x <= right)
			{
				return true;
			}

			return false;
		}
	};
}
