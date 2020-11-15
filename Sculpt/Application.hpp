#pragma once

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"
#include "Renderer.hpp"


namespace nui {
	class Window;
}

class Application {
public:
	scme::SculptMesh mesh;

	MeshRenderer renderer;
	nui::Window* window;

	float field_of_view;
	float z_near;
	float z_far;
	glm::vec3 camera_pos;
	glm::quat camera_quat_inv;

	float mouse_sensitivity;

public:
	void arcballOrbitCamera(float deg_x, float deg_y, glm::vec3& pivot);
	void pivotCameraAroundY(float deg_x, float deg_y, glm::vec3& pivot);
	void setCameraPosition(float x, float y, float z);
	void setCameraRotation(float x, float y, float z);
};

extern Application application;