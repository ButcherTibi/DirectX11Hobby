#pragma once

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>


//GLM
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


class AxisBoundingBox3D {
public:
	glm::vec3 min;
	glm::vec3 max;

public:
	bool isPositionInside(glm::vec3& pos);
	bool isRayIsect(glm::vec3& origin, glm::vec3& direction);
	// bool isSphereIsect();
};

float toRad(float degree);

glm::vec3 toNormal(float nord, float east);

float remapAboveTo01(float half_to_one);

float remapBelowTo01(float zero_to_half);
