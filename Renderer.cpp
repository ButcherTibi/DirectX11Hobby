
// Standard

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

void Renderer::loadMeshesToBuffs()
{
	// calculate sizes
	{
		uint64_t total_verts_size = 0;
		uint64_t total_idxs_size = 0;

		for (LinkageMesh* mesh : meshes) {

			total_verts_size += mesh->verts_size;
			total_idxs_size += mesh->ttris_count * 3;
		}

		this->gpu_verts.resize(total_verts_size);
		this->gpu_indexs.resize(total_idxs_size);
		this->gpu_meshes.resize(meshes.size());
	}
	

	uint32_t total_verts_idx = 0;
	uint32_t total_idxs_idx = 0;
	for (uint64_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {

		LinkageMesh* mesh = meshes[mesh_idx];

		// Vertices
		for (Vertex& v : mesh->verts) {

			GPUVertex& gpu_v = gpu_verts[total_verts_idx];
			gpu_v.mesh_id = (uint32_t)mesh_idx;		
			gpu_v.pos = v.pos;
			gpu_v.color = v.color;

			v.gpu_idx = total_verts_idx;  // for index buffer

			total_verts_idx++;
		}

		// Indexes
		for (Poly& p : mesh->polys) {

			uint32_t& i = total_idxs_idx;

			for (TriangulationTris& tris : p.tess_tris) {

				gpu_indexs[i + 0] = tris.vs[0]->gpu_idx;
				gpu_indexs[i + 1] = tris.vs[1]->gpu_idx;
				gpu_indexs[i + 2] = tris.vs[2]->gpu_idx;

				total_idxs_idx += 3;
			}
		}

		// Mesh Data
		GPUMeshProperties& gpu_mesh = gpu_meshes[mesh_idx];
		gpu_mesh.pos = mesh->position;
		gpu_mesh.rot.data = mesh->rotation.data;  // same order x,y,z,w
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
	vk_man.loadStorageData(gpu_meshes.data(), sizeof(GPUMeshProperties) * gpu_meshes.size());
	vk_man.loadUniformData(&gpu_uniform, sizeof(GPUUniform));
}
