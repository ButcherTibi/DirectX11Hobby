
// Standard

#include "MathTypes.h"
#include "stb_image.h"
#include "UserInterface.h"

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

void BasicBitmap::calcMemSize()
{
	this->mem_size = width * height * channels;
}

Renderer::Renderer()
{

}

ErrStack Renderer::create(RendererCreateInfo& render_info)
{
	checkErrStack(instance.create(), "");
	checkErrStack(surface.create(&instance, render_info.hinstance, render_info.hwnd), "");
	checkErrStack(phys_dev.create(&instance, &surface), "");
	checkErrStack(logical_dev.create(&instance, &phys_dev), "");
	checkErrStack(swapchain.create(&surface, &phys_dev, &logical_dev, render_info.width, render_info.height), "");

	checkErrStack(cmd_pool.create(&logical_dev, &phys_dev), "");

	// Images
	{
		// 3D color multisample
		{
			std::vector<vks::DesiredImageProps> g3d_color_MSAA_props(1);
			g3d_color_MSAA_props[0].format = swapchain.surface_format.format;
			g3d_color_MSAA_props[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			vks::ImageCreateInfo info = {};
			info.desired_props = &g3d_color_MSAA_props;
			info.width = swapchain.resolution.width;
			info.height = swapchain.resolution.height;
			info.samples = phys_dev.max_MSAA;
			checkErrStack(g3d_color_MSAA_img.create(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create image for 3D color MSAA");
		}

		// Depth
		{
			std::vector<vks::DesiredImageProps> depth_props(1);
			depth_props[0].format = VK_FORMAT_D32_SFLOAT;
			depth_props[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			vks::ImageCreateInfo img_create_info = {};
			img_create_info.desired_props = &depth_props;
			img_create_info.width = swapchain.resolution.width;
			img_create_info.height = swapchain.resolution.height;
			img_create_info.samples = phys_dev.max_MSAA;
			checkErrStack(g3d_depth_img.create(&logical_dev, &phys_dev, img_create_info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create depth image");
		}
		
		// 3D Color Resolve
		{
			std::vector<vks::DesiredImageProps> msaa_props(1);
			msaa_props[0].format = swapchain.surface_format.format;
			msaa_props[0].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

			vks::ImageCreateInfo msaa_img_info = {};
			msaa_img_info.desired_props = &msaa_props;
			msaa_img_info.width = swapchain.resolution.width;
			msaa_img_info.height = swapchain.resolution.height;
			checkErrStack(g3d_color_resolve_img.create(&logical_dev, &phys_dev, msaa_img_info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create MSAA image");
		}		

		// Swapchain Images
		swapchain_images = swapchain.images;

		// Model texture
		{
			std::vector<vks::DesiredImageProps> props(1);
			props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			vks::ImageCreateInfo info = {};
			info.desired_props = &props;
			info.width = mesh_difuse.width;
			info.height = mesh_difuse.height;

			checkErrStack(tex_img.create(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create texture image");

			checkErrStack(tex_img.load(mesh_difuse.colors.data(), mesh_difuse.mem_size, &cmd_pool, &staging_buff),
				"failed to load pixels into image");

			checkErrStack(vks::changeImageLayout(&logical_dev, &cmd_pool, &tex_img,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), "");
		}

		// UI Atlas
		{
			std::vector<vks::DesiredImageProps> props(1);
			props[0].format = VK_FORMAT_R8_UNORM;
			props[0].usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			vks::ImageCreateInfo info = {};
			info.desired_props = &props;
			info.width = symbol_atlas.width;
			info.height = symbol_atlas.height;

			checkErrStack(ui_symbol_atlas_img.create(&logical_dev, &phys_dev, info, VMA_MEMORY_USAGE_GPU_ONLY),
				"failed to create symbol atlas image");

			checkErrStack(ui_symbol_atlas_img.load(symbol_atlas.colors.data(), symbol_atlas.mem_size, &cmd_pool, &staging_buff),
				"failed to load pixels into symbol atlas image");

			vks::changeImageLayout(&logical_dev, &cmd_pool, &ui_symbol_atlas_img,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	// Image Views
	{
		swapchain_img_views.resize(swapchain_images.size());
		for (size_t i = 0; i < swapchain_img_views.size(); i++) {
			checkErrStack(swapchain_img_views[i].createPresentView(&logical_dev, swapchain_images[i],
				swapchain.surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT), "");
		}

		checkErrStack(g3d_color_MSAA_view.create(&logical_dev, &g3d_color_MSAA_img, VK_IMAGE_ASPECT_COLOR_BIT), "");
		checkErrStack(g3d_depth_view.create(&logical_dev, &g3d_depth_img, VK_IMAGE_ASPECT_DEPTH_BIT), "");		
		checkErrStack(g3d_color_resolve_view.create(&logical_dev, &g3d_color_resolve_img, VK_IMAGE_ASPECT_COLOR_BIT), "");

		// Texture
		checkErrStack(tex_view.create(&logical_dev, &tex_img, VK_IMAGE_ASPECT_COLOR_BIT), "");
		checkErrStack(ui_symbol_atlas_view.create(&logical_dev, &ui_symbol_atlas_img, VK_IMAGE_ASPECT_COLOR_BIT), "");
	}
	
	// Samplers
	{
		// Texture Sampler
		{
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

			checkErrStack(sampler.create(&logical_dev, info), "");
		}
		
		// UI Symbol Sampler
		{
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

			checkErrStack(ui_symbol_sampler.create(&logical_dev, info), "");
		}
	}

	// Renderpass
	{
		checkErrStack(renderpass.create(&logical_dev, &phys_dev, swapchain.surface_format.format, g3d_depth_img.format), 
			"failed to create renderpass");
	}

	// Framebuffers
	{
		vks::FrameBufferCreateInfo framebuffs_info = {};
		framebuffs_info.g3d_color_MSAA_view = &g3d_color_MSAA_view;
		framebuffs_info.g3d_depth_view = &g3d_depth_view;
		framebuffs_info.g3d_color_resolve_view = &g3d_color_resolve_view;
		framebuffs_info.swapchain_views = &swapchain_img_views;

		checkErrStack(frame_buffs.create(&logical_dev, framebuffs_info, &renderpass,
			swapchain.resolution.width, swapchain.resolution.height), "");
	}
	
	// Buffers
	{
		size_t uniform_buff_size = sizeof(vks::GPUUniform);
		size_t vertex_buff_size = sizeof(vks::GPU_3D_Vertex) * gpu_3d_verts.size();
		size_t storage_buff_size = sizeof(vks::GPUMeshProperties) * gpu_storage.size();
		size_t ui_vertex_size = sizeof(vks::GPU_3D_Vertex) * gpu_ui_verts.size();

		// Staging
		{
			std::array<size_t, 4> resource_sizes = {
			uniform_buff_size, vertex_buff_size, storage_buff_size,
			ui_vertex_size
			};
			size_t staging_size = *std::max_element(resource_sizes.begin(), resource_sizes.end());

			checkErrStack(staging_buff.createOrGrowStaging(&logical_dev, staging_size), "");
		}	

		checkErrStack(uniform_buff.create(&logical_dev, uniform_buff_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), "");
		checkErrStack(vertex_buff.create(&logical_dev, vertex_buff_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), "");
		checkErrStack(storage_buff.create(&logical_dev, storage_buff_size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY), "");
		
		// User Interface
		checkErrStack(ui_vertex_buff.create(&logical_dev, ui_vertex_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY), "");	
	}

	// 3D Pipeline
	{
		// Descriptor
		{
			// Descriptor Layout
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

			// Descriptor Pool
			std::vector<VkDescriptorPoolSize> pool_sizes(3);
			pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			pool_sizes[0].descriptorCount = 1;  // swapchain image count ???

			pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			pool_sizes[1].descriptorCount = 1;

			pool_sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			pool_sizes[2].descriptorCount = 1;

			checkErrStack(g3d_descp.create(&logical_dev, bindings, pool_sizes),
				"failed to create descriptor");

			// Descriptor Set
			std::vector<vks::DescriptorWrite> descp_writes(3);

			VkDescriptorBufferInfo uniform_buff_info = {};
			uniform_buff_info.buffer = uniform_buff.buff;
			uniform_buff_info.offset = 0;
			uniform_buff_info.range = uniform_buff.buff_alloc_info.size;

			descp_writes[0].dstBinding = 0;
			descp_writes[0].dstArrayElement = 0;
			descp_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descp_writes[0].descriptorCount = 1;
			descp_writes[0].buff_info = &uniform_buff_info;

			VkDescriptorBufferInfo storage_buff_info = {};
			storage_buff_info.buffer = storage_buff.buff;
			storage_buff_info.offset = 0;
			storage_buff_info.range = storage_buff.buff_alloc_info.size;

			descp_writes[1].dstBinding = 1;
			descp_writes[1].dstArrayElement = 0;
			descp_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descp_writes[1].descriptorCount = 1;
			descp_writes[1].buff_info = &storage_buff_info;

			VkDescriptorImageInfo image_info = {};
			image_info.sampler = sampler.sampler;
			image_info.imageView = tex_view.img_view;
			image_info.imageLayout = tex_img.layout;

			descp_writes[2].dstBinding = 2;
			descp_writes[2].dstArrayElement = 0;
			descp_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descp_writes[2].descriptorCount = 1;
			descp_writes[2].img_info = &image_info;

			g3d_descp.update(descp_writes);
		}

		checkErrStack(g3d_pipe_layout.create(&logical_dev, &g3d_descp), "");
		checkErrStack(g3d_vertex_module.create(&logical_dev, *render_info.g3d_vert_shader_code, VK_SHADER_STAGE_VERTEX_BIT), "");
		checkErrStack(g3d_frag_module.create(&logical_dev, *render_info.g3d_frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT), "");

		g3d_pipe.configureFor3D();
		g3d_pipe.multisample_state_info.rasterizationSamples = phys_dev.max_MSAA;
		checkErrStack(g3d_pipe.create(&logical_dev, &g3d_vertex_module, &g3d_frag_module,
			render_info.width, render_info.height, &g3d_pipe_layout, &renderpass, 0), "");
	}

	// UI Pipeline
	{
		// Descriptors
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(2);
			bindings[0] = {};
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			bindings[1] = {};
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			std::vector<VkDescriptorPoolSize> pool_sizes(2);
			pool_sizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			pool_sizes[0].descriptorCount = 1;

			pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			pool_sizes[1].descriptorCount = 1;

			checkErrStack(ui_descp.create(&logical_dev, bindings, pool_sizes),
				"failed to create descriptor for composition subpass");

			// Descriptor Set Write
			std::vector<vks::DescriptorWrite> write(2);

			VkDescriptorImageInfo g3d_color_descp_img = {};
			g3d_color_descp_img.imageView = g3d_color_resolve_view.img_view;
			g3d_color_descp_img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
			write[0].dstBinding = 0;
			write[0].dstArrayElement = 0;
			write[0].descriptorCount = 1;
			write[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			write[0].img_info = &g3d_color_descp_img;

			VkDescriptorImageInfo ui_symbols_descp_img = {};
			ui_symbols_descp_img.sampler = ui_symbol_sampler.sampler;
			ui_symbols_descp_img.imageView = ui_symbol_atlas_view.img_view;
			ui_symbols_descp_img.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			write[1].dstBinding = 1;
			write[1].dstArrayElement = 0;
			write[1].descriptorCount = 1;
			write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write[1].img_info = &ui_symbols_descp_img;

			ui_descp.update(write);
		}

		checkErrStack(ui_pipe_layout.create(&logical_dev, &ui_descp), "");
		checkErrStack(ui_vertex_module.create(&logical_dev, *render_info.ui_vert_shader_code, VK_SHADER_STAGE_VERTEX_BIT), "");
		checkErrStack(ui_frag_module.create(&logical_dev, *render_info.ui_frag_shader_code, VK_SHADER_STAGE_FRAGMENT_BIT), "");

		ui_pipe.configureForUserInterface();
		checkErrStack(ui_pipe.create(&logical_dev, &ui_vertex_module, &ui_frag_module,
			swapchain.resolution.width, swapchain.resolution.height,
			&ui_pipe_layout, &renderpass, 1), "");
	}

	// Command Buffers
	checkErrStack(render_cmd_buffs.create(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()), "");

	vks::RenderingCmdBuffsUpdateInfo info = {};
	info.renderpass = &renderpass;
	info.frame_buffs = &frame_buffs;
	info.width = swapchain.resolution.width;
	info.height = swapchain.resolution.height;

	// 3D
	info.g3d_pipe_layout = &g3d_pipe_layout;
	info.g3d_pipe = &g3d_pipe;
	info.g3d_descp = &g3d_descp;
	info.g3d_vertex_buff = &vertex_buff;
	info.g3d_vertex_count = (uint32_t)gpu_3d_verts.size();

	// UI
	info.ui_descp = &ui_descp;
	info.ui_pipe_layout = &ui_pipe_layout;
	info.ui_pipe = &ui_pipe;
	info.ui_vertex_buff = &ui_vertex_buff;
	info.ui_vertex_count = (uint32_t)gpu_ui_verts.size();

	checkErrStack(render_cmd_buffs.update(info),
		"");

	// Syncronization
	checkErrStack(img_acquired.create(&logical_dev),
		"failed to create semaphore for acquiring image from swapchain");
	checkErrStack(rendering_ended_sem.create(&logical_dev),
		"failed to create semaphore for rendering ended");

	return ErrStack();
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

ErrStack Renderer::waitForRendering()
{
	checkVkRes(vkDeviceWaitIdle(logical_dev.logical_device), "");
	return ErrStack();
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

		this->gpu_3d_verts.resize(total_idxs_size);
		this->gpu_storage.resize(meshes.size());
	}

	// extract data
	uint64_t i = 0;

	for (uint64_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {

		LinkageMesh& mesh = meshes[mesh_idx];

		for (Poly& p : mesh.polys) {
			for (TesselationTris& t : p.tess_tris) {
				for (Vertex* v : t.vs) {

					vks::GPU_3D_Vertex& gpu_v = gpu_3d_verts[i];

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

	// UI Vertices
	{
		this->gpu_ui_verts.resize(6);

		// Positions
		gpu_ui_verts[0].pos = { 0, 0};
		gpu_ui_verts[1].pos = { 1, 0};
		gpu_ui_verts[2].pos = { 0, 1};

		gpu_ui_verts[3].pos = { 0, 1 };
		gpu_ui_verts[4].pos = { 1, 0 };
		gpu_ui_verts[5].pos = { 1, 1 };

		// UVs
		gpu_ui_verts[0].uv = { 0, 0 };
		gpu_ui_verts[1].uv = { 1, 0 };
		gpu_ui_verts[2].uv = { 0, 1 };

		gpu_ui_verts[3].uv = { 0, 1 };
		gpu_ui_verts[4].uv = { 1, 0 };
		gpu_ui_verts[5].uv = { 1, 1 };
	}
}

ErrStack Renderer::loadGPUData()
{
	// 3D
	checkErrStack(vertex_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_3d_verts.data(), vertex_buff.buff_alloc_info.size), "");
	checkErrStack(uniform_buff.load(&logical_dev, &cmd_pool, &staging_buff, &gpu_uniform, uniform_buff.buff_alloc_info.size), "");
	checkErrStack(storage_buff.load(&logical_dev, &cmd_pool, &staging_buff, gpu_storage.data(), storage_buff.buff_alloc_info.size), "");

	// UI
	checkErrStack(ui_vertex_buff.load(&logical_dev, &cmd_pool, &staging_buff,
		gpu_ui_verts.data(), ui_vertex_buff.buff_alloc_info.size), "");

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
