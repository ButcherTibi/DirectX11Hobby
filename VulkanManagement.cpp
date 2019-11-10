
// Standard
#include <iostream>
#include <atomic>
#include <execution>
#include <algorithm>

// GLM
#include <glm/gtc/matrix_transform.hpp>

// mine
#include "FileIO.h"
#include "ErrorStuff.h"

#include "Renderer.h"


NodeTask::NodeTask(VulkanNode* node)
{
	this->node = node;
}

ErrorStack VulkanManagement::init(HINSTANCE hinstance, HWND hwnd)
{
	ErrorStack res;

	// Level 0
	instance.defaultValues();
	inst_node.initInstance(&instance);
	level_nodes[0].push_back(NodeTask(&inst_node));

	// Level 1
	surf.init(hinstance, hwnd, &instance);
	surf_node.initSurface(&surf, &inst_node);
	level_nodes[1].push_back(&surf_node);

	// Level 2
	dev.init(&instance, &surf);
	dev_node.initDevice(&dev, &inst_node, &surf_node);
	level_nodes[2].push_back(&dev_node);

	// Level 3
	dev_mem.init(&dev);
	dev_mem_node.initDeviceMemory(&dev_mem, &dev_node);
	level_nodes[3].push_back(&dev_mem_node);

	cmd_pools.init(&dev);
	cmd_pools_node.initCmdPools(&cmd_pools, &dev_node);
	level_nodes[3].push_back(&cmd_pools_node);

	load_cmd_buff.init(&dev);
	load_cmd_buff_node.initLoadCmdBuff(&load_cmd_buff, &dev_node);
	level_nodes[3].push_back(&load_cmd_buff_node);

	swapchain.init(&dev, &surf);
	swapchain_node.initSwapchain(&swapchain, &dev_node, &surf_node);
	level_nodes[3].push_back(&swapchain_node);

	renderpass.init(&dev);
	renderpass_node.initRenderpass(&renderpass, &dev_node);
	level_nodes[3].push_back(&renderpass_node);

	descp_sets.dev = &dev;
	descp_layout_node.initDescpLayout(&descp_sets, &dev_node);
	level_nodes[3].push_back(&descp_layout_node);

	// Vertex shader module
	{
		auto vert_code_path = Path("E:/my_work/Vulkan/Sculpt/Sculpt/shaders/vert.spv");
		std::vector<char> vertex_shader_code;
		res = vert_code_path.read(vertex_shader_code);
		if (res.isBad()) {
			res.report(code_location, "failed to read vertex shader code file");
			return res;
		}
		vert_module.init(&dev, vertex_shader_code, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	}

	// Fragment Shader module
	{
		auto frag_code_path = Path("E:/my_work/Vulkan/Sculpt/Sculpt/shaders/frag.spv");
		std::vector<char> frag_shader_code;
		res = frag_code_path.read(frag_shader_code);
		if (res.isBad()) {
			res.report(code_location, "failed to read fragment shader code file");
			return res;
		}
		frag_module.init(&dev, frag_shader_code, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	std::vector<ShaderModule*> shaders = { &vert_module, &frag_module };
	shaders_node.initShaders(shaders, &dev_node);
	level_nodes[3].push_back(&shaders_node);

	sync.init(&dev);
	sync_node.initSync(&sync, &dev_node);
	level_nodes[3].push_back(&sync_node);

	// Level 4
	img_views.init(&dev, &swapchain);
	img_views_node.initImageViews(&img_views, &dev_node, &swapchain_node);
	level_nodes[4].push_back(&img_views_node);

	vertex_buff.init(&dev, &dev_mem, BufferUsage::VERTEX, MemoryUsage::GPU_BULK_DATA);
	vertex_buff_node.initVertexBuff(&vertex_buff, &dev_node, &dev_mem_node);
	level_nodes[4].push_back(&vertex_buff_node);

	index_buff.init(&dev, &dev_mem, BufferUsage::INDEX, MemoryUsage::GPU_UPLOAD);
	index_buff_node.initIndexBuff(&index_buff, &dev_node, &dev_mem_node);
	level_nodes[4].push_back(&index_buff_node);

	uniform_buff.init(&dev, &dev_mem, BufferUsage::UNIFORM, MemoryUsage::GPU_UPLOAD);
	uniform_buff_node.initUniformBuff(&uniform_buff, &dev_node, &dev_mem_node);
	level_nodes[4].push_back(&uniform_buff_node);

	storage_buff.init(&dev, &dev_mem, BufferUsage::STORAGE, MemoryUsage::GPU_BULK_DATA);
	storage_buff_node.initStorageBuff(&storage_buff, &dev_node, &dev_mem_node);
	level_nodes[4].push_back(&storage_buff_node);

	graphics_pipe.init(&dev, &swapchain, &renderpass, &descp_sets, &vert_module, &frag_module);	
	pipe_layout_node.initPipelineLayout(&graphics_pipe, &dev_node, &descp_layout_node);
	level_nodes[4].push_back(&pipe_layout_node);
	
	// Level 5
	frame_buffs.init(&dev, &swapchain, &img_views, &renderpass);
	frame_buffs_node.initFrameBuffers(&frame_buffs, &dev_node, &swapchain_node, &img_views_node, &renderpass_node);
	level_nodes[5].push_back(&frame_buffs_node);
	
	descp_sets.uniform_buff = &uniform_buff;
	descp_sets.storage_buff = &storage_buff;
	descp_update_node.initDescpUpdate(&descp_sets, &descp_layout_node, &uniform_buff_node, &storage_buff_node);
	level_nodes[5].push_back(&descp_update_node);
	
	// Level 6
	pipe_pipeline_node.initPipeline(&graphics_pipe, &pipe_layout_node, &swapchain_node, &renderpass_node);
	level_nodes[5].push_back(&pipe_pipeline_node);
	
	// Level 7
	cmd_buffs.init(&dev, &swapchain, &renderpass, &vertex_buff, &index_buff, &frame_buffs, &descp_sets, &graphics_pipe);
	cmd_buff_node.initComandBuff(&cmd_buffs, &frame_buffs_node);
	level_nodes[6].push_back(&cmd_buff_node);
	
	// Level 8
	cmd_buff_update_node.initComandBuffUpdate(&cmd_buffs, &cmd_buff_node, &pipe_pipeline_node, &descp_update_node,
		&vertex_buff_node, &index_buff_node);
	level_nodes[7].push_back(&cmd_buff_update_node);

	return ErrorStack();
}

ErrorStack VulkanManagement::waitForRendering()
{
	if (sync.render_ended_fence != VK_NULL_HANDLE) {

		VkResult vk_res = vkWaitForFences(dev.logical_device, 1, &sync.render_ended_fence,
			VK_TRUE, UINT64_MAX);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_WAIT_ON_FENCE, code_location, "failed to wait on render_ended_fence fence");
		}

		vk_res = vkResetFences(dev.logical_device, 1, &sync.render_ended_fence);
		if (vk_res != VK_SUCCESS) {
			return ErrorStack(vk_res, ExtraError::FAILED_TO_RESET_FENCE, code_location, "failed to reset render_ended_fence fence");
		}
	}
	return ErrorStack();
}

void VulkanManagement::changeDebugSeverity(bool verbose, bool warning, bool info)
{
	VkDebugUtilsMessageSeverityFlagsEXT flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	if (verbose) {
		flags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	}
	if (info) {
		flags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	}
	if (warning) {
		flags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	}

	instance.debug_msg_severity = flags;

	inst_node.propagateUpdateStatus();
}

void VulkanManagement::getRequestedRenderResolution(uint32_t& width, uint32_t& height)
{
	width = swapchain.req_width;
	height = swapchain.req_height;
}

void VulkanManagement::getRenderResolution(uint32_t& width, uint32_t& height)
{
	width = swapchain.resolution.width;
	height = swapchain.resolution.height;
}

void VulkanManagement::changeRequestedRenderResolution(uint32_t width, uint32_t height)
{
	swapchain.req_width = width;
	swapchain.req_height = height;

	swapchain_node.propagateUpdateStatus();
}

void VulkanManagement::changePresentationMode(VkPresentModeKHR mode)
{
	swapchain.presentation_mode = mode;

	swapchain_node.propagateUpdateStatus();
}

void VulkanManagement::changeVertexCode(const std::vector<char>& code)
{
	vert_module.code = code;

	shaders_node.propagateUpdateStatus();
}

void VulkanManagement::changeFragCode(const std::vector<char>& code)
{
	frag_module.code = code;

	shaders_node.propagateUpdateStatus();
}

void VulkanManagement::loadVertexData(std::vector<GPUVertex>* verts)
{
	vertex_load = true;
	this->verts = verts;

	VkDeviceSize load_size = sizeof(GPUVertex) * verts->size();

	// signal if rebuild of vertex buffer required
	if (load_size > vertex_buff.buff_size) {
		vertex_buff.buff_size = load_size;
		vertex_buff_node.propagateUpdateStatus();
	}
}

void VulkanManagement::loadIndexData(std::vector<uint32_t>* indexs)
{
	index_load = true;
	this->indexs = indexs;

	VkDeviceSize load_size = sizeof(uint32_t) * indexs->size();

	if (load_size > index_buff.buff_size) {
		index_buff.buff_size = load_size;
		index_buff_node.propagateUpdateStatus();
	}
}

void VulkanManagement::loadUniformData(void* raw_data, size_t load_size)
{
	uniform_load = true;
	this->uniform_data = raw_data;
	this->uniform_size = load_size;

	if (load_size > uniform_buff.buff_size) {
		uniform_buff.buff_size = load_size;
		uniform_buff_node.propagateUpdateStatus();
	}
}

void VulkanManagement::loadStorageData(void* raw_data, size_t load_size)
{
	storage_load = true;
	this->storage_data = raw_data;
	this->storage_size = load_size;

	if (load_size > storage_buff.buff_size) {
		storage_buff.buff_size = load_size;
		storage_buff_node.propagateUpdateStatus();
	}
}

ErrorStack VulkanManagement::rebuild()
{	
	// Destroy
	size_t i;
	for (i = level_nodes.size() - 1; i > 0; i--) {

		std::for_each(std::execution::seq, level_nodes[i].begin(), level_nodes[i].end(), [](NodeTask& task) {

			if (task.node->needs_update) {
				task.node->destroy();
			}
		});	
	}

	// ReBuild
	for (i = 0; i < level_nodes.size(); i++) {

		std::for_each(std::execution::seq, level_nodes[i].begin(), level_nodes[i].end(), [](NodeTask& task) {

			if (task.node->needs_update) {
				task.err = task.node->build();
			}
		});

		for (auto& task : level_nodes[i]) {
			
			if (task.err.isBad()) {
				task.err.report(code_location, "failed to rebuild node " + task.node->name);
				return task.err;
			}
		}	
	}

	// Reset Update Status
	for (i = 0; i < level_nodes.size(); i++) {

		std::for_each(std::execution::seq, level_nodes[i].begin(), level_nodes[i].end(), [](NodeTask& task) {

			task.node->needs_update = false;
		});
	}

	// Direct load
	{
		if (vertex_load && vertex_buff.load_type == LoadType::MEMCPY) {
			vertex_load = false;
			std::memcpy(vertex_buff.alloc->data, verts->data(), sizeof(GPUVertex) * verts->size());
		}
		if (index_load && index_buff.load_type == LoadType::MEMCPY) {
			index_load = false;
			std::memcpy(index_buff.alloc->data, indexs->data(), sizeof(uint32_t) * indexs->size());
		}
		if (uniform_load && uniform_buff.load_type == LoadType::MEMCPY) {
			uniform_load = false;
			std::memcpy(uniform_buff.alloc->data, uniform_data, uniform_size);
		}
		if (storage_load && storage_buff.load_type == LoadType::MEMCPY) {
			storage_load = false;
			std::memcpy(storage_buff.alloc->data, storage_data, storage_size);
		}
	}

	// Staged Load
	{
		ErrorStack record_err;
		ErrorStack err;

		//Begin Command Buffer Recording
		{
			auto record_cmd_buff = RecordCmdBuffer(&load_cmd_buff, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &record_err);
			if (record_err.isBad()) {
				return err;
			}

			if (vertex_load) {

				vertex_load = false;
				
				std::memcpy(vertex_buff.staging_alloc->data, verts->data(), sizeof(GPUVertex) * verts->size());

				VkBufferCopy copy_region = {};
				copy_region.size = sizeof(GPUVertex) * verts->size();
				vkCmdCopyBuffer(load_cmd_buff.cmd_buff, vertex_buff.staging_buff, vertex_buff.buff, 1, &copy_region);
			}

			if (index_load) {

				index_load = false;

				std::memcpy(index_buff.staging_alloc->data, indexs->data(), sizeof(uint32_t) * indexs->size());

				VkBufferCopy copy_region = {};
				copy_region.size = sizeof(uint32_t) * indexs->size();
				vkCmdCopyBuffer(load_cmd_buff.cmd_buff, index_buff.staging_buff, index_buff.buff, 1, &copy_region);
			}

			if (uniform_load) {

				uniform_load = false;

				std::memcpy(uniform_buff.staging_alloc->data, uniform_data, uniform_size);

				VkBufferCopy copy_region = {};
				copy_region.size = uniform_size;
				vkCmdCopyBuffer(load_cmd_buff.cmd_buff, uniform_buff.staging_buff, uniform_buff.buff, 1, &copy_region);
			}

			if (storage_load ) {

				storage_load = false;

				std::memcpy(storage_buff.staging_alloc->data, storage_data, storage_size);

				VkBufferCopy copy_region = {};
				copy_region.size = storage_size;
				vkCmdCopyBuffer(load_cmd_buff.cmd_buff, storage_buff.staging_buff, storage_buff.buff, 1, &copy_region);
			}
		}
		if (record_err.isBad()) {
			return err;
		}
		// End Command Buffer Recording

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &load_cmd_buff.cmd_buff;

		VkResult vk_err = vkQueueSubmit(dev.queue, 1, &submit_info, sync.buff_upload_done);
		if (vk_err != VK_SUCCESS) {
			return ErrorStack(vk_err, ExtraError::FAILED_TO_SUBMIT_QUEUE, code_location, "failed to submit command buffer to queue");
		}
		vk_err = vkWaitForFences(dev.logical_device, 1, &sync.buff_upload_done, VK_TRUE, UINT64_MAX);
		if (vk_err != VK_SUCCESS) {
			return ErrorStack(vk_err, ExtraError::FAILED_TO_WAIT_ON_FENCE, code_location, "failed to wait on fence");
		}
		vk_err = vkResetFences(dev.logical_device, 1, &sync.buff_upload_done);
		if (vk_err != VK_SUCCESS) {
			return ErrorStack(vk_err, ExtraError::FAILED_TO_RESET_FENCE, code_location, "failed to reset fence");
		}
	}

	return ErrorStack();
}

ErrorStack VulkanManagement::draw()
{
	ErrorStack res;
	VkResult vk_res;

	uint32_t image_index;

	vk_res = vkAcquireNextImageKHR(dev.logical_device, swapchain.swapchain, UINT64_MAX,
		sync.img_acquired, VK_NULL_HANDLE, &image_index);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, ExtraError::FAILED_TO_ACQUIRE_IMAGE, code_location, "failed to acquire next image in swapchain");
	}

	// Submit Queue
	VkSubmitInfo queue_submit_info = {};
	queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	queue_submit_info.waitSemaphoreCount = 1;
	queue_submit_info.pWaitSemaphores = &sync.img_acquired;
	queue_submit_info.pWaitDstStageMask = wait_stages;
	queue_submit_info.commandBufferCount = 1;
	queue_submit_info.pCommandBuffers = &cmd_buffs.cmd_buff_tasks[image_index].cmd_buff;	
	queue_submit_info.signalSemaphoreCount = 1;
	queue_submit_info.pSignalSemaphores = &sync.render_ended;

	vk_res = vkQueueSubmit(dev.queue, 1, &queue_submit_info, sync.render_ended_fence);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, ExtraError::FAILED_TO_SUBMIT_QUEUE, code_location, "failed to submit draw commands to gpu queue");
	}

	// Present image
	VkSwapchainKHR swapChains[] = { swapchain.swapchain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &sync.render_ended;		
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;

	vk_res = vkQueuePresentKHR(dev.queue, &present_info);
	if (vk_res != VK_SUCCESS) {
		return ErrorStack(vk_res, ExtraError::FAILED_TO_PRESENT_IMAGE, code_location, "failed to present image");
	}
	return ErrorStack();
}

VulkanManagement::~VulkanManagement()
{
	if (dev.logical_device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(dev.logical_device);
	}
}
