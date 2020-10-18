#pragma once

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>


//GLM
#include <glm/vec3.hpp>


class Ray {
public:
	glm::vec3 origin;
	glm::vec3 dir;
};


class AxisBoundingBox3D {
public:
	glm::vec3 min;
	glm::vec3 max;

public:
	bool isRayIsect(Ray& ray);
	// bool isSphereIsect();
};


// Math
float toRad(float degree);
