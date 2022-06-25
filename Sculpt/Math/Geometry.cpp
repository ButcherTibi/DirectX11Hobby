#include "Geometry.hpp"

#include <corecrt_math_defines.h>


float toRad(float degree)
{
	return (float)(degree * (M_PI / 180.));
}

glm::vec3 toNormal(float nord, float east)
{
	glm::quat rot = glm::rotate(glm::quat(1, 0, 0, 0), toRad(east), glm::vec3(0, 1, 0));
	rot = glm::rotate(rot, toRad(nord), glm::vec3(1, 0, 0));
	rot = glm::normalize(rot);
	
	return glm::vec3(0, 0, -1) * rot;
}
