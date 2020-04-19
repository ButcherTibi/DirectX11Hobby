
// Standard

#include "MathTypes.h"
#include "stb_image.h"
#include "TextRendering.h"

// Header
#include "Renderer.h"


// globals
Renderer renderer;


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

void Camera::rotateCameraUpLocked(float delta_pitch, float delta_yaw)
{
	float pitch_rad = glm::radians(delta_pitch);
	float yaw_rad = glm::radians(delta_yaw);

	rotation = rotateQuat(rotation, pitch_rad, right());
	rotation = glm::normalize(rotation);
	rotation = rotateQuat(rotation, yaw_rad, { 0, 1, 0 });
	rotation = glm::normalize(rotation);
}

void Camera::orbitCameraArcball(glm::vec3 center, float delta_pitch, float delta_yaw)
{
	glm::vec3 right_axis = right();
	glm::vec3 up_axis = up();

	float pitch_rad = glm::radians(delta_pitch);
	float yaw_rad = glm::radians(delta_yaw);

	// Orbit Position
	position = rotatePos(position, -pitch_rad, right_axis, center);
	position = rotatePos(position, -yaw_rad, up_axis, center);

	// Make camera point at center
	rotation = rotateQuat(rotation, pitch_rad, right_axis);
	rotation = glm::normalize(rotation);
	rotation = rotateQuat(rotation, yaw_rad, up_axis);
	rotation = glm::normalize(rotation);
}

void Camera::zoomCamera(glm::vec3 center, float zoom)
{
	float dist = glm::distance(center, position);
	float amount = dist * zoom;

	glm::vec3 cam_forward = forward();
	glm::vec3 cam_pos = position + cam_forward * amount;

	// only move camera if center is not behind
	if (glm::dot(cam_pos - center, cam_forward) < 0) {
		position = cam_pos;
	}
	// TODO: make camera stop at exactly dot == 0
}

void Camera::panCamera(float delta_vertical, float delta_horizontal)
{
	position += up() * delta_vertical;
	position += right() * -delta_horizontal;
}

ErrStack Renderer::create3D_DiffuseTexture(BasicBitmap& mesh_diffuse)
{
	std::vector<vks::DesiredImageProps> props(1);
	props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	vks::ImageCreateInfo info = {};
	info.desired_props = &props;
	info.width = mesh_diffuse.width;
	info.height = mesh_diffuse.height;
	info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;

	checkErrStack(mesh_diffuse_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
		"failed to create texture image");
	checkErrStack(mesh_diffuse_img.setDebugName("mesh diffuse texture"), "");

	// Load Bitmap into Image
	checkErrStack1(staging_buff.recreateStaging(&logical_dev, mesh_diffuse.calcMemSize()));

	checkErrStack(mesh_diffuse_img.load(mesh_diffuse.colors.data(), mesh_diffuse.calcMemSize(),
		&cmd_pool, &staging_buff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		"failed to load pixels into image");

	return ErrStack();
}

ErrStack Renderer::recreate3D_MeshBuffers(std::vector<LinkageMesh>& meshes)
{
	std::vector<GPU_3D_Vertex> gpu_3d_verts;
	std::vector<GPU_3D_Instance> gpu_3d_storage;

	// calculate sizes
	size_t vertex_buff_size;
	size_t storage_buff_size;
	{
		uint32_t total_idxs_size = 0;

		for (LinkageMesh& mesh : meshes) {

			total_idxs_size += mesh.ttris_count * 3;
		}

		gpu_3d_verts.resize(total_idxs_size);
		gpu_3d_storage.resize(meshes.size());

		vertex_buff_size = gpu_3d_verts.size() * sizeof(GPU_3D_Vertex);
		storage_buff_size = gpu_3d_storage.size() * sizeof(GPU_3D_Instance);

		g3d_vertex_count = total_idxs_size;
	}

	size_t staging_size = vertex_buff_size > storage_buff_size ? vertex_buff_size : storage_buff_size;
	checkErrStack1(staging_buff.recreateStaging(&logical_dev, staging_size));

	checkErrStack(g3d_vertex_buff.recreate(&logical_dev, vertex_buff_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY),
		"failed to recreate 3D vertex buffer");

	checkErrStack(g3d_storage_buff.recreate(&logical_dev, storage_buff_size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY),
		"failed to recreate 3D storage buffer");

	// Extract Data
	uint64_t vert_idx = 0;

	for (uint64_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {

		LinkageMesh& mesh = meshes[mesh_idx];

		for (Poly& p : mesh.polys) {
			for (TesselationTris& t : p.tess_tris) {
				for (Vertex* v : t.vs) {

					GPU_3D_Vertex& gpu_v = gpu_3d_verts[vert_idx];

					// Vertex
					gpu_v.mesh_id = (uint32_t)mesh_idx;
					gpu_v.pos = v->pos;
					gpu_v.vertex_normal = v->normal;
					gpu_v.tess_normal = t.normal;
					gpu_v.poly_normal = p.normal;
					gpu_v.uv = v->uv;
					gpu_v.color = v->color;
					vert_idx++;
				}
			}
		}

		// Mesh Data
		GPU_3D_Instance& gpu_mesh = gpu_3d_storage[mesh_idx];
		gpu_mesh.pos = mesh.position;
		gpu_mesh.rot.data = mesh.rotation.data;  // same order x,y,z,w
	}

	// Load extracted data
	checkErrStack(g3d_vertex_buff.load(&cmd_pool, &staging_buff,
		gpu_3d_verts.data(), vertex_buff_size),
		"failed to load 3D vertices");

	checkErrStack(g3d_storage_buff.load(&cmd_pool, &staging_buff,
		gpu_3d_storage.data(), storage_buff_size),
		"failed to load storage data");

	return ErrStack();
}

ErrStack Renderer::recreate3D_UniformBuffer(Camera& camera)
{
	size_t uniform_buff_size = sizeof(GPU_3D_Uniform);

	checkErrStack1(staging_buff.recreateStaging(&logical_dev, uniform_buff_size));

	checkErrStack(g3d_uniform_buff.recreate(&logical_dev, uniform_buff_size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU),
		"failed to recreate 3D uniform buffer");

	// extract data
	GPU_3D_Uniform gpu_uniform;
	gpu_uniform.camera_pos = camera.position;
	gpu_uniform.camera_rot.data = camera.rotation.data;

	// conform aspect ratio to render resolution
	float width = (float)swapchain.resolution.width;
	float height = (float)swapchain.resolution.height;

	gpu_uniform.camera_perspective = glm::perspectiveFovRH_ZO(
		glm::radians(camera.fov), width, height,
		camera.near_plane, camera.far_plane);

	gpu_uniform.camera_forward = camera.forward();

	// load
	checkErrStack(g3d_uniform_buff.load(&cmd_pool, &staging_buff,
		&gpu_uniform, g3d_uniform_buff.buff_alloc_info.size), "");

	return ErrStack();
}

ErrStack Renderer::recreateUI_MeshBuffers(TextStuff& txt_rend)
{
	// calculate sizes
	size_t vertex_buff_size = txt_rend.char_meshs.size() * 6 * sizeof(GPU_UI_Vertex);
	size_t storage_buff_size = 0;
	{
		ui_batches.resize(txt_rend.char_meshs.size());

		for (CharacterMesh& mesh : txt_rend.char_meshs) {
			storage_buff_size += mesh.instances.size() * sizeof(GPU_UI_Instance);
		}
	}

	size_t staging_size = vertex_buff_size > storage_buff_size ? vertex_buff_size : storage_buff_size;
	checkErrStack1(staging_buff.recreateStaging(&logical_dev, staging_size));

	checkErrStack(ui_vertex_buff.recreate(&logical_dev, vertex_buff_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY),
		"failed to recreate UI vertex buffer");

	checkErrStack(ui_storage_buff.recreate(&logical_dev, storage_buff_size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY),
		"failed to recreate UI storage buffer");

	// Transfer Vertices To Vertex Buffer
	uint32_t batch_idx = 0;
	uint32_t vert_count = 0;

	for (CharacterMesh& mesh : txt_rend.char_meshs) {

		UI_DrawBatch& batch = ui_batches[batch_idx++];
		batch.vert_count = 6;
		batch.vert_idx_start = vert_count;

		std::array<GPU_UI_Vertex, 6> gpu_verts;
		gpu_verts[0].local_pos = mesh.verts[0];
		gpu_verts[1].local_pos = mesh.verts[1];
		gpu_verts[2].local_pos = mesh.verts[3];

		gpu_verts[3].local_pos = mesh.verts[1];
		gpu_verts[4].local_pos = mesh.verts[2];
		gpu_verts[5].local_pos = mesh.verts[3];

		for (uint32_t i = 0; i < gpu_verts.size(); i++) {
			gpu_verts[i].local_vert_idx = i;
		}

		ui_vertex_buff.scheduleLoad(vert_count * sizeof(GPU_UI_Vertex), &staging_buff,
			gpu_verts.data(), gpu_verts.size() * sizeof(GPU_UI_Vertex));

		vert_count += batch.vert_count;
	}
	checkErrStack(ui_vertex_buff.flush(&cmd_pool, &staging_buff),
		"failed to flush UI vertex buffer content");

	// Transfer Instances To Storage Buffer
	batch_idx = 0;
	uint32_t inst_count = 0;

	for (CharacterMesh& mesh : txt_rend.char_meshs) {

		UI_DrawBatch& batch = ui_batches[batch_idx++];
		batch.inst_count = (uint32_t)mesh.instances.size();
		batch.inst_idx_start = inst_count;

		for (CharacterInstance& inst : mesh.instances) {

			GPU_UI_Instance gpu_inst;
			gpu_inst.pos = inst.screen_pos;
			gpu_inst.scale = inst.scale;

			gpu_inst.uvs[0] = inst.zone->bb_uv.getTopLeft();
			gpu_inst.uvs[1] = inst.zone->bb_uv.getTopRight();
			gpu_inst.uvs[2] = inst.zone->bb_uv.getBotLeft();

			gpu_inst.uvs[3] = inst.zone->bb_uv.getTopRight();
			gpu_inst.uvs[4] = inst.zone->bb_uv.getBotRight();
			gpu_inst.uvs[5] = inst.zone->bb_uv.getBotLeft();

			ui_storage_buff.scheduleLoad(inst_count * sizeof(GPU_UI_Instance), &staging_buff,
				&gpu_inst, sizeof(GPU_UI_Instance));

			inst_count++;
		}
	}
	checkErrStack(ui_storage_buff.flush(&cmd_pool, &staging_buff),
		"failed to flush UI storage buffer content");

	return ErrStack();
}

Renderer::Renderer()
{

}

struct UpdateVulkanObjects {
	bool instance;
	bool surface;
	bool phys_dev;
	bool logical_dev;
	bool cmd_pool;
	bool staging_buff;

	// Frame
	bool swapchain;
	bool g3d_color_MSAA_img;
	bool g3d_depth_img;
	bool g3d_resolve_img;
	bool ui_color_img;
	bool renderpass;
	bool framebuffs;

	// 3D
	bool g3d_descp_layout;
	bool g3d_mesh_diffuse_tex;
	bool g3d_mesh_diffuse_sampler;
	bool g3d_vertex_buff;
	bool g3d_storage_buff;
	bool g3d_uniform_buff;
	bool g3d_vertex_shader;
	bool g3d_fragment_shader;
	bool g3d_pipe_layout;
	bool g3d_pipe;
	bool g3d_descp_set_write;

	// UI
	bool ui_descp_layout;
	bool ui_char_atlas_tex;
	bool ui_char_atlas_sampler;
	bool ui_vertex_buff;
	bool ui_storage_buff;
	bool ui_vertex_shader;
	bool ui_fragment_shader;
	bool ui_pipe_layout;
	bool ui_pipe;
	bool ui_descp_set_write;

	// Composition
	bool comp_descp_layout;
	bool comp_vertex_shader;
	bool comp_fragment_shader;
	bool comp_pipe_layout;
	bool comp_pipe;
	bool comp_descp_set_write;

	// Command Buffers
	bool cmd_buffs;
	bool cmd_buffs_update;

	// Sync
	bool present_img_acquired_sem;
	bool rendering_ended_sem;
};

ErrStack Renderer::recreate(RenderingContent& content)
{
	if (instance.instance == VK_NULL_HANDLE) {
		checkErrStack1(instance.create());
	}

	if (surface.surface == VK_NULL_HANDLE) {
		checkErrStack1(surface.create(&instance, *content.hinstance, *content.hwnd));
	}

	if (phys_dev.physical_device == VK_NULL_HANDLE) {
		checkErrStack1(phys_dev.create(&instance, &surface));
	}

	if (logical_dev.logical_device == VK_NULL_HANDLE) {
		checkErrStack1(logical_dev.create(&instance, &phys_dev));
	}

	if (cmd_pool.cmd_pool == VK_NULL_HANDLE) {
		checkErrStack1(cmd_pool.create(&logical_dev, &phys_dev));
	}

	if (staging_buff.buff == VK_NULL_HANDLE) {
		checkErrStack1(staging_buff.recreateStaging(&logical_dev, 1));
		checkErrStack1(staging_buff.setDebugName("staging"));
	}

	if (swapchain.swapchain == VK_NULL_HANDLE) {
		checkErrStack1(swapchain.create(&surface, &phys_dev, &logical_dev,
			content.width, content.height));
		checkErrStack1(swapchain.setDebugName("swapchain"));
	}

	std::vector<vks::DesiredImageProps> props;

	// 3D color multisample
	if (g3d_color_MSAA_img.img == VK_NULL_HANDLE) {

		props.resize(1);
		props[0].format = swapchain.surface_format.format;
		props[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		vks::ImageCreateInfo info = {};
		info.desired_props = &props;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;
		info.samples = phys_dev.max_MSAA;
		info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		checkErrStack(g3d_color_MSAA_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create image for 3D color MSAA");

		checkErrStack(g3d_color_MSAA_img.setDebugName("3D color MSAA"), "");
	}

	// 3D Depth
	if (g3d_depth_img.img == VK_NULL_HANDLE) {

		props.resize(1);
		props[0].format = VK_FORMAT_D32_SFLOAT;
		props[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		vks::ImageCreateInfo info = {};
		info.desired_props = &props;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;
		info.samples = phys_dev.max_MSAA;
		info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		checkErrStack(g3d_depth_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create image for 3D depth");

		checkErrStack(g3d_depth_img.setDebugName("3D depth"), "")
	}

	// 3D Color Resolve
	if (g3d_color_resolve_img.img == VK_NULL_HANDLE) {

		props.resize(1);
		props[0].format = swapchain.surface_format.format;
		props[0].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

		vks::ImageCreateInfo info = {};
		info.desired_props = &props;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;
		info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		checkErrStack(g3d_color_resolve_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create image for 3D color resolve");

		checkErrStack(g3d_color_resolve_img.setDebugName("3D color resolve"), "");
	}

	// UI Color
	if (ui_color_img.img == VK_NULL_HANDLE) {

		props.resize(1);
		props[0].format = swapchain.surface_format.format;
		props[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

		vks::ImageCreateInfo info = {};
		info.desired_props = &props;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;
		info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		checkErrStack(ui_color_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create image for UI color");

		checkErrStack(ui_color_img.setDebugName("UI color"), "");
	}


	if (renderpass.renderpass == VK_NULL_HANDLE) {

		checkErrStack(renderpass.create(&logical_dev, &phys_dev, 
			swapchain.surface_format.format, g3d_depth_img.format),
			"failed to create renderpass");
	}

	if (!frame_buffs.frame_buffs.size()) {

		vks::FrameBufferCreateInfo framebuffs_info = {};
		framebuffs_info.g3d_color_MSAA_img = &g3d_color_MSAA_img;
		framebuffs_info.g3d_depth_img = &g3d_depth_img;
		framebuffs_info.g3d_color_resolve_img = &g3d_color_resolve_img;
		framebuffs_info.ui_color_img = &ui_color_img;
		framebuffs_info.swapchain = &swapchain;

		checkErrStack1(frame_buffs.create(&logical_dev, framebuffs_info, &renderpass,
			swapchain.resolution.width, swapchain.resolution.height));
	}

	// 3D Subpass
	{
		if (g3d_descp.descp_pool == VK_NULL_HANDLE) {

			std::vector<VkDescriptorSetLayoutBinding> bindings(3);
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[0].pImmutableSamplers = NULL;

			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			bindings[1].pImmutableSamplers = NULL;

			bindings[2].binding = 2;
			bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[2].descriptorCount = 1;
			bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[2].pImmutableSamplers = NULL;

			checkErrStack(g3d_descp.create(&logical_dev, bindings),
				"failed to create 3D descriptor");
		}

		// 3D Mesh Diffuse Texture
		if (content.mesh_diffuse != nullptr) {

			BasicBitmap* mesh_diffuse = content.mesh_diffuse;

			std::vector<vks::DesiredImageProps> props(1);
			props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			vks::ImageCreateInfo info = {};
			info.desired_props = &props;
			info.width = mesh_diffuse->width;
			info.height = mesh_diffuse->height;
			info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;

			checkErrStack(mesh_diffuse_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create texture image");
			checkErrStack(mesh_diffuse_img.setDebugName("mesh diffuse texture"), "");

			// Load Bitmap into Image
			checkErrStack1(staging_buff.recreateStaging(&logical_dev, mesh_diffuse->calcMemSize()));

			checkErrStack(mesh_diffuse_img.load(mesh_diffuse->colors.data(), mesh_diffuse->calcMemSize(),
				&cmd_pool, &staging_buff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				"failed to load pixels into image");
		}

		if (mesh_diffuse_sampler.sampler == VK_NULL_HANDLE) {

			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.anisotropyEnable = VK_TRUE;
			info.maxAnisotropy = 16;
			info.compareEnable = VK_FALSE;
			info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

			checkErrStack(mesh_diffuse_sampler.create(&logical_dev, info), "");
		}

		if (content.meshes != nullptr) {
			
			checkErrStack(recreate3D_MeshBuffers(*content.meshes),
				"failed to fill buffers from meshes");
		}

		if (content.camera != nullptr) {

			checkErrStack(recreate3D_UniformBuffer(*content.camera),
				"failed to fill uniform buffer from camera");
		}

		// Vertex Shader
		if (content.g3d_vert_shader_code != nullptr) {

			checkErrStack(g3d_vertex_module.recreate(&logical_dev,
				*content.g3d_vert_shader_code, VK_SHADER_STAGE_VERTEX_BIT),
				"failed to create 3D vertex shader module");
		}

		// Fragment Shader 
		if (content.g3d_frag_shader_code != nullptr) {

			checkErrStack(g3d_frag_module.recreate(&logical_dev,
				*content.g3d_frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT),
				"failed to create 3D fragment shader module");
		}

		// Pipeline Layout
		if (g3d_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

			checkErrStack1(g3d_pipe_layout.create(&logical_dev, &g3d_descp));
		}

		// Pipeline
		if (content.width != 0 ||
			content.height != 0 ||
			content.g3d_vert_shader_code != nullptr ||
			content.g3d_frag_shader_code != nullptr)
		{
			g3d_pipe.setToDefault();
			g3d_pipe.configureFor3D();
			g3d_pipe.multisample_state_info.rasterizationSamples = phys_dev.max_MSAA;

			checkErrStack(g3d_pipe.recreate(&logical_dev, &g3d_vertex_module, &g3d_frag_module,
				swapchain.resolution.width, swapchain.resolution.height,
				&g3d_pipe_layout, &renderpass, 0),
				"failed to create 3D pipeline");
		}

		std::vector<vks::DescriptorWrite> descp_writes;

		// Descriptor Update
		if (content.mesh_diffuse != nullptr) {

			vks::DescriptorWrite& write = descp_writes.emplace_back();

			VkDescriptorImageInfo image_info = {};
			image_info.sampler = mesh_diffuse_sampler.sampler;
			image_info.imageView = mesh_diffuse_img.img_view;
			image_info.imageLayout = mesh_diffuse_img.layout;

			write.dstBinding = 2;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.img_info = &image_info;
		}

		if (content.meshes != nullptr) {

			vks::DescriptorWrite& write = descp_writes.emplace_back();

			VkDescriptorBufferInfo storage_buff_info = {};
			storage_buff_info.buffer = g3d_storage_buff.buff;
			storage_buff_info.offset = 0;
			storage_buff_info.range = g3d_storage_buff.buff_alloc_info.size;

			write.dstBinding = 1;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.descriptorCount = 1;
			write.buff_info = &storage_buff_info;
		}

		if (content.camera != nullptr) {

			vks::DescriptorWrite& write = descp_writes.emplace_back();

			VkDescriptorBufferInfo uniform_buff_info = {};
			uniform_buff_info.buffer = g3d_uniform_buff.buff;
			uniform_buff_info.offset = 0;
			uniform_buff_info.range = g3d_uniform_buff.buff_alloc_info.size;

			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.buff_info = &uniform_buff_info;
		}

		if (descp_writes.size()) {
			g3d_descp.update(descp_writes);
		}
	}

	// UI Subpass
	{
		// Descriptor
		if (ui_descp.descp_pool == VK_NULL_HANDLE) {

			std::vector<VkDescriptorSetLayoutBinding> bindings(2);
			bindings[0] = {};
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			bindings[1] = {};
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			checkErrStack(ui_descp.create(&logical_dev, bindings),
				"failed to create UI descriptor");
		}

		// Character Atlas Image
		if (content.char_atlas_changed) {

			TextureAtlas& atlas = content.text_rendering->atlas;

			std::vector<vks::DesiredImageProps> props(1);
			props[0].format = VK_FORMAT_R8_UNORM;
			props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			vks::ImageCreateInfo info = {};
			info.desired_props = &props;
			info.width = atlas.tex_size;
			info.height = atlas.tex_size;
			info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;

			checkErrStack(ui_char_atlas_img.recreate(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create symbol atlas image");
			checkErrStack(ui_char_atlas_img.setDebugName("character atlas"), "");

			checkErrStack(ui_char_atlas_img.load(atlas.colors.data(), atlas.mem_size, 
				& cmd_pool, & staging_buff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				"failed to load pixels into symbol atlas image");
		}

		if (ui_char_atlas_sampler.sampler == VK_NULL_HANDLE) {

			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.anisotropyEnable = VK_FALSE;
			info.compareEnable = VK_FALSE;
			info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

			checkErrStack(ui_char_atlas_sampler.create(&logical_dev, info), "");
		}

		if (content.text_rendering != nullptr) {

			checkErrStack(recreateUI_MeshBuffers(*content.text_rendering),
				"failed to fill UI mesh buffers");
		}

		if (content.ui_vert_shader_code != nullptr) {

			checkErrStack(ui_vertex_module.recreate(&logical_dev,
				*content.ui_vert_shader_code, VK_SHADER_STAGE_VERTEX_BIT),
				"failed to create UI vertex shader");
		}

		if (content.ui_frag_shader_code != nullptr) {

			checkErrStack(ui_frag_module.recreate(&logical_dev,
				*content.ui_frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT),
				"failed to create UI fragment shader");
		}

		if (ui_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

			checkErrStack(ui_pipe_layout.create(&logical_dev, &ui_descp),
				"failed to create UI pipeline layout");
		}

		if (content.width || content.height ||
			content.ui_vert_shader_code != nullptr || content.ui_frag_shader_code)
		{
			ui_pipe.setToDefault();
			ui_pipe.configureForUserInterface();

			checkErrStack(ui_pipe.recreate(&logical_dev, &ui_vertex_module, &ui_frag_module,
				swapchain.resolution.width, swapchain.resolution.height,
				&ui_pipe_layout, &renderpass, 1),
				"failed to create UI pipeline");
		}

		// Desciptor Update
		std::vector<vks::DescriptorWrite> writes;

		if (content.char_atlas_changed) {

			vks::DescriptorWrite& write = writes.emplace_back();

			VkDescriptorImageInfo ui_symbols_descp_img = {};
			ui_symbols_descp_img.sampler = ui_char_atlas_sampler.sampler;
			ui_symbols_descp_img.imageView = ui_char_atlas_img.img_view;
			ui_symbols_descp_img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.img_info = &ui_symbols_descp_img;
		}

		if (content.text_rendering != nullptr) {

			vks::DescriptorWrite& write = writes.emplace_back();

			VkDescriptorBufferInfo ui_instance_descp_buff = {};
			ui_instance_descp_buff.buffer = ui_storage_buff.buff;
			ui_instance_descp_buff.offset = 0;
			ui_instance_descp_buff.range = ui_storage_buff.buff_alloc_info.size;

			write.dstBinding = 1;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.buff_info = &ui_instance_descp_buff;
		}

		if (writes.size()) {
			ui_descp.update(writes);
		}
	}

	// Composition Subpass
	{
		if (compose_descp.descp_pool == VK_NULL_HANDLE) {

			std::vector<VkDescriptorSetLayoutBinding> bindings(2);
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			checkErrStack(compose_descp.create(&logical_dev, bindings), 
				"failed to create Compose descriptor");
		}

		if (content.comp_vert_shader_code != nullptr) {

			checkErrStack(compose_vert_module.recreate(&logical_dev, 
				*content.comp_vert_shader_code, VK_SHADER_STAGE_VERTEX_BIT),
				"failed to create Compose vertex shader");
		}

		if (content.comp_frag_shader_code != nullptr) {

			checkErrStack(compose_frag_module.recreate(&logical_dev,
				*content.comp_frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT),
				"failed to create Compose fragment shader");
		}

		if (compose_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

			checkErrStack1(compose_pipe_layout.create(&logical_dev, &compose_descp));
		}

		if (content.comp_vert_shader_code != nullptr ||
			content.comp_frag_shader_code != nullptr)
		{
			compose_pipe.setToDefault();

			checkErrStack(compose_pipe.recreate(&logical_dev,
				&compose_vert_module, &compose_frag_module,
				swapchain.resolution.width, swapchain.resolution.height,
				&compose_pipe_layout, &renderpass, 2),
				"failed to create compose pipeline");
		}

		// Desciptor Update
		std::vector<vks::DescriptorWrite> writes;

		if (content.width || content.height) {

			// 3D renderpass result
			vks::DescriptorWrite& g3d_write = writes.emplace_back();

			VkDescriptorImageInfo g3d_img = {};
			g3d_img.imageView = g3d_color_resolve_img.img_view;
			g3d_img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			g3d_write.img_info = &g3d_img;
			g3d_write.dstBinding = 0;
			g3d_write.dstArrayElement = 0;
			g3d_write.descriptorCount = 1;
			g3d_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

			// UI renderpass result
			vks::DescriptorWrite& ui_write = writes.emplace_back();

			VkDescriptorImageInfo ui_img = {};
			ui_img.imageView = ui_color_img.img_view;
			ui_img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			ui_write.img_info = &ui_img;
			ui_write.dstBinding = 1;
			ui_write.dstArrayElement = 0;
			ui_write.descriptorCount = 1;
			ui_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}

		if (writes.size()) {
			compose_descp.update(writes);
		}
	}

	// Command Buffers
	checkErrStack1(render_cmd_buffs.recreate(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()));

	// Command Buffer Update
	{
		vks::RenderingCmdBuffsUpdateInfo info = {};
		info.renderpass = &renderpass;
		info.frame_buffs = &frame_buffs;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;

		// 3D
		info.g3d_descp = &g3d_descp;
		info.g3d_pipe_layout = &g3d_pipe_layout;
		info.g3d_pipe = &g3d_pipe;
		info.g3d_vertex_buff = &g3d_vertex_buff;
		info.g3d_vertex_count = g3d_vertex_count;

		// UI
		info.ui_descp = &ui_descp;
		info.ui_pipe_layout = &ui_pipe_layout;
		info.ui_pipe = &ui_pipe;
		info.ui_draw_batches = &ui_batches;
		info.ui_vertex_buff = &ui_vertex_buff;
		info.ui_storage_buff = &ui_storage_buff;

		// Compose
		info.compose_descp = &compose_descp;
		info.compose_pipe_layout = &compose_pipe_layout;
		info.compose_pipe = &compose_pipe;

		checkErrStack(render_cmd_buffs.update(info), "failed to update rendering commands");
	}

	// Syncronization
	checkErrStack(img_acquired.create(&logical_dev),
		"failed to create semaphore for acquiring image from swapchain");
	checkErrStack(rendering_ended_sem.create(&logical_dev),
		"failed to create semaphore for rendering ended");

	return ErrStack();
}

ErrStack Renderer::draw()
{
	uint32_t image_index;

	checkVkRes(vkAcquireNextImageKHR(logical_dev.logical_device, swapchain.swapchain, UINT64_MAX,
		img_acquired.semaphore, VK_NULL_HANDLE, &image_index), 
		"failed to acquire next image in swapchain");

	// Render
	VkSubmitInfo queue_submit_info = {};
	queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	queue_submit_info.waitSemaphoreCount = 1;
	queue_submit_info.pWaitSemaphores = &img_acquired.semaphore;
	queue_submit_info.pWaitDstStageMask = wait_stages;
	queue_submit_info.commandBufferCount = 1;
	queue_submit_info.pCommandBuffers = &render_cmd_buffs.cmd_buff_tasks[image_index].cmd_buff;
	queue_submit_info.signalSemaphoreCount = 1;
	queue_submit_info.pSignalSemaphores = &rendering_ended_sem.semaphore;

	checkVkRes(vkQueueSubmit(logical_dev.queue, 1, &queue_submit_info, NULL),
		"failed to submit draw commands to gpu queue");

	// Present image
	VkSwapchainKHR swapChains[] = { swapchain.swapchain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &rendering_ended_sem.semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;

	checkVkRes(vkQueuePresentKHR(logical_dev.queue, &present_info),
		"failed to present image");

	return ErrStack();
}

ErrStack Renderer::waitForRendering()
{
	checkVkRes(vkDeviceWaitIdle(logical_dev.logical_device), "");
	return ErrStack();
}
