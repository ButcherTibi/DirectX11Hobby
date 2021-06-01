#pragma once

//GLM
#include <glm/vec3.hpp>


class AxisBoundingBox3D {
public:
	glm::vec3 min;
	glm::vec3 max;

public:
	bool isPositionInside(glm::vec3& pos);
	bool isRayIsect(glm::vec3& origin, glm::vec3& direction);
};

float toRad(float degree);

glm::vec3 toNormal(float nord, float east);
