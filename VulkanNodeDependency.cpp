
// Standard
#include <execution>

#include "VulkanNodeDependency.h"


void VulkanNode::initInstance(Instance* inst)
{
	this->type = VulkanDependencyTypes::INSTANCE;
	this->vk_obj = inst;

	this->name = "Instance node";
}

void VulkanNode::initSurface(Surface* surf, VulkanNode* inst_parent)
{
	assert_cond(inst_parent->type == VulkanDependencyTypes::INSTANCE, "");

	this->type = VulkanDependencyTypes::SURFACE;
	this->vk_obj = surf;

	this->parents.push_back(inst_parent);
	inst_parent->children.push_back(this);

	this->name = "Surface node";
}

void VulkanNode::initDevice(Device* dev, VulkanNode* inst_parent, VulkanNode* surf_parent)
{
	assert_cond(inst_parent->type == VulkanDependencyTypes::INSTANCE, "");
	assert_cond(surf_parent->type == VulkanDependencyTypes::SURFACE, "");

	this->type = VulkanDependencyTypes::DEVICE;
	this->vk_obj = dev;

	this->parents.push_back(inst_parent);
	this->parents.push_back(surf_parent);
	inst_parent->children.push_back(this);
	surf_parent->children.push_back(this);

	this->name = "Device node";
}

void VulkanNode::initDeviceMemory(DeviceMemory* dev_mem, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::DEVICE_MEMORY;
	this->vk_obj = dev_mem;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Device Memmory node";
}

void VulkanNode::initSwapchain(Swapchain* swapchain, VulkanNode* dev_parent, VulkanNode* surf_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");
	assert_cond(surf_parent->type == VulkanDependencyTypes::SURFACE, "");

	this->type = VulkanDependencyTypes::SWAPCHAIN;
	this->vk_obj = swapchain;

	this->parents.push_back(dev_parent);
	this->parents.push_back(surf_parent);
	dev_parent->children.push_back(this);
	surf_parent->children.push_back(this);

	this->name = "Swapchain node";
}

void VulkanNode::initRenderpass(Renderpass* renderpass, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::RENDERPASS;
	this->vk_obj = renderpass;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Renderpass node";
}

void VulkanNode::initCmdPools(ComandPools* cmd_pool, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::COMAND_POOLS;
	this->vk_obj = cmd_pool;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Command Pool node";
}

void VulkanNode::initLoadCmdBuff(LoadComandBuffer* load_cmd_buff, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::LOAD_CMD_BUFF;
	this->vk_obj = load_cmd_buff;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Load Command Buffer node";
}

void VulkanNode::initImageViews(ImageViews* img_views, VulkanNode* dev_parent, VulkanNode* swapchain_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");
	assert_cond(swapchain_parent->type == VulkanDependencyTypes::SWAPCHAIN, "");

	this->type = VulkanDependencyTypes::IMAGEVIEW;
	this->vk_obj = img_views;

	this->parents.push_back(dev_parent);
	this->parents.push_back(swapchain_parent);
	dev_parent->children.push_back(this);
	swapchain_parent->children.push_back(this);

	this->name = "Image Views node";
}

void VulkanNode::initBuff(Buffer* buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");
	assert_cond(dev_mem_parent->type == VulkanDependencyTypes::DEVICE_MEMORY, "");

	this->vk_obj = buff;

	this->parents.push_back(dev_parent);
	this->parents.push_back(dev_mem_parent);
	dev_parent->children.push_back(this);
	dev_mem_parent->children.push_back(this);
}

void VulkanNode::initVertexBuff(Buffer* vertex_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent)
{
	assert_cond(vertex_buff->buff_usage == BufferUsage::VERTEX, "");

	initBuff(vertex_buff, dev_parent, dev_mem_parent);
	this->type = VulkanDependencyTypes::VERTEX_BUFFER;

	this->name = "Vertex Buffer node";
}

void VulkanNode::initIndexBuff(Buffer* index_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent)
{
	assert_cond(index_buff->buff_usage == BufferUsage::INDEX, "");

	initBuff(index_buff, dev_parent, dev_mem_parent);
	this->type = VulkanDependencyTypes::INDEX_BUFFER;

	this->name = "Index Buffer node";
}

void VulkanNode::initUniformBuff(Buffer* uniform_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent)
{
	assert_cond(uniform_buff->buff_usage == BufferUsage::UNIFORM, "");

	initBuff(uniform_buff, dev_parent, dev_mem_parent);
	this->type = VulkanDependencyTypes::UNIFORM_BUFFER;

	this->name = "Uniform Buffer node";
}

void VulkanNode::initStorageBuff(Buffer* storage_buff, VulkanNode* dev_parent, VulkanNode* dev_mem_parent)
{
	assert_cond(storage_buff->buff_usage == BufferUsage::STORAGE, "");

	initBuff(storage_buff, dev_parent, dev_mem_parent);
	this->type = VulkanDependencyTypes::STORAGE_BUFFER;

	this->name = "Storage Buffer node";
}

void VulkanNode::initFrameBuffers(FrameBuffers* frame_buffs, VulkanNode* dev_parent, VulkanNode* swapchain_parent,
	VulkanNode* img_views_parent, VulkanNode* renderpass_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");
	assert_cond(swapchain_parent->type == VulkanDependencyTypes::SWAPCHAIN, "");
	assert_cond(img_views_parent->type == VulkanDependencyTypes::IMAGEVIEW, "");
	assert_cond(renderpass_parent->type == VulkanDependencyTypes::RENDERPASS, "");

	this->type = VulkanDependencyTypes::FRAME_BUFFERS;
	this->vk_obj = frame_buffs;

	this->parents.push_back(dev_parent);
	this->parents.push_back(swapchain_parent);
	this->parents.push_back(img_views_parent);
	this->parents.push_back(renderpass_parent);
	dev_parent->children.push_back(this);
	swapchain_parent->children.push_back(this);
	img_views_parent->children.push_back(this);
	renderpass_parent->children.push_back(this);

	this->name = "Frame Buffers node";
}

void VulkanNode::initDescpLayout(DescriptorSets* descp_set, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::DESCP_SETS_LAYOUT;
	this->vk_obj = descp_set;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Descriptor Sets Layout node";
}

void VulkanNode::initShaders(std::vector<ShaderModule*>& shaders, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::SHADERS;
	this->vk_obj = shaders;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Shaders node";
}

void VulkanNode::initSync(VulkanSync* sync, VulkanNode* dev_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");

	this->type = VulkanDependencyTypes::SYNCRONIZATION;
	this->vk_obj = sync;

	this->parents.push_back(dev_parent);
	dev_parent->children.push_back(this);

	this->name = "Syncronization node";
}

void VulkanNode::initDescpUpdate(DescriptorSets* descp_set, VulkanNode* descp_layout_parent,
	VulkanNode* uniform_buff_parent, VulkanNode* storage_buff_parent)
{
	assert_cond(descp_layout_parent->type == VulkanDependencyTypes::DESCP_SETS_LAYOUT, "");
	assert_cond(uniform_buff_parent->type == VulkanDependencyTypes::UNIFORM_BUFFER, "");
	assert_cond(storage_buff_parent->type == VulkanDependencyTypes::STORAGE_BUFFER, "");

	this->type = VulkanDependencyTypes::DESCP_SETS_UPDATE;
	this->vk_obj = descp_set;

	this->parents.push_back(descp_layout_parent);
	this->parents.push_back(uniform_buff_parent);
	this->parents.push_back(storage_buff_parent);
	descp_layout_parent->children.push_back(this);
	uniform_buff_parent->children.push_back(this);
	storage_buff_parent->children.push_back(this);

	this->name = "Descriptor Sets Update node";
}

void VulkanNode::initPipelineLayout(GraphicsPipeline* pipe, VulkanNode* dev_parent,
	VulkanNode* descp_layout_parent)
{
	assert_cond(dev_parent->type == VulkanDependencyTypes::DEVICE, "");
	assert_cond(descp_layout_parent->type == VulkanDependencyTypes::DESCP_SETS_LAYOUT, "");

	this->type = VulkanDependencyTypes::PIPELINE_LAYOUT;
	this->vk_obj = pipe;

	this->parents.push_back(dev_parent);
	this->parents.push_back(descp_layout_parent);
	dev_parent->children.push_back(this);
	descp_layout_parent->children.push_back(this);

	this->name = "Pipeline Layout node";
}

void VulkanNode::initPipeline(GraphicsPipeline* pipe, VulkanNode* pipe_layout_parent, VulkanNode* swapchain_parent,
	VulkanNode* renderpass_parent)
{
	assert_cond(pipe_layout_parent->type == VulkanDependencyTypes::PIPELINE_LAYOUT, "");
	assert_cond(swapchain_parent->type == VulkanDependencyTypes::SWAPCHAIN, "");
	assert_cond(renderpass_parent->type == VulkanDependencyTypes::RENDERPASS, "");

	this->type = VulkanDependencyTypes::PIPELINE_PIPELINE;
	this->vk_obj = pipe;

	this->parents.push_back(pipe_layout_parent);
	this->parents.push_back(swapchain_parent);
	this->parents.push_back(renderpass_parent);
	pipe_layout_parent->children.push_back(this);
	swapchain_parent->children.push_back(this);
	renderpass_parent->children.push_back(this);

	this->name = "Graphics Pipeline node";
}

void VulkanNode::initComandBuff(ComandBuffers* cmd_buffs, VulkanNode* frame_buffs_parent)
{
	assert_cond(frame_buffs_parent->type == VulkanDependencyTypes::FRAME_BUFFERS, "");

	this->type = VulkanDependencyTypes::COMAND_BUFFERS;
	this->vk_obj = cmd_buffs;

	this->parents.push_back(frame_buffs_parent);
	frame_buffs_parent->children.push_back(this);

	this->name = "Command Buffers node";
}

void VulkanNode::initComandBuffUpdate(ComandBuffers* cmd_buffs, VulkanNode* cmd_buffs_parent,
	VulkanNode* pipeline_pipe_parent, VulkanNode* descp_update_parent,
	VulkanNode* vertex_buff_parent, VulkanNode* index_buff_parent)
{
	assert_cond(cmd_buffs_parent->type == VulkanDependencyTypes::COMAND_BUFFERS, "");
	assert_cond(pipeline_pipe_parent->type == VulkanDependencyTypes::PIPELINE_PIPELINE, "");
	assert_cond(descp_update_parent->type == VulkanDependencyTypes::DESCP_SETS_UPDATE, "");
	assert_cond(vertex_buff_parent->type == VulkanDependencyTypes::VERTEX_BUFFER, "");
	assert_cond(index_buff_parent->type == VulkanDependencyTypes::INDEX_BUFFER, "");

	this->type = VulkanDependencyTypes::COMAND_BUFFERS_UPDATE;
	this->vk_obj = cmd_buffs;

	this->parents.push_back(cmd_buffs_parent);
	this->parents.push_back(pipeline_pipe_parent);
	this->parents.push_back(descp_update_parent);
	this->parents.push_back(vertex_buff_parent);
	this->parents.push_back(index_buff_parent);
	cmd_buffs_parent->children.push_back(this);
	pipeline_pipe_parent->children.push_back(this);
	descp_update_parent->children.push_back(this);
	vertex_buff_parent->children.push_back(this);
	index_buff_parent->children.push_back(this);

	this->name = "Command Buffers Update node";
}

void VulkanNode::propagateUpdateStatus()
{
	needs_update = true;

	std::for_each(std::execution::par, children.begin(), children.end(), [](auto& node) {

		node->propagateUpdateStatus();
	});
}

ErrorStack VulkanNode::build()
{
	switch (type)
	{
	// Level 0, 1, 2
	case VulkanDependencyTypes::INSTANCE: {
		Instance* inst = std::get<Instance*>(vk_obj);
		return inst->build();
	}
	case VulkanDependencyTypes::SURFACE: {
		Surface* surf = std::get<Surface*>(vk_obj);
		return surf->build();
	}
	case VulkanDependencyTypes::DEVICE: {
		Device* dev = std::get<Device*>(vk_obj);
		return dev->build();
	}

	// Level 3
	case VulkanDependencyTypes::DEVICE_MEMORY: {
		DeviceMemory* dev_mem = std::get<DeviceMemory*>(vk_obj);
		dev_mem->build();
		return ErrorStack();
	}
	case VulkanDependencyTypes::COMAND_POOLS: {
		ComandPools* cmd_pools = std::get<ComandPools*>(vk_obj);
		return cmd_pools->build();
	}
	case VulkanDependencyTypes::LOAD_CMD_BUFF: {
		LoadComandBuffer* load_cmd_buff = std::get<LoadComandBuffer*>(vk_obj);
		return load_cmd_buff->build();
	}
	case VulkanDependencyTypes::SWAPCHAIN: {
		Swapchain* swapchain = std::get<Swapchain*>(vk_obj);
		return swapchain->build();
	}
	case VulkanDependencyTypes::RENDERPASS: {
		Renderpass* renderpass = std::get<Renderpass*>(vk_obj);
		return renderpass->build();
	}	
	case VulkanDependencyTypes::DESCP_SETS_LAYOUT: {
		DescriptorSets* descp_sets = std::get<DescriptorSets*>(vk_obj);
		return descp_sets->buildDescpSet();
	}
	case VulkanDependencyTypes::SHADERS: {
		auto shaders = std::get<std::vector<ShaderModule*>>(vk_obj);
		for (auto shader : shaders) {
			ErrorStack err = shader->build();
			if (err.isBad()) {
				return err;
			}
		}
		return ErrorStack();
	}
	case VulkanDependencyTypes::SYNCRONIZATION: {
		auto sync = std::get<VulkanSync*>(vk_obj);
		return sync->build();
	}

	// Level 4
	case VulkanDependencyTypes::IMAGEVIEW: {
		ImageViews* img_views = std::get<ImageViews*>(vk_obj);
		return img_views->build();
	}
	case VulkanDependencyTypes::VERTEX_BUFFER:
	case VulkanDependencyTypes::INDEX_BUFFER:
	case VulkanDependencyTypes::UNIFORM_BUFFER: 
	case VulkanDependencyTypes::STORAGE_BUFFER: {
		Buffer* buff = std::get<Buffer*>(vk_obj);
		return buff->build();
	}
	case VulkanDependencyTypes::PIPELINE_LAYOUT: {
		GraphicsPipeline* pipe = std::get<GraphicsPipeline*>(vk_obj);
		return pipe->buildLayoutAndCache();
	}
	
	// Level 5
	case VulkanDependencyTypes::FRAME_BUFFERS: {
		auto frame_buffs = std::get<FrameBuffers*>(vk_obj);
		return frame_buffs->build();
	}
	case VulkanDependencyTypes::DESCP_SETS_UPDATE: {
		DescriptorSets* descp_sets = std::get<DescriptorSets*>(vk_obj);

		// Instead of having 2 calls to vkUpdateDescriptorSets do this
		for (VulkanNode* parent : parents) {

			if (parent->type == VulkanDependencyTypes::UNIFORM_BUFFER &&
				parent->needs_update)
			{
				descp_sets->uniform_update = true;
			}

			if (parent->type == VulkanDependencyTypes::STORAGE_BUFFER &&
				parent->needs_update)
			{
				descp_sets->storage_update = true;
			}
		}

		descp_sets->update();
		return ErrorStack();
	}

	// Level 6
	case VulkanDependencyTypes::PIPELINE_PIPELINE: {
		GraphicsPipeline* pipe = std::get<GraphicsPipeline*>(vk_obj);
		return pipe->buildPipeline();
	}

	// Level 7
	case VulkanDependencyTypes::COMAND_BUFFERS: {
		ComandBuffers* cmd_buffs = std::get<ComandBuffers*>(vk_obj);
		return cmd_buffs->build();
	}

	// Level 8
	case VulkanDependencyTypes::COMAND_BUFFERS_UPDATE: {
		ComandBuffers* cmd_buffs = std::get<ComandBuffers*>(vk_obj);
		return cmd_buffs->updateRenderComands();
	}
	}

	return ErrorStack(code_location, " unrecognized dependency type");
}

void VulkanNode::destroy()
{
	switch (type) {
	// Level 0, 1, 2
	case VulkanDependencyTypes::INSTANCE: {
		Instance* inst = std::get<Instance*>(vk_obj);
		if (inst->instance != VK_NULL_HANDLE) {
			inst->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::SURFACE: {
		Surface* surf = std::get<Surface*>(vk_obj);
		if (surf->surface != VK_NULL_HANDLE) {
			surf->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::DEVICE: {
		Device* dev = std::get<Device*>(vk_obj);
		if (dev->logical_device != VK_NULL_HANDLE) {
			dev->destroy();
		}
		break;
	}

	// Level 3
	case VulkanDependencyTypes::DEVICE_MEMORY: {
		auto dev_mem = std::get<DeviceMemory*>(vk_obj);
		if (dev_mem->mem_recoms[0].size()) {
			dev_mem->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::COMAND_POOLS: {
		auto cmd_pools = std::get<ComandPools*>(vk_obj);
		if (cmd_pools->cmd_pools[0].cmd_pool != VK_NULL_HANDLE) {
			cmd_pools->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::LOAD_CMD_BUFF: {
		LoadComandBuffer* load_cmd_buff = std::get<LoadComandBuffer*>(vk_obj);
		if (load_cmd_buff->cmd_pool) {
			load_cmd_buff->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::SWAPCHAIN: {
		auto swapchain = std::get<Swapchain*>(vk_obj);
		if (swapchain->swapchain != VK_NULL_HANDLE) {
			swapchain->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::RENDERPASS: {
		auto renderpass = std::get<Renderpass*>(vk_obj);
		if (renderpass->renderpass != VK_NULL_HANDLE) {
			renderpass->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::DESCP_SETS_LAYOUT: {
		auto descp_set = std::get<DescriptorSets*>(vk_obj);
		if (descp_set->descp_layout != VK_NULL_HANDLE) {
			descp_set->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::SHADERS: {
		auto shaders = std::get<std::vector<ShaderModule*>>(vk_obj);
		for (auto shader : shaders) {
			if (shader->sh_module != VK_NULL_HANDLE) {
				shader->destroy();
			}		
		}
		break;
	}
	case VulkanDependencyTypes::SYNCRONIZATION: {
		auto sync = std::get<VulkanSync*>(vk_obj);
		if (sync->render_ended_fence != VK_NULL_HANDLE) {
			sync->destroy();
		}
		break;
	}

	// Level 4
	case VulkanDependencyTypes::IMAGEVIEW: {
		auto img_views = std::get<ImageViews*>(vk_obj);
		if (img_views->image_views.size()) {
			img_views->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::VERTEX_BUFFER:
	case VulkanDependencyTypes::INDEX_BUFFER:
	case VulkanDependencyTypes::UNIFORM_BUFFER:
	case VulkanDependencyTypes::STORAGE_BUFFER: {
		auto buff = std::get<Buffer*>(vk_obj);
		if (buff->buff != VK_NULL_HANDLE) {
			buff->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::PIPELINE_LAYOUT: {
		auto pipe = std::get<GraphicsPipeline*>(vk_obj);
		if (pipe->pipeline_layout != VK_NULL_HANDLE) {
			pipe->destroyLayoutAndCache();
		}
		break;
	}

	// Level 5
	case VulkanDependencyTypes::FRAME_BUFFERS: {
		auto frame_buffs = std::get<FrameBuffers*>(vk_obj);
		if (frame_buffs->frame_buffers.size()) {
			frame_buffs->destroy();
		}
		break;
	}
	case VulkanDependencyTypes::DESCP_SETS_UPDATE: {
		// nothing to destroy
		break;
	}

	// Level 6
	case VulkanDependencyTypes::PIPELINE_PIPELINE: {
		auto pipe = std::get<GraphicsPipeline*>(vk_obj);
		if (pipe->pipeline != VK_NULL_HANDLE) {
			pipe->destroyPipeline();
		}
		break;
	}

	// Level 7
	case VulkanDependencyTypes::COMAND_BUFFERS: {
		auto cmd_buffs = std::get<ComandBuffers*>(vk_obj);
		if (cmd_buffs->cmd_buff_tasks.size()) {
			cmd_buffs->destroy();
		}
		break;
	}

	// Level 8
	case VulkanDependencyTypes::COMAND_BUFFERS_UPDATE: {
		// nothing to destroy
		break;
	}
	default:
		std::cout << code_location << " unrecognized dependency type" << std::endl;
	}
}