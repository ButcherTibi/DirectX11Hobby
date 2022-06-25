#include "./Camera.hpp"

#include <App/Application.hpp>

// #include <glm/common.hpp>


void Camera::setCameraFocalPoint(glm::vec3& new_focal_point)
{
	focal_point = new_focal_point;
}

/* Previous attempts

Attempt 1:
  glm::quat rotation_x = glm::rotate(glm::quat(1, 0, 0, 0), toRad(deg_x), { 1, 0, 0 });
  glm::quat rotation_y = glm::rotate(glm::quat(1, 0, 0, 0), toRad(deg_y), { 0, 1, 0 });
  mesh.rot = glm::normalize(mesh.rot * rotation_x * rotation_y);


Attempt 2:
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), { 1, 0, 0 });
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), { 0, 1, 0 });
  mesh.rot = glm::normalize(mesh.rot);


Attempt 3: Rotation Around X Local, Rotation Around Y Screen
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), { 1, 0, 0 });

  glm::vec3 y_axis = { 0, 1, 0 };
  y_axis = y_axis * mesh.rot;
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), y_axis);

  mesh.rot = glm::normalize(mesh.rot);


Attempt 4: Rotation Around X Screen, Rotation Around Y Local
  glm::vec3 x_axis = { 1, 0, 0 };
  x_axis = x_axis * mesh.rot;
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), x_axis);
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), { 0, 1, 0 });

  mesh.rot = glm::normalize(mesh.rot);


Attempt 5: Rotation Around X Screen, Rotation Around Y Screen
  glm::vec3 x_axis = { 1, 0, 0 };
  x_axis = x_axis * mesh.rot;
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_x), x_axis);
  
  glm::vec3 y_axis = { 0, 1, 0 };
  y_axis = y_axis * mesh.rot;
  mesh.rot = glm::rotate(mesh.rot, toRad(deg_y), y_axis);
  
  mesh.rot = glm::normalize(mesh.rot);

Notes:
  If I rotate a quaternion by position then clusterfuck.
  If the W component is missing then thinning effect.
*/

void Camera::arcballOrbitCamera(float raw_x, float raw_y)
{
	float deg_x = raw_x * orbit_sensitivity * app.delta_time;
	float deg_y = raw_y * orbit_sensitivity * app.delta_time;

	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } * quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } * quat_inv);

	glm::quat delta_rot = { 1, 0, 0, 0 };
	delta_rot = glm::rotate(delta_rot, toRad(deg_y), camera_right);
	delta_rot = glm::rotate(delta_rot, toRad(deg_x), camera_up);
	delta_rot = glm::normalize(delta_rot);

	pos = (pos - focal_point) * delta_rot;
	pos += focal_point;

	glm::quat reverse_rot = quat_inv;
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_y), camera_right);
	reverse_rot = glm::rotate(reverse_rot, toRad(deg_x), camera_up);

	quat_inv = glm::normalize(reverse_rot);

	forward = glm::normalize(glm::vec3{ 0, 0, -1 } * quat_inv);

	//win32::printToOutput(std::format(L"degrees {} {} \n", deg_x, deg_y));
	//win32::printToOutput(std::format(L"camera_pos {} {} {} \n", pos.x, pos.y, pos.z));
	//win32::printToOutput(std::format(L"quat_inv {} {} {} {} \n", quat_inv.x, quat_inv.y, quat_inv.z, quat_inv.w));
}

void Camera::panCamera(float raw_x, float raw_y)
{
	auto& camera_quat_inv = quat_inv;
	auto& camera_focus = focal_point;
	auto& camera_pos = pos;

	glm::vec3 camera_right = glm::normalize(glm::vec3{ 1, 0, 0 } * camera_quat_inv);
	glm::vec3 camera_up = glm::normalize(glm::vec3{ 0, 1, 0 } * camera_quat_inv);

	float dist = glm::distance(camera_focus, camera_pos);
	if (!dist) {
		dist = 1;
	}

	float amount_x = raw_x * pan_sensitivity * app.delta_time;
	float amount_y = raw_y * pan_sensitivity * app.delta_time;

	camera_pos += amount_x * dist * camera_right + amount_y * dist * camera_up;
}

void Camera::dollyCamera(float amount)
{
	float dist = glm::distance(focal_point, pos);
	if (dist == 0) {
		dist = 1;
	}

	pos += (amount * dist) * forward;
}

void Camera::setPosition(float x, float y, float z)
{
	pos = { x, y, z };
}

void Camera::setCameraRotation(float x, float y, float z)
{
	auto& camera_quat_inv = quat_inv;
	auto& camera_forward = forward;

	glm::quat x_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, x, { 1, 0, 0 });
	glm::quat y_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, y, { 0, 1, 0 });
	glm::quat z_rot = glm::rotate(glm::quat{ 1, 0, 0, 0 }, z, { 0, 0, 1 });

	camera_quat_inv = glm::normalize(x_rot * y_rot * z_rot);

	camera_forward = glm::normalize(glm::vec3{ 0, 0, -1 } *camera_quat_inv);
}
