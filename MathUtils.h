#pragma once

#include <glm/vec3.hpp>

uint32_t clamp(uint32_t a, uint32_t min, uint32_t max);

// Linear to (+1 0 -1)
float lerpToCavity(float alpha);

// Linear to (0 1 0)
float lerpToHill(float alpha);

float distBetweenPos(glm::vec3 pos1, glm::vec3 pos2);