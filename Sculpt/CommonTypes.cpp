
// Header
#include "CommonTypes.h"


size_t BasicBitmap::calcMemSize()
{
	return width * height * channels;
}

void BasicBitmap::debugPrint()
{
	for (size_t row = 0; row < height; row++) {
		for (size_t col = 0; col < width; col++) {

			uint8_t color = this->colors[row * width + col];

			if (color == 0) {
				putchar('0');
			}
			else if (color == 1) {
				putchar('#');
			}
			else {
				putchar('1');
			}
		}
		printf("\n");
	}
}

template glm::vec2 BoundingBox<float>::getTopLeft();
template glm::vec2 BoundingBox<float>::getTopRight();
template glm::vec2 BoundingBox<float>::getBotRight();
template glm::vec2 BoundingBox<float>::getBotLeft();

template glm::vec2 BoundingBox<uint32_t>::getTopLeft();
template glm::vec2 BoundingBox<uint32_t>::getTopRight();
template glm::vec2 BoundingBox<uint32_t>::getBotRight();
template glm::vec2 BoundingBox<uint32_t>::getBotLeft();

template<typename T>
glm::vec2 BoundingBox<T>::getTopLeft()
{
	return glm::vec2(x0, y0);
}

template<typename T>
glm::vec2 BoundingBox<T>::getTopRight()
{
	return glm::vec2(x1, y0);
}

template<typename T>
glm::vec2 BoundingBox<T>::getBotRight()
{
	return glm::vec2(x1, y1);
}

template<typename T>
glm::vec2 BoundingBox<T>::getBotLeft()
{
	return glm::vec2(x0, y1);
}
