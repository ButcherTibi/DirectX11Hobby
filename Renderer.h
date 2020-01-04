#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// 
#include "stb_image.h"

// mine
#include "Meshes.h"
#include "VulkanSystems.h"


class Camera
{
public:
	glm::vec3 position = { 0, 0, 0 };
	glm::quat rotation = {1, 0, 0, 0};

	float fov = 28.0f;
	float near_plane = 0.1f;
	float far_plane = 100.0f;

public:

	/* Camera local axes */

	glm::vec3 right();
	glm::vec3 up();
	glm::vec3 forward();
};


enum class ShadingMode {
	VERTEX,
	TESSELATION,
	POLYGON
};

//enum class CullingMode {
//	NORMAL_OUT,
//	NORMAL_IN,
//	CLOCKWISE_WINDING,
//	COUNTER_CLOCKWISE_WINDING
//};


class Renderer
{
public:
	ShadingMode shading_mode = ShadingMode::VERTEX;

public:
	Camera camera;
	std::vector<LinkageMesh> meshes;

	// Vulkan Systems
	vks::Instance instance;
	vks::Surface surface;
	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;
	vks::Swapchain swapchain;

	vks::CommandPool cmd_pool;

	// Images
	std::vector<VkImage> present_images;  // owned by swapchain
	vks::Image depth_img;
	vks::Image tex_img;

	// ImageView
	std::vector<vks::ImageView> present_views;
	vks::ImageView depth_view;
	vks::ImageView tex_view;

	// Samplers
	vks::Sampler sampler;

	// Renderpass
	vks::Renderpass renderpass;

	// Framebuffers
	vks::Framebuffers frame_buffs;

	// Buffers
	std::vector<vks::GPUVertex> gpu_verts;
	vks::Buffer vertex_buff;

	vks::GPUUniform gpu_uniform;
	vks::Buffer uniform_buff;

	std::vector<vks::GPUMeshProperties> gpu_storage;
	vks::Buffer storage_buff;

	vks::Buffer staging_buff;

	vks::DescriptorSetLayout descp_layout;
	vks::DescriptorPool descp_pool;
	vks::DescriptorSet descp_set;

	vks::ShaderModule vertex_module;
	vks::ShaderModule frag_module;
	vks::PipelineLayout pipe_layout;
	vks::GraphicsPipeline graphics_pipe;

	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

public:
	Renderer();

	ErrorStack create(HINSTANCE hinstance, HWND hwnd, uint32_t width, uint32_t height,
		std::vector<char>& vertex_shader_code, std::vector<char>& fragment_shader_code);


	/* Camera control */

	/* rotate the camera around its own position, up axis locked */
	void rotateCameraUpLocked(float delta_pitch, float delta_yaw);

	/* rotate camera around center, arcball motion */
	void orbitCameraArcball(glm::vec3 center, float delta_pitch, float delta_yaw);

	/* moves camera closer to center */
	void zoomCamera(glm::vec3 center, float zoom);

	/* moves the camera up,down, left, right relative to camera rotation */
	void panCamera(float delta_vertical, float delta_horizontal);
	
	void generateGPUData();


	/* Below GPU Command Only */

	ErrorStack waitForRendering();


	/* Swapchain */

	// get the actual rendering resolution that is supported by the surface
	void getRenderResolution(uint32_t& width, uint32_t& height);

	// get the rendering resolution that is being desired
	void getRequestedRenderResolution(uint32_t& width, uint32_t& height);

	void changeRenderResolution(uint32_t width, uint32_t height);


	/* GPU Data */

	ErrorStack loadGPUData();

	/*  */
	ErrorStack draw();
};

extern Renderer renderer;
