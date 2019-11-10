
// Standard
#include <execution>

// Math
#include "MathTypes.h"

#include "Renderer.h"


glm::vec3 Camera::right()
{
	return glm::normalize(rotatePos({ 1.0f, 0.0f, 0.0f }, glm::inverse(rotation)));
}

glm::vec3 Camera::up()
{
	return glm::normalize(rotatePos({ 0.0f, 1.0f, 0.0f }, glm::inverse(rotation)));
}

glm::vec3 Camera::forward()
{
	return glm::normalize(rotatePos({ 0.0f, 0.0f, -1.0f }, glm::inverse(rotation)));
}

void Renderer::resizeScreenResolution(uint32_t new_width, uint32_t new_height)
{
	// update rendering resolution
	uint32_t r_width;
	uint32_t r_height;
	vk_man.getRequestedRenderResolution(r_width, r_height);

	if (new_width != r_width || new_height != r_height) {
		vk_man.changeRequestedRenderResolution(new_width, new_height);
	}
}

void Renderer::rotateCameraUpLocked(float delta_pitch, float delta_yaw)
{
	float pitch_rad = glm::radians(delta_pitch);
	float yaw_rad = glm::radians(delta_yaw);

	camera.rotation = rotateQuat(camera.rotation, pitch_rad, camera.right());
	camera.rotation = glm::normalize(camera.rotation);
	camera.rotation = rotateQuat(camera.rotation, yaw_rad, { 0, 1, 0 });
	camera.rotation = glm::normalize(camera.rotation);
}

void Renderer::orbitCameraArcball(glm::vec3 center, float delta_pitch, float delta_yaw)
{
	glm::vec3 right_axis = camera.right();
	glm::vec3 up_axis = camera.up();

	float pitch_rad = glm::radians(delta_pitch);
	float yaw_rad = glm::radians(delta_yaw);

	// Orbit Position
	camera.position = rotatePos(camera.position, -pitch_rad, right_axis, center);
	camera.position = rotatePos(camera.position, -yaw_rad, up_axis, center);

	// Make camera point at center
	camera.rotation = rotateQuat(camera.rotation, pitch_rad, right_axis);
	camera.rotation = glm::normalize(camera.rotation);
	camera.rotation = rotateQuat(camera.rotation, yaw_rad, up_axis);
	camera.rotation = glm::normalize(camera.rotation);
}

void Renderer::zoomCamera(glm::vec3 center, float zoom)
{
	float dist = glm::distance(center, camera.position);
	float amount = dist * zoom;

	glm::vec3 cam_forward = camera.forward();
	glm::vec3 cam_pos = camera.position + cam_forward * amount;

	// only move camera if center is not behind
	if (glm::dot(cam_pos - center, cam_forward) < 0) {
		camera.position = cam_pos;
	}
	// TODO: make camera stop at exactly dot == 0
}

void Renderer::panCamera(float delta_vertical, float delta_horizontal)
{
	camera.position += camera.up() * delta_vertical;
	camera.position += camera.right() * -delta_horizontal;
}

void Renderer::loadMeshToBuffs()
{
	// Vertex buffers
	{
		gpu_verts.resize(mesh.verts_size);

		std::atomic_uint32_t atomic_i = 0;

		std::for_each(std::execution::par_unseq, mesh.verts.begin(), mesh.verts.end(), [this, &atomic_i](Vertex& v) {

			uint32_t i = atomic_i.fetch_add(1, std::memory_order_relaxed);

			gpu_verts[i].color.r = v.color.r;
			gpu_verts[i].color.g = v.color.g;
			gpu_verts[i].color.b = v.color.b;
			gpu_verts[i].pos = v.pos;

			v.gpu_idx = i;  // for index buffer
		});
	}

	// Index buffer
	{
		gpu_indexs.resize(mesh.ttris_count.load() * 3);

		std::atomic_uint32_t atomic_i = 0;

		std::for_each(std::execution::par_unseq, mesh.polys.begin(), mesh.polys.end(), [this, &atomic_i](Poly& p) {

			uint32_t i = atomic_i.fetch_add((uint32_t)p.tess_tris.size() * 3, std::memory_order_relaxed);

			for (TriangulationTris& tris : p.tess_tris) {

				gpu_indexs[i++] = tris.vs[0]->gpu_idx;
				gpu_indexs[i++] = tris.vs[1]->gpu_idx;
				gpu_indexs[i++] = tris.vs[2]->gpu_idx;
			}
		});
	}

	// Storage buffer
	{
		gpu_storage.mesh_pos = mesh.position;

		gpu_storage.mesh_rot.x = mesh.rotation.x;
		gpu_storage.mesh_rot.y = mesh.rotation.y;
		gpu_storage.mesh_rot.z = mesh.rotation.z;
		gpu_storage.mesh_rot.w = mesh.rotation.w;
	}

	// Uniform buffer
	{
		gpu_uniform.camera_pos = camera.position;

		gpu_uniform.camera_rot.data = camera.rotation.data;

		// conform aspect ratio to render resolution
		uint32_t render_width;
		uint32_t render_height;
		vk_man.getRenderResolution(render_width, render_height);

		gpu_uniform.camera_perspective = glm::perspectiveFovRH_ZO(
			glm::radians(camera.fov), (float) render_width, (float) render_height,
			camera.near_plane, camera.far_plane);
	}

	vk_man.loadVertexData(&gpu_verts);
	vk_man.loadIndexData(&gpu_indexs);
	vk_man.loadStorageData(&gpu_storage, sizeof(GPUStorage));
	vk_man.loadUniformData(&gpu_uniform, sizeof(GPUUniform));
}
