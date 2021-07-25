#pragma once

//GLM
#include <glm/vec3.hpp>


template<typename T = float>
class AxisBoundingBox3D {
public:
	glm::vec<3, T, glm::defaultp> min;
	glm::vec<3, T, glm::defaultp> max;

public:
	void zero()
	{
		// TODO: memset zero
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
		return (T)((max.y + min.y) / 2.0);
	}

	T midZ()
	{
		return (T)((max.z + min.z) / 2.0f);
	}

	bool isPositionInside(glm::vec3& pos);

	bool isRayIsect(glm::vec3& origin, glm::vec3& direction);

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
