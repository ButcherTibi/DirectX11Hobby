#pragma once

// standard
#include <vector>

// GLM
#include <glm/vec3.hpp>

// mine
#include "Meshes.h"
#include "VulkanNodeDependency.h"

#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "VulkanFrames.h"
#include "VulkanCommandPool.h"
#include "VulkanBuffers.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffers.h"


struct NodeTask
{
	VulkanNode* node;
	ErrorStack err;

	NodeTask(VulkanNode* node);
};


class VulkanManagement
{
public:
	// Level 0, 1, 2
	Instance instance;
	VulkanNode inst_node;

	Surface surf;
	VulkanNode surf_node;

	Device dev;
	VulkanNode dev_node;

	// Level 3
	DeviceMemory dev_mem;
	VulkanNode dev_mem_node;

	//ComandPools cmd_pools;
	//VulkanNode cmd_pools_node;

	LoadComandBuffer load_cmd_buff;
	VulkanNode load_cmd_buff_node;

	Swapchain swapchain;
	VulkanNode swapchain_node;

	Renderpass renderpass;
	VulkanNode renderpass_node;

	DescriptorSets descp_sets;
	VulkanNode descp_layout_node;

	ShaderModule vert_module;
	ShaderModule frag_module;
	VulkanNode shaders_node;

	VulkanSync sync;
	VulkanNode sync_node;
	
	// Level 4
	ImageViews img_views;
	VulkanNode img_views_node;

	// Vertex Buffers
	bool vertex_load = false;
	std::vector<GPUVertex>* verts;
	Buffer vertex_buff;
	VulkanNode vertex_buff_node;

	// Index buffer
	bool index_load = false;
	std::vector<uint32_t>* indexs;
	Buffer index_buff;
	VulkanNode index_buff_node;

	// Uniform buffer
	bool uniform_load = false;
	void* uniform_data;
	size_t uniform_size;

	Buffer uniform_buff;
	VulkanNode uniform_buff_node;
	
	// Storage buffer
	bool storage_load = false;
	void* storage_data;
	size_t storage_size;

	Buffer storage_buff;
	VulkanNode storage_buff_node;

	GraphicsPipeline graphics_pipe;
	VulkanNode pipe_layout_node;

	// Level 5
	FrameBuffers frame_buffs;
	VulkanNode frame_buffs_node;

	VulkanNode descp_update_node;

	// Level 6
	VulkanNode pipe_pipeline_node;

	// Level 7
	ComandBuffers cmd_buffs;
	VulkanNode cmd_buff_node;

	// Level 8
	VulkanNode cmd_buff_update_node;

	// Dependency Levels
	std::array<std::vector<NodeTask>, 8> level_nodes;

private:

	// 
public:
	ErrorStack init(HINSTANCE hinstance, HWND hwnd);

	ErrorStack waitForRendering();

	// Settings methods

	// Instance
	void changeDebugSeverity(bool verbose, bool warning, bool info = false);

	// Instance + Device
	//void changeDebugMode(bool enable);


	/* Swapchain */

	// get the rendering resolution that is being aimed
	void getRequestedRenderResolution(uint32_t& width, uint32_t& height);

	// get the actual rendering resolution that is supported by the surface
	void getRenderResolution(uint32_t& width, uint32_t& height);

	void changeRequestedRenderResolution(uint32_t width, uint32_t height);
	void changePresentationMode(VkPresentModeKHR mode);

	// Shader Modules
	void changeVertexCode(const std::vector<char>& code);
	void changeFragCode(const std::vector<char>& code);

	// Buffers
	void loadVertexData(std::vector<GPUVertex>* verts);
	void loadIndexData(std::vector<uint32_t>* index);
	void loadUniformData(void* raw_data, size_t load_size);
	void loadStorageData(void* raw_data, size_t load_size);


	// 
	ErrorStack rebuild();

	ErrorStack draw();

	~VulkanManagement();
};


class Camera
{
public:
	glm::vec3 position = { 0, 0, 0 };
	glm::quat rotation = {1, 0, 0, 0};

	float fov = 90.0f;
	float near_plane = 0.1f;
	float far_plane = 100.0f;

public:

	/* Camera local axes */

	glm::vec3 right();
	glm::vec3 up();
	glm::vec3 forward();
};


class Renderer
{
public:
	VulkanManagement vk_man;

	Camera camera;
	std::vector<LinkageMesh*> meshes;

	std::vector<GPUVertex> gpu_verts;
	std::vector<uint32_t> gpu_indexs;

	std::vector<GPUMeshProperties> gpu_meshes;

	GPUUniform gpu_uniform;

public:
	void resizeScreenResolution(uint32_t width, uint32_t height);

	// Camera control

	/* rotate the camera around its own position, up axis locked */
	void rotateCameraUpLocked(float delta_pitch, float delta_yaw);

	/* rotate camera around center, arcball motion */
	void orbitCameraArcball(glm::vec3 center, float delta_pitch, float delta_yaw);

	/* moves camera closer to center */
	void zoomCamera(glm::vec3 center, float zoom);

	/* moves the camera up,down, left, right relative to camera rotation */
	void panCamera(float delta_vertical, float delta_horizontal);

	void loadMeshesToBuffs();
};