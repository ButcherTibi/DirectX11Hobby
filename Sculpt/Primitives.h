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
void createTriangleMesh(CreateTriangleInfo info, LinkageMesh& tris);

struct CreateQuadInfo {
	glm::vec3 pos = { 0, 0, 0 };
	glm::quat rot = { 1, 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };

	glm::vec4 color = { 1, 1, 1, 1 };
};
void createQuadMesh(CreateQuadInfo info, LinkageMesh& quad);

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
void createCubeMesh(CreateCubeInfo info, LinkageMesh& cube);


struct CreateCoordinateCubeInfo
{
	glm::vec3 pos = { 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };

	float y_dim = 1.0f;
	float x_dim = 1.0f;
	float z_dim = 1.0f;

	bool clockwise = true;
};
void createCoordinateCubeMesh(CreateCoordinateCubeInfo info, LinkageMesh& coordinate_cube);
