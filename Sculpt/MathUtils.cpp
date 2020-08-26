

// standard
#include <cmath>

// Other
#include "MathDefinitions.h"

#include "MathUtils.h"

uint32_t clamp(uint32_t a, uint32_t min, uint32_t max)
{
	if (a < min) {
		return min;
	}
	else if (a > max) {
		return max;
	}
	return a;
}

float lerpToCavity(float alpha)
{
	if (alpha == 0.0f || alpha == 1.0f) {
		return 1.0f;
	}

	if (alpha < 0.5f) {

		return 1.0f - (alpha / 0.5f);
	}
	return -(alpha - 0.5f) / 0.5f;
}

float lerpToHill(float alpha)
{
	if (alpha == 0.0f || alpha == 1.0f) {
		return 0.0f;
	}

	if (alpha < 0.5f) {

		return alpha / 0.5f;
	}
	return 1.0f - (alpha - 0.5f) / 0.5f;
}
