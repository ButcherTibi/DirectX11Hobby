module;

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>

#include <array>
#include <cstdint>

export module MathStuff;


namespace nui {

	export float toRadians(float degrees)
	{
		return degrees * ((float)M_PI / 180.f);
	}

	export
	template<typename T>
	T inverseLerp(T value, T min, T max)
	{
		return (value - min) / (max - min);
	}

	export struct Box2D {
		std::array<int32_t, 2> pos;
		std::array<uint32_t, 2> size;

		bool isInside(int32_t x, int32_t y);
	};

	bool Box2D::isInside(int32_t x, int32_t y)
	{
		int32_t top = pos[1];
		int32_t bot = top + size[1];
		int32_t left = pos[0];
		int32_t right = left + size[0];

		if (top <= y && y < bot &&
			left <= x && x < right)
		{
			return true;
		}

		return false;
	}
}