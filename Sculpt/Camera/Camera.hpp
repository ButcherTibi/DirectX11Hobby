#pragma once


class Camera {
public:
	// 3D position in space where the camera is starring at,
	// used to tweak camera sensitivity to make movement consistent across al mesh sizes
	glm::vec3 focal_point;

	// horizontal field of view used in the perspective matrix
	float field_of_view;

	// defines the clipping planes used in rendering (does not affect depth)
	float z_near;
	float z_far;

	// camera position in global space
	glm::vec3 pos;

	// reverse camera rotation applied in vertex shader
	glm::quat quat_inv;

	// camera's direction of pointing
	glm::vec3 forward;

	// camera movement sensitivity
	float orbit_sensitivity;
	float pan_sensitivity;
	float dolly_sensitivity;

public:
	// sets the camera point of focus to adjust sensitivity
	void setCameraFocalPoint(glm::vec3& new_focus);

	// orbit camera around focus
	void arcballOrbitCamera(float raw_x, float raw_y);

	// moves the camera along the camera's up and right axes
	void panCamera(float raw_x, float raw_y);

	// moves the camera along the camera's forward axis
	void dollyCamera(float amount);

	// sets the camera's global position
	void setPosition(float x, float y, float z);

	// sets the camera's rotation
	// arguments are Euler and get converted to quartenion
	void setCameraRotation(float x, float y, float z);
};
