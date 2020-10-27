#pragma once

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"


class Application {
public:
	scme::SculptMesh mesh;

	float field_of_view;
	float z_near;
	float z_far;
	
	glm::vec3 camera_pos;
	glm::quat camera_quat;
};

extern Application application;