#pragma once

// Standard
#include <vector>
#include <variant>

// mine
#include "ErrorStuff.h"

// std::variant needs full type
#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "VulkanFrames.h"
#include "VulkanCommandPool.h"
#include "VulkanBuffers.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffers.h"
#include "VulkanSyncronization.h"


enum class VulkanDependencyTypes
{
	INSTANCE,

	SURFACE,

	DEVICE,

	DEVICE_MEMORY,
	COMAND_POOLS,
	LOAD_CMD_BUFF,
	SWAPCHAIN,
	RENDERPASS,
	DESCP_SETS_LAYOUT,
	SHADERS,
	SYNCRONIZATION,
	
	IMAGEVIEW,	
	VERTEX_BUFFER,
	INDEX_BUFFER,
	UNIFORM_BUFFER,
	STORAGE_BUFFER,
	STAGING_BUFFER,
	PIPELINE_LAYOUT,

	FRAME_BUFFERS,
	DESCP_SETS_UPDATE,

	PIPELINE_PIPELINE,

	COMAND_BUFFERS,

	COMAND_BUFFERS_UPDATE,

	VulkanDependencyTypes_UNINITILIZED
};


// Used to specify a dependecy in data for a vulkan object wrapper
// because every vulkan object is imutable if you change an object that 
// you used to create some other vulkan object, you need to update the latter as well
class VulkanNode
{
public:
	// Flags
	bool needs_update = true;
	//bool unregister_flag = false;

	std::vector<VulkanNode*> parents;  // this depends on (if any changes then update this)
	std::vector<VulkanNode*> children;  // who needs this

	std::string name;
public:
	// Constructors
	// first argument is object to operate on, others a dependecies
	void initInstance(Instance* inst);
	void initSurface(Surface* surf, VulkanNode* inst_parent);
	void initDevice(Device* dev, VulkanNode* inst_parent, VulkanNode* surf_parent);

	void initDeviceMemory(DeviceMemory* dev_mem, VulkanNode* dev_parent);
	void initSwapchain(Swapchain* swapchain, VulkanNode* dev_parent, VulkanNode* surf_parent);
	void initRenderpass(Renderpass* renderpass, VulkanNode* dev_parent);
	void initCmdPools(ComandPools* cmd_pool, VulkanNode* dev_parent);
	void initLoadCmdBuff(LoadComandBuffer* load_cmd_buff, VulkanNode* dev_parent);
	void initDescpLayout(DescriptorSets* descp_set, VulkanNode* dev_parent);
	void initShaders(std::vector<ShaderModule*>& shaders, VulkanNode* dev_parent);
	void initSync(VulkanSync* sync, VulkanNode* dev_parent);

	void initImageViews(ImageViews* img_views, VulkanNode* dev_parent, VulkanNode* swapchain_parent);
	void initBuff(Buffer* buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent);
	void initVertexBuff(Buffer* vertex_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent);
	void initIndexBuff(Buffer* index_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent);
	void initUniformBuff(Buffer* uniform_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent);
	void initStorageBuff(Buffer* storage_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent);
	void initPipelineLayout(GraphicsPipeline* pipe, VulkanNode* dev_parent, VulkanNode* descp_layout_parent);

	void initFrameBuffers(FrameBuffers* frame_buffs, VulkanNode* dev_parent, VulkanNode* swapchain_parent,
		VulkanNode* img_views_parent, VulkanNode* renderpass_parent);
	void initDescpUpdate(DescriptorSets* descp_set, VulkanNode* descp_sets_parent, 
		VulkanNode* uniform_buff_parent, VulkanNode* storage_buff_parent);

	void initPipeline(GraphicsPipeline* pipe, VulkanNode* pipe_layout_parent, VulkanNode* swapchain_parent,
		VulkanNode* renderpass_parent);

	void initComandBuff(ComandBuffers* cmd_buffs, VulkanNode* frame_buffs_parent);

	// Depends on everything . . .
	void initComandBuffUpdate(ComandBuffers* cmd_buffs, VulkanNode* cmd_buffs_parent,
		VulkanNode* pipeline_pipe_parent, VulkanNode* descp_update_parent, 
		VulkanNode* vertex_buff_parent, VulkanNode* index_buff_parent);

	// Signal
	void propagateUpdateStatus();

	// Type
	VulkanDependencyTypes type;
	std::variant<Instance*, Surface*, Device*, 
		DeviceMemory*, Swapchain*, Renderpass*, ComandPools*, VulkanSync*,
		ImageViews*, Buffer*, FrameBuffers*, DescriptorSets*,
		std::vector<ShaderModule*>,
		GraphicsPipeline*,
		ComandBuffers*, LoadComandBuffer*> vk_obj;

	// Actions
	ErrorStack build();
	void destroy();
};
