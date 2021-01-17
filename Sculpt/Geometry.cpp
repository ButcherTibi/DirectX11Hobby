
// Header
#include "Geometry.hpp"

#include <glm\ext\quaternion_transform.hpp>


bool AxisBoundingBox3D::isPositionInside(glm::vec3& pos)
{
	if (min.x <= pos.x && pos.x <= max.x &&
		min.y <= pos.y && pos.y <= max.y &&
		min.z <= pos.z && pos.z <= max.z)
	{
		return true;
	}
	return false;
}

bool AxisBoundingBox3D::isRayIsect(Ray& ray)
{
	/* From Stack exchange
	The 3 slabs method
	https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms */

	float t[6];
	t[0] = (min.x - ray.origin.x) / ray.dir.x;
	t[1] = (max.x - ray.origin.x) / ray.dir.x;
	t[2] = (min.y - ray.origin.y) / ray.dir.y;
	t[3] = (max.y - ray.origin.y) / ray.dir.y;
	t[4] = (min.z - ray.origin.z) / ray.dir.z;
	t[5] = (max.z - ray.origin.z) / ray.dir.z;

	float tmin = fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
	float tmax = fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

	float dist_until_isect;

	// ray is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		dist_until_isect = tmax;
		return false;
	}

	// ray doesn't intersect AABB
	if (tmin > tmax)
	{
		dist_until_isect = tmax;
		return false;
	}

	dist_until_isect = tmin;
	return true;
}

void calcMax(glm::vec3& pos, float& r_max)
{
	if (std::abs(pos.x) > r_max) {
		r_max = pos.x;
	}
	
	if (std::abs(pos.y) > r_max) {
		r_max = pos.y;
	}

	if (std::abs(pos.z) > r_max) {
		r_max = pos.z;
	}
}

float toRad(float degree)
{
	return degree * (M_PI / 180.0f);
}

glm::vec3 toNormal(float nord, float east)
{
	glm::quat rot = glm::rotate(glm::quat(1, 0, 0, 0), toRad(east), glm::vec3(0, 1, 0));
	rot = glm::rotate(rot, toRad(nord), glm::vec3(1, 0, 0));
	rot = glm::normalize(rot);
	
	return glm::vec3(0, 0, -1) * rot;
}
