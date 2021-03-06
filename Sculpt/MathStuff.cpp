module MathStuff;

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>


//bool AxisBoundingBox3D::isPositionInside(glm::vec3& pos)
//{
//	if (min.x <= pos.x && pos.x <= max.x &&
//		min.y <= pos.y && pos.y <= max.y &&
//		min.z <= pos.z && pos.z <= max.z)
//	{
//		return true;
//	}
//	return false;
//}
//
//bool AxisBoundingBox3D::isRayIsect(glm::vec3& origin, glm::vec3& direction)
//{
//	/* From Stack exchange
//	The 3 slabs method
//	https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms */
//
//	float t[6];
//	t[0] = (min.x - origin.x) / direction.x;
//	t[1] = (max.x - origin.x) / direction.x;
//	t[2] = (min.y - origin.y) / direction.y;
//	t[3] = (max.y - origin.y) / direction.y;
//	t[4] = (min.z - origin.z) / direction.z;
//	t[5] = (max.z - origin.z) / direction.z;
//
//	float tmin = fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
//	float tmax = fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));
//
//	// float dist_until_isect;
//
//	// ray is intersecting AABB, but the whole AABB is behind us
//	if (tmax < 0)
//	{
//		// dist_until_isect = tmax;
//		return false;
//	}
//
//	// ray doesn't intersect AABB
//	if (tmin > tmax)
//	{
//		// dist_until_isect = tmax;
//		return false;
//	}
//
//	// dist_until_isect = tmin;
//	return true;
//}

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
