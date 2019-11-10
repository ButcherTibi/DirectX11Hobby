#pragma once

// keep one to one VkCommandPool to VkCommandBuffer mapping
// the alternative with fewer VkCommandPools is too tiresome
struct CmdBufferTask
{
	uint32_t idx;
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buff;
	ErrorStack err;
};


class ComandBuffers
{
public:
	// Parents
	Device* dev = nullptr;

	Swapchain* swapchain;
	Renderpass* renderpass;	

	Buffer* vertex_buff;
	Buffer* index_buff;

	FrameBuffers* frame_buffs;
	DescriptorSets* descp_sets;

	GraphicsPipeline* pipeline;

	std::vector<CmdBufferTask> cmd_buff_tasks;
public:
	ErrorStack build();

	void init(Device* device,
		Swapchain* swapchain, Renderpass* renderpass, 
		Buffer* vertex_buff, Buffer* index_buff,
		FrameBuffers* frame_buffers, DescriptorSets* descp_sets,
		GraphicsPipeline* pipeline);
	
	ErrorStack updateRenderComands();

	void destroy();

	~ComandBuffers();
};


class LoadComandBuffer
{
public:
	// Parents
	Device* dev = nullptr;

	VkCommandPool cmd_pool = VK_NULL_HANDLE;
	VkCommandBuffer cmd_buff;

public:
	void init(Device* device);

	ErrorStack build();

	void destroy();

	~LoadComandBuffer();
};


class RecordCmdBuffer
{
	LoadComandBuffer* load_cmd_buff;
	ErrorStack* err;

public:
	RecordCmdBuffer(LoadComandBuffer* load_cmd_buff, VkCommandBufferUsageFlags flags,
		ErrorStack* r_err);

	~RecordCmdBuffer();
};
