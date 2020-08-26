#pragma once

// GLM
#include <glm/geometric.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1 \

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>


/* BitVector 3 */

glm::vec3 pitchPos(glm::vec3 const& pos, float const& angle);

glm::vec3 pitchPos(glm::vec3 const& pos, float const& angle, glm::vec3 const& center);

glm::vec3 yawPos(glm::vec3 const& pos, float const& angle);

glm::vec3 yawPos(glm::vec3 const& pos, float const& angle, glm::vec3 const& center);

glm::vec3 rotatePos(glm::vec3 pos, float angle, glm::vec3 normal);

glm::vec3 rotatePos(glm::vec3 pos, float angle, glm::vec3 normal, glm::vec3 center);

// rotates a position by a quaternion
glm::vec3 rotatePos(glm::vec3 const& pos, glm::quat q);


/* Quaternion */ 

glm::quat pitchQuat(glm::quat q, float const& angle);

glm::quat yawQuat(glm::quat q, float const& angle);

// rotates a quaternion q around axis by angle amount
glm::quat rotateQuat(glm::quat q, float const& angle, glm::vec3 const& axis);


/* Matrix */

