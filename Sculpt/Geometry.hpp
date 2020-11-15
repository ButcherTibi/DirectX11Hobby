#pragma once

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>


//GLM
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


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


float toRad(float degree);


class Quaternion {
public:
	glm::quat q;

public:
	void rotateAroundX(float deg);
	void rotateAroundY(float deg);
	void rotateAroundZ(float deg);
	void normalize();
};
