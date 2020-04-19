#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// mine
#include "CommonTypes.h"
#include "Meshes.h"
#include "TextRendering.h"
#include "VulkanSystems.h"


class Camera {
public:
	glm::vec3 position = { 0, 0, 0 };
	glm::quat rotation = {1, 0, 0, 0};

	float fov = 28.0f;
	float near_plane = 0.1f;
	float far_plane = 100.0f;

public:
	glm::vec3 right();
	glm::vec3 up();
	glm::vec3 forward();

	/* rotate the camera around its own position, up axis locked */
	void rotateCameraUpLocked(float delta_pitch, float delta_yaw);

	/* rotate camera around center, arcball motion */
	void orbitCameraArcball(glm::vec3 center, float delta_pitch, float delta_yaw);

	/* moves camera closer to center */
	void zoomCamera(glm::vec3 center, float zoom);

	/* moves the camera up,down, left, right relative to camera rotation */
	void panCamera(float delta_vertical, float delta_horizontal);
};


struct RenderingContent {
	// Surface
	HINSTANCE* hinstance = nullptr;
	HWND* hwnd = nullptr;

	// Swapchain
	uint32_t width = 0;
	uint32_t height = 0;

	// 3D Subpass
	BasicBitmap* mesh_diffuse = nullptr;
	std::vector<LinkageMesh>* meshes = nullptr;
	Camera* camera = nullptr;

	std::vector<char>* g3d_vert_shader_code;
	std::vector<char>* g3d_frag_shader_code;

	// UI Subpass
	bool char_atlas_changed = false;
	TextStuff* text_rendering = nullptr;

	std::vector<char>* ui_vert_shader_code;
	std::vector<char>* ui_frag_shader_code;

	// Compose Subpass
	std::vector<char>* comp_vert_shader_code;
	std::vector<char>* comp_frag_shader_code;
};

class Renderer {
public:
	// Vulkan Systems
	vks::Instance instance;
	vks::Surface surface;

	vks::PhysicalDevice phys_dev;
	vks::LogicalDevice logical_dev;

	vks::CommandPool cmd_pool;
	vks::Buffer staging_buff;

	// Frame
	vks::Swapchain swapchain;

	vks::Image g3d_color_MSAA_img;
	vks::Image g3d_depth_img;
	vks::Image g3d_color_resolve_img;
	vks::Image ui_color_img;

	vks::Renderpass renderpass;
	vks::Framebuffers frame_buffs;

	// 3D Subpass
	vks::Descriptor g3d_descp;

	vks::Image mesh_diffuse_img;
	vks::Sampler mesh_diffuse_sampler;

	uint32_t g3d_vertex_count;
	vks::Buffer g3d_vertex_buff;
	vks::Buffer g3d_uniform_buff;
	vks::Buffer g3d_storage_buff;

	vks::ShaderModule g3d_vertex_module;
	vks::ShaderModule g3d_frag_module;

	vks::PipelineLayout g3d_pipe_layout;
	vks::GraphicsPipeline g3d_pipe;

	// UI Subpass
	vks::Descriptor ui_descp;

	vks::Image ui_char_atlas_img;
	vks::Sampler ui_char_atlas_sampler;

	std::vector<UI_DrawBatch> ui_batches;
	vks::Buffer ui_vertex_buff;
	vks::Buffer ui_storage_buff;

	vks::ShaderModule ui_vertex_module;
	vks::ShaderModule ui_frag_module;

	vks::PipelineLayout ui_pipe_layout;
	vks::GraphicsPipeline ui_pipe;

	// Composition Subpass
	vks::Descriptor compose_descp;

	vks::ShaderModule compose_vert_module;
	vks::ShaderModule compose_frag_module;

	vks::PipelineLayout compose_pipe_layout;
	vks::GraphicsPipeline compose_pipe;

	// Command Buffer
	vks::RenderingComandBuffers render_cmd_buffs;

	vks::Semaphore img_acquired;
	vks::Semaphore rendering_ended_sem;

private:
	ErrStack create3D_DiffuseTexture(BasicBitmap& mesh_diffuse);
	ErrStack recreate3D_MeshBuffers(std::vector<LinkageMesh>& meshes);
	ErrStack recreate3D_UniformBuffer(Camera& camera);

	ErrStack recreateUI_MeshBuffers(TextStuff& txt_rend);

public:
	Renderer();

	ErrStack recreate(RenderingContent& content);

	ErrStack draw();

	ErrStack waitForRendering();
};

extern Renderer renderer;
