#pragma once

// GLM
#include <glm/vec3.hpp>

// mine
#include "Meshes.h"


struct CreateTriangleInfo
{
	glm::vec3 pos = {0, 0, 0};
	glm::quat rotation = {1, 0, 0, 0};
	glm::vec3 scale = {1, 1, 1};

	bool clockwise = true;
};
// create a mesh consisting of a single triangle
void createTriangleMesh(CreateTriangleInfo info, LinkageMesh& me);


struct CreateCubeInfo
{
	glm::vec3 pos = { 0, 0, 0 };
	glm::quat rotation = { 1, 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };

	float y_dim = 1.0f;
	float x_dim = 1.0f;
	float z_dim = 1.0f;

	bool clockwise = true;
};
void createCubeMesh(CreateCubeInfo info, LinkageMesh& me);


struct CreateCoordinateCubeInfo
{
	glm::vec3 pos = { 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };

	float y_dim = 1.0f;
	float x_dim = 1.0f;
	float z_dim = 1.0f;

	bool clockwise = true;
};
void createCoordinateCubeMesh(CreateCoordinateCubeInfo info, LinkageMesh& me);
