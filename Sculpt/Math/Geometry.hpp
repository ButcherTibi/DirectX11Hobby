#pragma once

#undef min
#undef max

//GLM
#include <glm/vec3.hpp>
#include <glm/common.hpp>


template<typename T = float>
class AxisBoundingBox3D {
public:
	glm::vec<3, T, glm::defaultp> min;
	glm::vec<3, T, glm::defaultp> max;

public:
	void zero()
	{
		min = { 0, 0, 0 };
		max = { 0, 0, 0 };
	}

	T sizeX()
	{
		return max.x - min.x;
	}

	T sizeY()
	{
		return max.y - min.y;
	}

	T sizeZ()
	{
		return max.z - min.z;
	}

	T midX()
	{
		return (T)((max.x + min.x) / 2.0f);
	}

	T midY()
	{
		return (T)((max.y + min.y) / 2.0f);
	}

	T midZ()
	{
		return (T)((max.z + min.z) / 2.0f);
	}

	/// <summary>
	/// Is position inside AABB
	/// </summary>
	/// <param name="pos">position to check if inside AABB</param>
	/// <returns>True if position is inside</returns>
	bool isPositionInside(glm::vec3& pos)
	{
		if (min.x <= pos.x && pos.x <= max.x &&
			min.y <= pos.y && pos.y <= max.y &&
			min.z <= pos.z && pos.z <= max.z)
		{
			return true;
		}
		return false;
	}

	/// <summary>
	/// Does ray intersect with AABB
	/// </summary>
	/// <param name="origin">ray origin</param>
	/// <param name="direction">ray direction</param>
	/// <returns>True if ray intersects sphere</returns>
	bool isRayIsect(glm::vec3& origin, glm::vec3& direction)
	{
		/* From Stack exchange
		 * The 3 slabs method
		 * https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		*/

		float t[6];
		t[0] = (min.x - origin.x) / direction.x;
		t[1] = (max.x - origin.x) / direction.x;
		t[2] = (min.y - origin.y) / direction.y;
		t[3] = (max.y - origin.y) / direction.y;
		t[4] = (min.z - origin.z) / direction.z;
		t[5] = (max.z - origin.z) / direction.z;

		float tmin = fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
		float tmax = fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

		// float dist_until_isect;

		// ray is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			// dist_until_isect = tmax;
			return false;
		}

		// ray doesn't intersect AABB
		if (tmin > tmax)
		{
			// dist_until_isect = tmax;
			return false;
		}

		// dist_until_isect = tmin;
		return true;
	}

	/// <summary>
	/// Does sphere intersect AABB
	/// </summary>
	/// <param name="origin">sphere origin</param>
	/// <param name="radius">sphere radius</param>
	/// <returns></returns>
	bool isSphereIsect(glm::vec3& origin, float radius)
	{
		// get box closest point to sphere center by clamping
		float x = glm::max(min.x, glm::min(origin.x, max.x));
		float y = glm::max(min.y, glm::min(origin.y, max.y));
		float z = glm::max(min.z, glm::min(origin.z, max.z));

		// this is the same as isPointInsideSphere
		float distance = sqrt(
			(x - origin.x) * (x - origin.x) +
			(y - origin.y) * (y - origin.y) +
			(z - origin.z) * (z - origin.z));

		return distance < radius;
	}

	/// <summary>
	/// Splits a AABB into eight equal parts in each axis
	/// </summary>
	void subdivide(
		AxisBoundingBox3D<T>& box_0, AxisBoundingBox3D<T>& box_1,
		AxisBoundingBox3D<T>& box_2, AxisBoundingBox3D<T>& box_3,
		AxisBoundingBox3D<T>& box_4, AxisBoundingBox3D<T>& box_5,
		AxisBoundingBox3D<T>& box_6, AxisBoundingBox3D<T>& box_7,
		glm::vec3& mid)
	{
		mid.x = midX();
		mid.y = midY();
		mid.z = midZ();

		//              +------------+------------+
		//             /|           /|           /|
		//           /      0     /      1     /  |
		//         /            /            /    |
		//        +------------+------------+     |
		//       /|     |     /|     |     /|     |
		//     /        + - / - - - -+ - / -|- - -+
		//   /         /  /    |    /  /    |    /
		//  +------------+------------+     |  /
		//  |     | /    |     | /    |     |/
		//  |     + - - -| - - + - - -|- - -+     ^ Y    ^ Z
		//  |    /       |    /       |    /      |     /
		//  |        2   |        3   |  /        |   /
		//  | /          | /          |/          | /
		//  +------------+------------+           *-------> X

		{
			// Top Forward Left
			box_0.max = { mid.x, max.y, max.z };
			box_0.min = { min.x, mid.y, mid.z };

			// Top Forward Right
			box_1.max = { max.x, max.y, max.z };
			box_1.min = { mid.x, mid.y, mid.z };

			// Top Backward Left
			box_2.max = { mid.x, max.y, mid.z };
			box_2.min = { min.x, mid.y, min.z };

			// Top Backward Right
			box_3.max = { max.x, max.y, mid.z };
			box_3.min = { mid.x, mid.y, min.z };
		}

		{
			// Bot Forward Left
			box_4.max = { mid.x, mid.y, max.z };
			box_4.min = { min.x, min.y, mid.z };

			// Bot Forward Right
			box_5.max = { max.x, mid.y, max.z };
			box_5.min = { mid.x, min.y, mid.z };

			// Bot Backward Left
			box_6.max = { mid.x, mid.y, mid.z };
			box_6.min = { min.x, min.y, min.z };

			// Bot Backward Right
			box_7.max = { max.x, mid.y, mid.z };
			box_7.min = { mid.x, min.y, min.z };
		}
	}
};


float toRad(float degree);

glm::vec3 toNormal(float nord, float east);
