

// Header
#include "MathTypes.h"


glm::vec3 pitchPos(glm::vec3 const& pos, float const& angle)
{
	return glm::rotateX(pos, angle);
}

glm::vec3 pitchPos(glm::vec3 const& pos, float const& angle, glm::vec3 const& center)
{
	glm::vec3 p = pos - center;
	p = glm::rotateX(p, angle);
	return p + center;
}

glm::vec3 yawPos(glm::vec3 const& pos, float const& angle)
{
	return glm::rotateY(pos, angle);
}

glm::vec3 yawPos(glm::vec3 const& pos, float const& angle, glm::vec3 const& center)
{
	glm::vec3 p = pos - center;
	p = glm::rotateY(p, angle);
	return p + center;
}

glm::vec3 rotatePos(glm::vec3 pos, float angle, glm::vec3 normal)
{
	return glm::rotate(pos, angle, normal);
}

glm::vec3 rotatePos(glm::vec3 pos, float angle, glm::vec3 normal, glm::vec3 center)
{
	glm::vec3 p = pos - center;
	p = glm::rotate(p, angle, normal);
	return p + center;
}

glm::vec3 rotatePos(glm::vec3 const& pos, glm::quat q)
{
	return q * pos;
}

glm::quat pitchQuat(glm::quat q, float const& angle)
{
	return glm::rotate(q, angle, glm::vec3{ 1.0f, 0.0f, 0.0f });
}

glm::quat yawQuat(glm::quat q, float const& angle)
{
	return glm::rotate(q, angle, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

glm::quat rotateQuat(glm::quat q, float const& angle, glm::vec3 const& axis)
{
	return glm::rotate(q, angle, axis);
}
