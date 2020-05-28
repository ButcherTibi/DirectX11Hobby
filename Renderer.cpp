
// mine
#include "MathTypes.h"
#include "FileIO.h"

// Header
#include "Renderer.h"


// globals
VulkanRenderer renderer;


void calcBoxVerts(ui::BoxModel& box,
	std::array<GPU_Rects_Vertex, 18>& rects_verts,
	std::array<GPU_Circles_Vertex, 24>& circles_verts)
{
	auto createRoundedRectangle = [&rects_verts, &circles_verts](glm::vec3 origin, float width, float height,
		float tl_radius, float tr_radius, float br_radius, float bl_radius,
		size_t rect_idx, size_t circles_idx)
	{
		glm::vec2 vec2_origin{ origin.x, origin.y };

		glm::vec2 top_left_up = vec2_origin;
		glm::vec2 top_left_down = vec2_origin;

		glm::vec2 top_right_up = vec2_origin;
		glm::vec2 top_right_down = vec2_origin;

		glm::vec2 bot_right_up = vec2_origin;
		glm::vec2 bot_right_down = vec2_origin;

		glm::vec2 bot_left_up = vec2_origin;
		glm::vec2 bot_left_down = vec2_origin;

		// Box positions
		top_left_up.x += tl_radius;
		top_left_down.y += tl_radius;

		top_right_up.x += width - tr_radius;
		top_right_down.x += width;
		top_right_down.y += tr_radius;

		bot_right_up.x += width;
		bot_right_up.y += height - br_radius;
		bot_right_down.x += width - br_radius;
		bot_right_down.y += height;

		bot_left_up.y += height - bl_radius;
		bot_left_down.x += bl_radius;
		bot_left_down.y += height;

		// Box triangles
		rects_verts[rect_idx].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 1].pos = { top_right_up, origin.z };
		rects_verts[rect_idx + 2].pos = { top_right_down, origin.z };

		rects_verts[rect_idx + 3].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 4].pos = { top_right_down, origin.z };
		rects_verts[rect_idx + 5].pos = { bot_right_up, origin.z };
		
		rects_verts[rect_idx + 6].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 7].pos = { bot_right_up, origin.z };
		rects_verts[rect_idx + 8].pos = { bot_right_down, origin.z };
		
		rects_verts[rect_idx + 9].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 10].pos = { bot_right_down, origin.z };
		rects_verts[rect_idx + 11].pos = { bot_left_down, origin.z };
		
		rects_verts[rect_idx + 12].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 13].pos = { bot_left_down, origin.z };
		rects_verts[rect_idx + 14].pos = { bot_left_up, origin.z };
		
		rects_verts[rect_idx + 15].pos = { top_left_up, origin.z };
		rects_verts[rect_idx + 16].pos = { bot_left_up, origin.z };
		rects_verts[rect_idx + 17].pos = { top_left_down, origin.z };

		// Corner Circles
		auto createRectangle = [&circles_verts](glm::vec3 origin, float radius, size_t idx) {

			glm::vec2 vec2_origin{ origin.x, origin.y };
			
			glm::vec2 top_left = vec2_origin;
			glm::vec2 top_right = vec2_origin;
			glm::vec2 bot_right = vec2_origin;
			glm::vec2 bot_left = vec2_origin;
		
			float diameter = radius * 2;
			top_right.x += diameter;
			bot_right += diameter;
			bot_left.y += diameter;

			// Position
			circles_verts[idx].pos = { top_left, origin.z };
			circles_verts[idx + 1].pos = { top_right, origin.z };
			circles_verts[idx + 2].pos = { bot_right, origin.z };

			circles_verts[idx + 3].pos = { bot_left, origin.z };
			circles_verts[idx + 4].pos = { top_left, origin.z };
			circles_verts[idx + 5].pos = { bot_right, origin.z };

			// Center and Radius
			glm::vec2 center = vec2_origin;
			center += radius;

			for (auto i = idx; i < idx + 6; i++) {
				circles_verts[idx].center = center;
				circles_verts[idx].radius = radius;
			}
		};

		glm::vec3 tr_origin = origin;
		glm::vec3 br_origin = origin;
		glm::vec3 bl_origin = origin;

		// Top Left Circle
		createRectangle(origin, tl_radius, circles_idx);

		// Top Right Circle
		tr_origin.x += width - (tr_radius * 2);
		createRectangle(tr_origin, tr_radius, circles_idx + 6);

		// Bot Right Circle
		br_origin.x += width - (br_radius * 2);
		br_origin.y += height - (br_radius * 2);
		createRectangle(br_origin, br_radius, circles_idx + 12);

		// Bot Left Circle
		bl_origin.y += height - (bl_radius * 2);
		createRectangle(bl_origin, bl_radius, circles_idx + 18);
	};

	// Create Border Box
	
}


ErrStack VulkanRenderer::recreate(RenderingContent& content)
{
	if (instance.instance == VK_NULL_HANDLE) {
		checkErrStack1(instance.create());
	}

	if (surface.surface == VK_NULL_HANDLE) {
		checkErrStack1(surface.create(&instance, *content.hinstance, *content.hwnd));
	}

	if (phys_dev.physical_device == VK_NULL_HANDLE) {
		checkErrStack1(phys_dev.create(&instance, &surface));
	}

	if (logical_dev.logical_device == VK_NULL_HANDLE) {
		checkErrStack1(logical_dev.create(&instance, &phys_dev));
	}

	if (cmd_pool.cmd_pool == VK_NULL_HANDLE) {
		checkErrStack1(cmd_pool.create(&logical_dev, &phys_dev));
	}

	if (staging_buff.buff == VK_NULL_HANDLE) {
		checkErrStack1(staging_buff.recreateStaging(&logical_dev, 1));
		checkErrStack1(staging_buff.setDebugName("staging"));
	}

	if (swapchain.swapchain == VK_NULL_HANDLE) {
		checkErrStack1(swapchain.create(&surface, &phys_dev, &logical_dev,
			content.width, content.height));
		checkErrStack1(swapchain.setDebugName("swapchain"));
	}

	if (rects_color_img.img == VK_NULL_HANDLE) {

		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = swapchain.surface_format.format;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack(rects_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY), "");
		checkErrStack(rects_color_img.setDebugName("rects color"), "");
	}

	if (rects_depth_img.img == VK_NULL_HANDLE) {

		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = VK_FORMAT_D32_SFLOAT;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack(rects_depth_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY), "");
		checkErrStack(rects_depth_img.setDebugName("rects depth"), "");
	}

	if (circles_color_img.img == VK_NULL_HANDLE) {

		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = swapchain.surface_format.format;;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack(circles_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY), "");
		checkErrStack(circles_color_img.setDebugName("circles color"), "");
	}

	if (circles_depth_img.img == VK_NULL_HANDLE) {

		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = VK_FORMAT_D32_SFLOAT;
		info.extent.width = swapchain.resolution.width;
		info.extent.height = swapchain.resolution.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack(circles_depth_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY), "");
		checkErrStack(circles_depth_img.setDebugName("rects depth"), "");
	}

	if (renderpass.renderpass == VK_NULL_HANDLE) {

		VkAttachmentDescription color_atach = {};
		color_atach.format = swapchain.surface_format.format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_atach = {};
		depth_atach.format = VK_FORMAT_D32_SFLOAT;
		depth_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_atach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Rects pass
		VkAttachmentReference rects_color_atach_ref = {};
		rects_color_atach_ref.attachment = 0;
		rects_color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rects_color_read_atach_ref = {};
		rects_color_read_atach_ref.attachment = 0;
		rects_color_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference rects_depth_atach_ref = {};
		rects_depth_atach_ref.attachment = 1;
		rects_depth_atach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference rects_depth_read_atach_ref = {};
		rects_depth_read_atach_ref.attachment = 1;
		rects_depth_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkSubpassDescription rects_subpass = {};
		rects_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		rects_subpass.colorAttachmentCount = 1;
		rects_subpass.pColorAttachments = &rects_color_atach_ref;
		rects_subpass.pDepthStencilAttachment = &rects_depth_atach_ref;

		// Circles pass
		VkAttachmentReference circles_color_atach_ref = {};
		circles_color_atach_ref.attachment = 2;
		circles_color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference circles_color_read_atach_ref = {};
		circles_color_read_atach_ref.attachment = 2;
		circles_color_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference circles_depth_atach_ref = {};
		circles_depth_atach_ref.attachment = 3;
		circles_depth_atach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference circles_depth_read_atach_ref = {};
		circles_depth_read_atach_ref.attachment = 3;
		circles_depth_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkSubpassDescription circles_subpass = {};
		circles_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		circles_subpass.colorAttachmentCount = 1;
		circles_subpass.pColorAttachments = &circles_color_atach_ref;
		circles_subpass.pDepthStencilAttachment = &circles_depth_atach_ref;

		// Composite pass
		VkAttachmentDescription compose_color_atach = color_atach;
		compose_color_atach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference compose_color_atach_ref = {};
		compose_color_atach_ref.attachment = 4;
		compose_color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, 4> input_atachments = {
			rects_color_read_atach_ref, rects_depth_read_atach_ref,
			circles_color_read_atach_ref, circles_depth_read_atach_ref
		};

		VkSubpassDescription compose_subpass = {};
		compose_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		compose_subpass.inputAttachmentCount = (uint32_t)input_atachments.size();
		compose_subpass.pInputAttachments = input_atachments.data();
		compose_subpass.colorAttachmentCount = 1;
		compose_subpass.pColorAttachments = &compose_color_atach_ref;

		// Dependencies
		VkSubpassDependency rects_depend = {};
		rects_depend.srcSubpass = 0;
		rects_depend.dstSubpass = 2;
		rects_depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		rects_depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		rects_depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		rects_depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubpassDependency circles_depend = {};
		circles_depend.srcSubpass = 0;
		circles_depend.dstSubpass = 2;
		circles_depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		circles_depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		circles_depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		circles_depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubpassDependency compose_depend = {};
		compose_depend.srcSubpass = 1;
		compose_depend.dstSubpass = 2;
		compose_depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		compose_depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		compose_depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		compose_depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// Attachments
		std::array<VkAttachmentDescription, 5> attachments = {
			color_atach, depth_atach, color_atach, depth_atach,
			compose_color_atach
		};

		std::array<VkSubpassDescription, 3> subpass_descps = {
			rects_subpass, circles_subpass, compose_subpass
		};

		std::array<VkSubpassDependency, 3> depends = {
			rects_depend, circles_depend, compose_depend
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = (uint32_t)subpass_descps.size();
		info.pSubpasses = subpass_descps.data();
		info.dependencyCount = (uint32_t)depends.size();
		info.pDependencies = depends.data();

		checkErrStack(renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
	}

	if (rects_color_view.view == VK_NULL_HANDLE) {

		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = rects_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = rects_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack(rects_color_view.create(&logical_dev, &info), "");
	}

	if (rects_depth_view.view == VK_NULL_HANDLE) {

		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = rects_depth_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = rects_depth_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack(rects_depth_view.create(&logical_dev, &info), "");
	}

	if (circles_color_view.view == VK_NULL_HANDLE) {

		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = circles_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = circles_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack(circles_color_view.create(&logical_dev, &info), "");
	}

	if (circles_depth_view.view == VK_NULL_HANDLE) {

		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = circles_depth_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = circles_depth_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack(circles_depth_view.create(&logical_dev, &info), "");
	}

	if (!frame_buffs.frame_buffs.size()) {

		std::array<VkImageView, 4> attachments = {
			rects_color_view.view, rects_depth_view.view,
			circles_color_view.view, circles_depth_view.view
		};

		checkErrStack(frame_buffs.create(&logical_dev, &renderpass,
			attachments, swapchain.views,
			swapchain.resolution.width, swapchain.resolution.height),
			"");
	}

	// Subpass Commons
	if (uniform_buff.buff == VK_NULL_HANDLE) {

		GPU_Uniform uniform;
		uniform.screen_width = (float)swapchain.resolution.width;
		uniform.screen_height = (float)swapchain.resolution.height;

		checkErrStack1(uniform_buff.create(&logical_dev, sizeof(GPU_Uniform),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));

		checkErrStack1(uniform_buff.load(&uniform, sizeof(GPU_Uniform),
			&cmd_pool, &staging_buff));
	}

	if (uniform_descp_layout.descp_layout == VK_NULL_HANDLE) {

		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(uniform_descp_layout.create(&logical_dev, bindings));
	}

	if (uniform_descp_pool.descp_pool == VK_NULL_HANDLE) {

		std::vector<VkDescriptorPoolSize> sizes(1);
		sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		sizes[0].descriptorCount = 1;

		checkErrStack1(uniform_descp_pool.create(&logical_dev, sizes));
	}

	if (uniform_descp_set.descp_set == VK_NULL_HANDLE) {
		
		checkErrStack1(uniform_descp_set.create(&logical_dev, &uniform_descp_pool, &uniform_descp_layout));

		std::vector<VkWriteDescriptorSet> writes(1);
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = uniform_descp_set.descp_set;
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorBufferInfo uniform_buff_info = {};
		uniform_buff_info.buffer = uniform_buff.buff;
		uniform_buff_info.range = sizeof(GPU_Uniform);
		writes[0].pBufferInfo = &uniform_buff_info;

		uniform_descp_set.update(writes);
	}

	// Create Geometry
	{
		ui::BoxModel box = {};
		box.background_color = { 1, 0, 0, 1 };

		box.paddingbox_width = 100;
		box.paddingbox_height = 100;
		box.padding_tl_radius = 25;
		box.padding_tr_radius = 25;
		box.padding_br_radius = 25;
		box.padding_bl_radius = 25;

		box.borderbox_width = 100;
		box.borderbox_height = 110;
		box.border_top_thick = 10;

		std::array<GPU_Rects_Vertex, 18> rects_verts;
		std::array<GPU_Circles_Vertex, 24> circles_verts;
		calcBoxVerts(box, rects_verts, circles_verts);

		checkErrStack1(rects_vertex_buff.create(&logical_dev, rects_verts.size() * sizeof(GPU_Rects_Vertex),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY));

		checkErrStack1(rects_vertex_buff.load(rects_verts.data(), rects_verts.size() * sizeof(GPU_Rects_Vertex),
			&cmd_pool, &staging_buff));

		rects_vertex_count = rects_verts.size();

		checkErrStack1(circles_vertex_buff.create(&logical_dev, circles_verts.size() * sizeof(GPU_Circles_Vertex),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY));

		checkErrStack1(circles_vertex_buff.load(circles_verts.data(), circles_verts.size() * sizeof(GPU_Circles_Vertex),
			&cmd_pool, &staging_buff));

		circles_vertex_count = circles_verts.size();
	}

	if (rects_vert_module.sh_module == VK_NULL_HANDLE) {
		
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rects_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	if (rects_frag_module.sh_module == VK_NULL_HANDLE) {

		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rects_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	if (rects_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &uniform_descp_layout.descp_layout;

		checkErrStack(rects_pipe_layout.create(&logical_dev, &info), "");
	}

	if (rects_pipe.pipeline == VK_NULL_HANDLE) {

		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = rects_vert_module.stage;
		shader_stages[0].module = rects_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = rects_frag_module.stage;
		shader_stages[1].module = rects_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input Description
		VkVertexInputBindingDescription vert_input_binding_descp = {};
		vert_input_binding_descp = GPU_Rects_Vertex::getBindingDescription();

		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descps;
		for (auto& atribute_descp : GPU_Rects_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descps.push_back(atribute_descp);
		}

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vert_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descps.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descps.data();

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments = {
			color_blend_attachment
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;
		
		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = NULL;
		pipeline_info.layout = rects_pipe_layout.pipe_layout;
		pipeline_info.renderPass = renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(rects_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(rects_pipe.setDebugName("rect pipeline"));
	}

	// Circles Subpass
	if (circles_vert_module.sh_module == VK_NULL_HANDLE) {

		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	if (circles_frag_module.sh_module == VK_NULL_HANDLE) {

		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	if (circles_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &uniform_descp_layout.descp_layout;

		checkErrStack(circles_pipe_layout.create(&logical_dev, &info), "");
	}

	if (circles_pipe.pipeline == VK_NULL_HANDLE) {

		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = circles_vert_module.stage;
		shader_stages[0].module = circles_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = circles_frag_module.stage;
		shader_stages[1].module = circles_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input Description
		VkVertexInputBindingDescription vert_input_binding_descp = {};
		vert_input_binding_descp = GPU_Circles_Vertex::getBindingDescription();

		std::vector<VkVertexInputAttributeDescription> vertex_input_atribute_descps;
		for (auto& atribute_descp : GPU_Circles_Vertex::getAttributeDescriptions()) {
			vertex_input_atribute_descps.push_back(atribute_descp);
		}

		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 1;
		vert_input_stage_info.pVertexBindingDescriptions = &vert_input_binding_descp;
		vert_input_stage_info.vertexAttributeDescriptionCount = (uint32_t)vertex_input_atribute_descps.size();
		vert_input_stage_info.pVertexAttributeDescriptions = vertex_input_atribute_descps.data();

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments = {
			color_blend_attachment
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = NULL;
		pipeline_info.layout = circles_pipe_layout.pipe_layout;
		pipeline_info.renderPass = renderpass.renderpass;
		pipeline_info.subpass = 1;

		checkErrStack(circles_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(circles_pipe.setDebugName("circles"));
	}

	// Compose Subpass
	if (compose_vert_module.sh_module == VK_NULL_HANDLE) {

		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	if (compose_frag_module.sh_module == VK_NULL_HANDLE) {

		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	if (compose_descp_layout.descp_layout == VK_NULL_HANDLE) {

		std::vector<VkDescriptorSetLayoutBinding> bindings(4);
		// Rects Color Image
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Rects Depth Image
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Circles Color Image
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[2].descriptorCount = 1;
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Circles Depth Image
		bindings[3].binding = 3;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[3].descriptorCount = 1;
		bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(compose_descp_layout.create(&logical_dev, bindings));
		checkErrStack1(compose_descp_layout.setDebugName("compose"));
	}

	if (compose_descp_pool.descp_pool == VK_NULL_HANDLE) {

		std::vector<VkDescriptorPoolSize> sizes(4);
		sizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[0].descriptorCount = 1;

		sizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[1].descriptorCount = 1;

		sizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[2].descriptorCount = 1;

		sizes[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[3].descriptorCount = 1;

		checkErrStack1(compose_descp_pool.create(&logical_dev, sizes));
		checkErrStack1(compose_descp_pool.setDebugName("compose"));
	}

	if (compose_descp_set.descp_set == VK_NULL_HANDLE) {

		checkErrStack1(compose_descp_set.create(&logical_dev, &compose_descp_pool, &compose_descp_layout));
		checkErrStack1(compose_descp_set.setDebugName("compose"));

		std::vector<VkWriteDescriptorSet> writes(4);
		// Rects Color Image
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = compose_descp_set.descp_set;
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo rects_color = {};
		rects_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rects_color.imageView = rects_color_view.view;
		writes[0].pImageInfo = &rects_color;
		
		// Rects Depth Image
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = compose_descp_set.descp_set;
		writes[1].dstBinding = 1;
		writes[1].dstArrayElement = 0;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo rects_depth = {};
		rects_depth.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rects_depth.imageView = rects_depth_view.view;
		writes[1].pImageInfo = &rects_depth;

		// Circles Color Image
		writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[2].dstSet = compose_descp_set.descp_set;
		writes[2].dstBinding = 2;
		writes[2].dstArrayElement = 0;
		writes[2].descriptorCount = 1;
		writes[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo circles_color = {};
		circles_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		circles_color.imageView = circles_color_view.view;
		writes[2].pImageInfo = &circles_color;

		// Circles Depth Image
		writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[3].dstSet = compose_descp_set.descp_set;
		writes[3].dstBinding = 3;
		writes[3].dstArrayElement = 0;
		writes[3].descriptorCount = 1;
		writes[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo circles_depth = {};
		circles_depth.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		circles_depth.imageView = circles_depth_view.view;
		writes[3].pImageInfo = &circles_depth;

		compose_descp_set.update(writes);
	}

	if (compose_pipe_layout.pipe_layout == VK_NULL_HANDLE) {

		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &compose_descp_layout.descp_layout;

		checkErrStack1(compose_pipe_layout.create(&logical_dev, &info));
	}

	if (compose_pipe.pipeline == VK_NULL_HANDLE) {

		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = compose_vert_module.stage;
		shader_stages[0].module = compose_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = compose_frag_module.stage;
		shader_stages[1].module = compose_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input Description
		VkPipelineVertexInputStateCreateInfo vert_input_stage_info = {};
		vert_input_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_stage_info.vertexBindingDescriptionCount = 0;
		vert_input_stage_info.pVertexBindingDescriptions = NULL;
		vert_input_stage_info.vertexAttributeDescriptionCount = 0;
		vert_input_stage_info.pVertexAttributeDescriptions = NULL;

		// Input Asembly State
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
		input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

		// Viewport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.resolution.width;
		viewport.height = (float)swapchain.resolution.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = swapchain.resolution.width;
		scissor.extent.height = swapchain.resolution.height;

		VkPipelineViewportStateCreateInfo viewport_state_info = {};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo raster_state_info = {};
		raster_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state_info.depthClampEnable = VK_FALSE;
		raster_state_info.rasterizerDiscardEnable = VK_FALSE;
		raster_state_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state_info.cullMode = VK_CULL_MODE_NONE;
		raster_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state_info.depthBiasEnable = VK_FALSE;
		raster_state_info.lineWidth = 1.0f;

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.sampleShadingEnable = VK_FALSE;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
		depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_info.depthTestEnable = VK_FALSE;
		depth_stencil_state_info.depthWriteEnable = VK_FALSE;
		depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_info.stencilTestEnable = VK_FALSE;

		// Color Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments = {
			color_blend_attachment
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
		color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_info.logicOpEnable = VK_FALSE;
		color_blend_state_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_info.attachmentCount = (uint32_t)color_blend_attachments.size();
		color_blend_state_info.pAttachments = color_blend_attachments.data();
		color_blend_state_info.blendConstants[0] = 0.0f;
		color_blend_state_info.blendConstants[1] = 0.0f;
		color_blend_state_info.blendConstants[2] = 0.0f;
		color_blend_state_info.blendConstants[3] = 0.0f;

		// Pipeline 
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vert_input_stage_info;
		pipeline_info.pInputAssemblyState = &input_assembly_state_info;
		pipeline_info.pTessellationState = NULL;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &raster_state_info;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = &depth_stencil_state_info;
		pipeline_info.pColorBlendState = &color_blend_state_info;
		pipeline_info.pDynamicState = NULL;
		pipeline_info.layout = compose_pipe_layout.pipe_layout;
		pipeline_info.renderPass = renderpass.renderpass;
		pipeline_info.subpass = 2;

		checkErrStack(compose_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(compose_pipe.setDebugName("compose"));
	}

	// Command Buffers
	checkErrStack1(render_cmd_buffs.recreate(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()));

	// Command Buffer Update
	{
		vks::RenderingCmdBuffsUpdateInfo info = {};
		info.renderpass = &renderpass;
		info.frame_buffs = &frame_buffs;
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;

		// Common
		info.uniform_buff = &uniform_buff;
		info.uniform_descp_set = &uniform_descp_set;

		// Rects
		info.rects_vertex_buff = &rects_vertex_buff;
		info.rects_vertex_count = rects_vertex_count;
		info.rects_pipe_layout = &rects_pipe_layout;
		info.rects_pipe = &rects_pipe;

		// Circles
		info.circles_vertex_buff = &circles_vertex_buff;
		info.circles_vertex_count = circles_vertex_count;
		info.circles_pipe_layout = &circles_pipe_layout;
		info.circles_pipe = &circles_pipe;

		// Compose
		info.compose_descp_set = &compose_descp_set;
		info.compose_pipe_layout = &compose_pipe_layout;
		info.compose_pipe = &compose_pipe;

		checkErrStack(render_cmd_buffs.update(info), "failed to update rendering commands");
	}

	// Syncronization
	if (img_acquired.semaphore == VK_NULL_HANDLE) {
		checkErrStack(img_acquired.recreate(&logical_dev),
			"failed to create semaphore for acquiring image from swapchain");
	}

	if (rendering_ended_sem.semaphore == VK_NULL_HANDLE) {
		checkErrStack(rendering_ended_sem.recreate(&logical_dev),
			"failed to create semaphore for rendering ended");
	}

	return ErrStack();
}

ErrStack VulkanRenderer::draw()
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

ErrStack VulkanRenderer::waitForRendering()
{
	checkVkRes(vkDeviceWaitIdle(logical_dev.logical_device), "");
	return ErrStack();
}
