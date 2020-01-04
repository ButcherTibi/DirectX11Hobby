
// Standard

// Math
#include "MathTypes.h"

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

Renderer::Renderer()
{

}

ErrorStack Renderer::create(HINSTANCE hinstance, HWND hwnd, uint32_t width, uint32_t height,
	std::vector<char>& vertex_shader_code, std::vector<char>& frag_shader_code)
{
	checkErrStack(instance.create(), "");
	checkErrStack(surface.create(&instance, hinstance, hwnd), "");
	checkErrStack(phys_dev.create(&instance, &surface), "");
	checkErrStack(logical_dev.create(&instance, &phys_dev), "");
	checkErrStack(swapchain.create(&surface, &phys_dev, &logical_dev, width, height), "");

	checkErrStack(cmd_pool.create(&logical_dev, &phys_dev), "");

	// Images
	{
		present_images = swapchain.images;

		std::vector<vks::DesiredImageProps> depth_props(2);
		depth_props[0].format = VK_FORMAT_D32_SFLOAT;
		depth_props[0].type = VK_IMAGE_TYPE_2D;
		depth_props[0].tiling = VK_IMAGE_TILING_OPTIMAL;
		depth_props[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depth_props[0].flags = 0;

		depth_props[1] = depth_props[0];
		depth_props[1].tiling = VK_IMAGE_TILING_LINEAR;

		vks::ImageCreateInfo img_create_info = {};
		img_create_info.desired_props = &depth_props;
		img_create_info.width = swapchain.resolution.width;
		img_create_info.height = swapchain.resolution.height;
		checkErrStack(depth_img.create(&logical_dev, &phys_dev, img_create_info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create depth image");
	}
	
	// Texture
	{
		// Load from Disk
		int32_t width, height, channels;
		uint8_t* tex_pixels = stbi_load("E:/my_work/Vulkan/Sculpt/Sculpt/textures/skunk.PNG",
			&width, &height, &channels, STBI_rgb_alpha);
		size_t tex_size = width * height * 4;

		// Load To Staging
		checkErrStack(staging_buff.createOrGrowStaging(&logical_dev, tex_size), "");
		std::memcpy(staging_buff.mem, tex_pixels, tex_size);
		stbi_image_free(tex_pixels);

		// Create Image
		std::vector<vks::DesiredImageProps> tex_props(1);
		tex_props[0].format = VK_FORMAT_R8G8B8A8_UNORM;
		tex_props[0].type = VK_IMAGE_TYPE_2D;
		tex_props[0].tiling = VK_IMAGE_TILING_OPTIMAL;
		tex_props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		tex_props[0].flags = 0;

		vks::ImageCreateInfo img_create_info = {};
		img_create_info.desired_props = &tex_props;
		img_create_info.width = width;
		img_create_info.height = height;
		checkErrStack(tex_img.create(&logical_dev, &phys_dev, img_create_info, VMA_MEMORY_USAGE_GPU_ONLY),
			"failed to create texture image");

		// Load Staging Buffer into Image
		checkErrStack(vks::changeImageLayout(&logical_dev, &cmd_pool, &tex_img, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL), "");
		checkErrStack(vks::copyBufferToImage(&logical_dev, &cmd_pool, &staging_buff, &tex_img), 
			"failed to copy buffer to image");
		checkErrStack(vks::changeImageLayout(&logical_dev, &cmd_pool, &tex_img,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), "");
	}

	// Image Views
	{
		present_views.resize(present_images.size());
		for (size_t i = 0; i < present_views.size(); i++) {

			checkErrStack(present_views[i].create(&logical_dev, present_images[i],
				swapchain.surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT), "");
		}
		checkErrStack(depth_view.create(&logical_dev, depth_img.img, depth_img.format,
			VK_IMAGE_ASPECT_DEPTH_BIT), "");
		checkErrStack(tex_view.create(&logical_dev, &tex_img, VK_IMAGE_ASPECT_COLOR_BIT), "");
	}
	
	// Samplers
	{
		checkErrStack(sampler.create(&logical_dev), "");
	}

	// Renderpass
	checkErrStack(renderpass.create(&logical_dev, swapchain.surface_format.format, depth_img.format), "");

	// Framebuffers
	checkErrStack(frame_buffs.create(&logical_dev, present_views, &depth_view, &renderpass,
		swapchain.resolution.width, swapchain.resolution.height), "");

	// Buffers
	{
		size_t uniform_buff_size = sizeof(vks::GPUUniform);
		size_t vertex_buff_size = sizeof(vks::GPUVertex) * gpu_verts.size();
		size_t storage_buff_size = sizeof(vks::GPUMeshProperties) * gpu_storage.size();

		std::array<size_t, 3> resource_sizes = {
			uniform_buff_size, vertex_buff_size, storage_buff_size
		};
		size_t staging_size = *std::max_element(resource_sizes.begin(), resource_sizes.end());

		checkErrStack(uniform_buff.create(&logical_dev, uniform_buff_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), "");
		checkErrStack(vertex_buff.create(&logical_dev, vertex_buff_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), "");
		checkErrStack(storage_buff.create(&logical_dev, storage_buff_size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY), "");
		checkErrStack(staging_buff.createOrGrowStaging(&logical_dev, staging_size), "");

		checkErrStack(vertex_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_verts.data(), vertex_buff_size), "");
		checkErrStack(uniform_buff.load(&logical_dev, &cmd_pool, &staging_buff, &gpu_uniform, uniform_buff_size), "");
		checkErrStack(storage_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_storage.data(), storage_buff_size), "");
	}

	// Descriptors
	checkErrStack(descp_layout.create(&logical_dev), "");
	checkErrStack(descp_pool.create(&logical_dev), "");
	checkErrStack(descp_set.create(&logical_dev, &descp_layout, &descp_pool), "");
	descp_set.update(&uniform_buff, &storage_buff, &sampler, &tex_view, tex_img.layout);

	// Pipeline
	checkErrStack(vertex_module.create(&logical_dev, vertex_shader_code, VK_SHADER_STAGE_VERTEX_BIT), "");
	checkErrStack(frag_module.create(&logical_dev, frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT), "");
	checkErrStack(pipe_layout.create(&logical_dev, &descp_layout), "");
	checkErrStack(graphics_pipe.create(&logical_dev, &vertex_module, &frag_module,
		width, height, &pipe_layout, &renderpass), "");

	// Command Buffers
	checkErrStack(render_cmd_buffs.create(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()), "");
	checkErrStack(render_cmd_buffs.update(&renderpass, &frame_buffs,
		swapchain.resolution.width, swapchain.resolution.height, &pipe_layout, &graphics_pipe,
		&descp_set, &vertex_buff, (uint32_t)gpu_verts.size()),
		"");

	checkErrStack(img_acquired.create(&logical_dev),
		"failed to create semaphore for acquiring image from swapchain");
	checkErrStack(rendering_ended_sem.create(&logical_dev),
		"failed to create semaphore for rendering ended");

	// Other
	//int32_t channels;
	//tex_pixels = stbi_load("E:/my_work/Vulkan/Sculpt/Sculpt/textures/skunk.PNG",
	//	&img_width, &img_height, &channels, STBI_rgb_alpha);
	//if (tex_pixels == NULL) {
	//	printf("failed to load image \n");
	//}
	//img_size = img_width * img_height * 4;

	//// stbi_image_free(pixels);

	//

	//checkErrStack(image.create(&logical_dev, img_size, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY), "");
	//this->load_image = true;

	return ErrorStack();
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

ErrorStack Renderer::waitForRendering()
{
	checkVkRes(vkDeviceWaitIdle(logical_dev.logical_device), "");
	return ErrorStack();
}

void Renderer::getRenderResolution(uint32_t& width, uint32_t& height)
{
	width = swapchain.resolution.width;
	height = swapchain.resolution.height;
}

void Renderer::getRequestedRenderResolution(uint32_t& width, uint32_t& height)
{
	width = swapchain.desired_width;
	height = swapchain.desired_height;
}

void Renderer::changeRenderResolution(uint32_t width, uint32_t height)
{

}

void Renderer::generateGPUData()
{
	// calculate sizes
	{
		//uint64_t total_verts_size = 0;
		uint64_t total_idxs_size = 0;

		for (LinkageMesh& mesh : meshes) {

			total_idxs_size += mesh.ttris_count * 3;
		}

		this->gpu_verts.resize(total_idxs_size);
		this->gpu_storage.resize(meshes.size());
	}

	// extract data
	uint64_t i = 0;

	for (uint64_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {

		LinkageMesh& mesh = meshes[mesh_idx];

		for (Poly& p : mesh.polys) {
			for (TesselationTris& t : p.tess_tris) {
				for (Vertex* v : t.vs) {

					vks::GPUVertex& gpu_v = gpu_verts[i];

					// Vertex
					gpu_v.mesh_id = (uint32_t)mesh_idx;
					gpu_v.pos = v->pos;
					gpu_v.vertex_normal = v->normal;
					gpu_v.tess_normal = t.normal;
					gpu_v.poly_normal = p.normal;
					gpu_v.uv = v->uv;
					gpu_v.color = v->color;
					i++;
				}
			}
		}

		// Mesh Data
		vks::GPUMeshProperties& gpu_mesh = gpu_storage[mesh_idx];
		gpu_mesh.pos = mesh.position;
		gpu_mesh.rot.data = mesh.rotation.data;  // same order x,y,z,w
	}

	// Uniform
	{
		gpu_uniform.camera_pos = camera.position;
		gpu_uniform.camera_rot.data = camera.rotation.data;

		// conform aspect ratio to render resolution
		uint32_t render_width;
		uint32_t render_height;
		getRenderResolution(render_width, render_height);

		gpu_uniform.camera_perspective = glm::perspectiveFovRH_ZO(
			glm::radians(camera.fov), (float) render_width, (float) render_height,
			camera.near_plane, camera.far_plane);

		gpu_uniform.camera_forward = camera.forward();
	}
}

ErrorStack Renderer::loadGPUData()
{
	checkErrStack(vertex_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_verts.data(), vertex_buff.buff_alloc_info.size), "");
	checkErrStack(uniform_buff.load(&logical_dev, &cmd_pool, &staging_buff, &gpu_uniform, uniform_buff.buff_alloc_info.size), "");
	checkErrStack(storage_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_storage.data(), storage_buff.buff_alloc_info.size), "");

	return ErrorStack();
}

ErrorStack Renderer::draw()
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

	return ErrorStack();
}
