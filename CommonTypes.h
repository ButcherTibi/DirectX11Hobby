#pragma once

// Standard
#include <vector>

// GLM
#include <glm\vec2.hpp>


struct BasicBitmap {
	std::vector<uint8_t> colors;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

public:
	size_t calcMemSize();

	void debugPrint();
};

template<typename T>
struct BoundingBox {
	T x0, y0;  // Top Left Corner
	T x1, y1;  // Bot Right Corner

	glm::vec2 getTopLeft();
	glm::vec2 getTopRight();
	glm::vec2 getBotRight();
	glm::vec2 getBotLeft();
};
