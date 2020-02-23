#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

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


//enum class CullingMode {
//	NORMAL_OUT,
//	NORMAL_IN,
//	CLOCKWISE_WINDING,
//	COUNTER_CLOCKWISE_WINDING
//};


struct RendererCreateInfo {
	// Window
	HINSTANCE hinstance;
	HWND hwnd;
	uint32_t width;
	uint32_t height;

	// Shader Code
	std::vector<char>* g3d_vert_shader_code;
	std::vector<char>* g3d_frag_shader_code;

	std::vector<char>* ui_vert_shader_code;
	std::vector<char>* ui_frag_shader_code;
};

struct BasicBitmap {
	std::vector<uint8_t> colors;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

	size_t mem_size;

	void calcMemSize();
};

class Renderer
{
public:
	Camera camera;
	std::vector<LinkageMesh> meshes;

	BasicBitmap mesh_difuse;
	BasicBitmap symbol_atlas;

	// Vulkan Systems
	vks::Instance instance;
	vks::Surface surface;
	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;
	vks::Swapchain swapchain;

	vks::CommandPool cmd_pool;

	// Images
	std::vector<VkImage> swapchain_images;  // owned by swapchain
	vks::Image g3d_color_MSAA_img;
	vks::Image g3d_depth_img;
	vks::Image g3d_color_resolve_img;

	// Textures
	vks::Image tex_img;
	vks::Image ui_symbol_atlas_img;

	// ImageView
	std::vector<vks::ImageView> swapchain_img_views;
	vks::ImageView g3d_color_MSAA_view;
	vks::ImageView g3d_depth_view;
	vks::ImageView g3d_color_resolve_view;

	// Texture ImageViews
	vks::ImageView tex_view;
	vks::ImageView ui_symbol_atlas_view;

	// Samplers
	vks::Sampler sampler;
	vks::Sampler ui_symbol_sampler;

	// Renderpass
	vks::Renderpass renderpass;

	// Framebuffers
	vks::Framebuffers frame_buffs;

	// Buffers
	std::vector<vks::GPU_UI_Vertex> gpu_ui_verts;
	vks::Buffer ui_vertex_buff;
	std::vector<vks::GPU_3D_Vertex> gpu_3d_verts;
	vks::Buffer vertex_buff;	

	vks::GPUUniform gpu_uniform;
	vks::Buffer uniform_buff;

	std::vector<vks::GPUMeshProperties> gpu_storage;
	vks::Buffer storage_buff;

	vks::Buffer staging_buff;		

	// 3D Pipeline
	vks::Descriptor g3d_descp;

	vks::PipelineLayout g3d_pipe_layout;
	vks::ShaderModule g3d_vertex_module;
	vks::ShaderModule g3d_frag_module;
	vks::GraphicsPipeline g3d_pipe;

	// UI Pipeline
	vks::Descriptor ui_descp;

	vks::PipelineLayout ui_pipe_layout;
	vks::ShaderModule ui_vertex_module;
	vks::ShaderModule ui_frag_module;
	vks::GraphicsPipeline ui_pipe;

	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

public:
	Renderer();

	ErrStack create(RendererCreateInfo& info);


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

	ErrStack waitForRendering();


	/* Swapchain */

	// get the actual rendering resolution that is supported by the surface
	void getRenderResolution(uint32_t& width, uint32_t& height);

	// get the rendering resolution that is being desired
	void getRequestedRenderResolution(uint32_t& width, uint32_t& height);

	void changeRenderResolution(uint32_t width, uint32_t height);


	/* GPU Data */

	ErrStack loadGPUData();

	/*  */
	ErrStack draw();
};

extern Renderer renderer;
