
// mine
#include "MathTypes.h"
#include "FileIO.h"
#include "UIComponents.h"  // has to go

// Header
#include "Renderer.h"


// globals
VulkanRenderer renderer;


struct BoxVerts {
	// Renderpass Types
	// rect renderpass
	// circles renderpass

	// Border Renderpass
	std::array<GPU_Rects_Vertex, 18> border_rect;
	// Border Circles Renderpass
	std::array<GPU_Circles_Vertex, 6> border_tl_circle;  // Draw Call
	std::array<GPU_Circles_Vertex, 6> border_tr_circle;  // Draw Call
	std::array<GPU_Circles_Vertex, 6> border_br_circle;  // Draw Call
	std::array<GPU_Circles_Vertex, 6> border_bl_circle;  // Draw Call

	// perform fancy coloring eg linear grading borders or corner falloff

	// Padding Renderpass
	std::array<GPU_Rects_Vertex, 18> padding_rect;
	// Padding Circles Renderpass
	std::array<GPU_Circles_Vertex, 6> padding_tl_circle;
	std::array<GPU_Circles_Vertex, 6> padding_tr_circle;
	std::array<GPU_Circles_Vertex, 6> padding_br_circle;
	std::array<GPU_Circles_Vertex, 6> padding_bl_circle;

	// perform fancy coloring eg linear grading padding or padding corner falloff

	// Composition Renderpass
	// -> border color image
	// -> border depth image
	// -> padding color image
	// -> padding depth image
	// -> previous layer color image (as blending target)
	// --------------------------------------------------
	// => next layer color image
};

void calcBoxVerts(ui::BoxModel& box, BoxVerts& r_verts)
{
	auto createChamferedRectangle = [&](glm::vec2 origin, float width, float height,
		float tl_radius, float tr_radius, float br_radius, float bl_radius,
		std::array<GPU_Rects_Vertex, 18>& rect_verts)
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
		rect_verts[0].pos = top_left_up;
		rect_verts[1].pos = top_right_up;
		rect_verts[2].pos = top_right_down;

		rect_verts[3].pos = top_left_up;
		rect_verts[4].pos = top_right_down;
		rect_verts[5].pos = bot_right_up;

		rect_verts[6].pos = top_left_up;
		rect_verts[7].pos = bot_right_up;
		rect_verts[8].pos = bot_right_down;

		rect_verts[9].pos = top_left_up;
		rect_verts[10].pos = bot_right_down;
		rect_verts[11].pos = bot_left_down;

		rect_verts[12].pos = top_left_up;
		rect_verts[13].pos = bot_left_down;
		rect_verts[14].pos = bot_left_up;

		rect_verts[15].pos = top_left_up;
		rect_verts[16].pos = bot_left_up;
		rect_verts[17].pos = top_left_down;
	};

	auto createCircle = [](glm::vec2 origin, float radius,
		std::array<GPU_Circles_Vertex, 6>& verts)
	{
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
		verts[0].pos = top_left;
		verts[1].pos = top_right;
		verts[2].pos = bot_right;

		verts[3].pos = bot_left;
		verts[4].pos = top_left;
		verts[5].pos = bot_right;

		// Center and Radius
		glm::vec2 center = vec2_origin;
		center += radius;

		for (auto& vert : verts) {
			vert.center = center;
			vert.radius = radius;
		}
	};

	// Border Box
	glm::vec2 border_origin = { box.origin };
	{
		createChamferedRectangle(border_origin, box.borderbox_width, box.borderbox_height,
			box.border_tl_radius, box.border_tr_radius, box.border_br_radius, box.border_bl_radius,
			r_verts.border_rect);

		for (GPU_Rects_Vertex& vert : r_verts.border_rect) {
			vert.color = box.border_color;
		}
	}

	// Create Border Circles
	{
		glm::vec2 tr_origin = border_origin;
		glm::vec2 br_origin = border_origin;
		glm::vec2 bl_origin = border_origin;

		tr_origin.x += box.borderbox_width - (box.border_tr_radius * 2);
		br_origin.x += box.borderbox_width - (box.border_br_radius * 2);
		br_origin.y += box.borderbox_height - (box.border_br_radius * 2);
		bl_origin.y += box.borderbox_height - (box.border_bl_radius * 2);

		createCircle(border_origin, box.border_tl_radius, r_verts.border_tl_circle);
		createCircle(tr_origin, box.border_tr_radius, r_verts.border_tr_circle);
		createCircle(br_origin, box.border_br_radius, r_verts.border_br_circle);
		createCircle(bl_origin, box.border_bl_radius, r_verts.border_bl_circle);

		for (auto& vert : r_verts.border_tl_circle) {
			vert.color = box.border_color;
		}
		for (auto& vert : r_verts.border_tr_circle) {
			vert.color = box.border_color;
		}
		for (auto& vert : r_verts.border_br_circle) {
			vert.color = box.border_color;
		}
		for (auto& vert : r_verts.border_bl_circle) {
			vert.color = box.border_color;
		}
	}
	
	// Create Padding Box
	glm::vec2 padding_origin = border_origin;
	padding_origin.x += box.border_left_thick;
	padding_origin.y += box.border_top_thick;
	{
		createChamferedRectangle(padding_origin, box.paddingbox_width, box.paddingbox_height,
			box.padding_tl_radius, box.padding_tr_radius, box.padding_br_radius, box.padding_bl_radius,
			r_verts.padding_rect);

		for (auto& vert : r_verts.padding_rect) {
			vert.color = box.background_color;
		}
	}

	// Create Padding Circles
	{
		glm::vec2 tr_origin = padding_origin;
		glm::vec2 br_origin = padding_origin;
		glm::vec2 bl_origin = padding_origin;

		tr_origin.x += box.paddingbox_width - (box.padding_tr_radius * 2);
		br_origin.x += box.paddingbox_width - (box.padding_br_radius * 2);
		br_origin.y += box.paddingbox_height - (box.padding_br_radius * 2);
		bl_origin.y += box.paddingbox_height - (box.padding_bl_radius * 2);

		createCircle(padding_origin, box.padding_tl_radius, r_verts.padding_tl_circle);
		createCircle(tr_origin, box.padding_tr_radius, r_verts.padding_tr_circle);
		createCircle(br_origin, box.padding_br_radius, r_verts.padding_br_circle);
		createCircle(bl_origin, box.padding_bl_radius, r_verts.padding_bl_circle);

		for (auto& vert : r_verts.padding_tl_circle) {
			vert.color = box.background_color;
		}
		for (auto& vert : r_verts.padding_tr_circle) {
			vert.color = box.background_color;
		}
		for (auto& vert : r_verts.padding_br_circle) {
			vert.color = box.background_color;
		}
		for (auto& vert : r_verts.padding_bl_circle) {
			vert.color = box.background_color;
		}
	}
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

	if (swapchain.swapchain == VK_NULL_HANDLE) {
		checkErrStack1(swapchain.create(&surface, &phys_dev, &logical_dev,
			content.width, content.height));
		checkErrStack1(swapchain.setDebugName("swapchain"));
	}

	// Command Pool
	{
		checkErrStack1(cmd_pool.create(&logical_dev, &phys_dev));
	}
	
	// Border Color Image
	{
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

		checkErrStack1(border_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(border_color_img.setDebugName("border color"));
	}

	// Padding Color Image
	{
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

		checkErrStack1(padding_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(padding_color_img.setDebugName("padding color"));
	}

	// Compose Color Image
	{
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
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		checkErrStack1(compose_color_img.recreate(&logical_dev, &info, VMA_MEMORY_USAGE_GPU_ONLY));
		checkErrStack1(compose_color_img.setDebugName("compose color"));
	}

	// Border Color View
	{
		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = border_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = border_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(border_color_view.create(&logical_dev, &info));
	}

	// Padding Color View
	{
		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = padding_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = padding_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(padding_color_view.create(&logical_dev, &info));
	}

	// Compose Color View
	{
		VkComponentMapping component_mapping = {};
		VkImageSubresourceRange sub_range = {};
		sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_range.levelCount = 1;
		sub_range.layerCount = 1;

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = compose_color_img.img;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = compose_color_img.format;
		info.components = component_mapping;
		info.subresourceRange = sub_range;

		checkErrStack1(compose_color_view.create(&logical_dev, &info));
	}

	// Uniform Buffer
	{
		uniform_staging_buff.logical_dev = &logical_dev;

		uniform_buff.create(&logical_dev, &cmd_pool, &uniform_staging_buff,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		GPU_Uniform uniform;
		uniform.screen_width = (float)swapchain.resolution.width;
		uniform.screen_height = (float)swapchain.resolution.height;

		checkErrStack1(uniform_buff.push(&uniform, sizeof(GPU_Uniform)));
		checkErrStack1(uniform_buff.push(&uniform, sizeof(GPU_Uniform)));
		checkErrStack1(uniform_buff.flush());
	}

	// Uniform Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(uniform_descp_layout.create(&logical_dev, bindings));
	}

	// Uniform Descriptor Pool
	{
		std::vector<VkDescriptorPoolSize> sizes(1);
		sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		sizes[0].descriptorCount = 1;

		checkErrStack1(uniform_descp_pool.create(&logical_dev, sizes));
	}

	// Uniform Descriptor Set
	{
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

	// Geometry Buffers
	{
		border_rect_staging_buff.logical_dev = &logical_dev;
		border_circles_staging_buff.logical_dev = &logical_dev;
		padding_rect_staging_buff.logical_dev = &logical_dev;
		padding_circles_staging_buff.logical_dev = &logical_dev;

		border_rect_vertex_buff.create(&logical_dev, &cmd_pool, &border_rect_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		border_circles_vertex_buff.create(&logical_dev, &cmd_pool, &border_circles_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		padding_rect_vertex_buff.create(&logical_dev, &cmd_pool, &padding_rect_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		padding_circles_vertex_buff.create(&logical_dev, &cmd_pool, &padding_circles_staging_buff,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		// Generate GPU Data
		ui::UserInterface user_interface = {};
		user_interface.recreateGraph((float)swapchain.resolution.width, (float)swapchain.resolution.height);

		/*ui::BasicElement basic_elem_0 = {};
		basic_elem_0.width.setAbsolute(500);
		basic_elem_0.height.setAbsolute(500);

		basic_elem_0.background_color = { 1, 0, 0, 1 };
		basic_elem_0.padding_tr_radius = 150;
		basic_elem_0.padding_br_radius = 150;

		basic_elem_0.border_top.setAbsolute(2);
		basic_elem_0.border_right.setAbsolute(2);
		basic_elem_0.border_bot.setAbsolute(2);
		basic_elem_0.border_left.setAbsolute(2);

		basic_elem_0.border_color = { 0, 1, 0, 1 };
		basic_elem_0.border_tr_radius = 150;
		basic_elem_0.border_br_radius = 150;

		user_interface.addElement(&user_interface.elems.front(), basic_elem_0);*/
		user_interface.calcGraphLayout();

		checkErrStack1(this->calc(user_interface));

		//border_rect_vertex_count = 0;
		//border_circles_vertex_count = 0;
		//padding_rect_vertex_count = 0;
		//padding_circles_vertex_count = 0;

		//for (auto it = ++user_interface.elems.begin(); it != user_interface.elems.end(); ++it) {

		//	ui::Element& elem = *it;
		//	ui::BasicElement* basic_elem = std::get_if<ui::BasicElement>(&elem.elem);

		//	BoxVerts verts;
		//	calcBoxVerts(*basic_elem, verts);

		//	border_rect_vertex_count += verts.border_rect.size();
		//	border_circles_vertex_count += verts.border_tl_circle.size();
		//	border_circles_vertex_count += verts.border_tr_circle.size();
		//	border_circles_vertex_count += verts.border_br_circle.size();
		//	border_circles_vertex_count += verts.border_bl_circle.size();

		//	padding_rect_vertex_count += verts.padding_rect.size();
		//	padding_circles_vertex_count += verts.padding_tl_circle.size();
		//	padding_circles_vertex_count += verts.padding_tr_circle.size();
		//	padding_circles_vertex_count += verts.padding_br_circle.size();
		//	padding_circles_vertex_count += verts.padding_bl_circle.size();

		//	// Border
		//	checkErrStack1(border_rect_vertex_buff.push(verts.border_rect.data(),
		//		verts.border_rect.size() * sizeof(GPU_Rects_Vertex)));

		//	checkErrStack1(border_circles_vertex_buff.push(verts.border_tl_circle.data(),
		//		verts.border_tl_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(border_circles_vertex_buff.push(verts.border_tr_circle.data(),
		//		verts.border_tr_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(border_circles_vertex_buff.push(verts.border_br_circle.data(),
		//		verts.border_br_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(border_circles_vertex_buff.push(verts.border_bl_circle.data(),
		//		verts.border_bl_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	// Padding
		//	checkErrStack1(padding_rect_vertex_buff.push(verts.padding_rect.data(),
		//		verts.padding_rect.size() * sizeof(GPU_Rects_Vertex)));

		//	checkErrStack1(padding_circles_vertex_buff.push(verts.padding_tl_circle.data(),
		//		verts.padding_tl_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(padding_circles_vertex_buff.push(verts.padding_tr_circle.data(),
		//		verts.padding_tr_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(padding_circles_vertex_buff.push(verts.padding_br_circle.data(),
		//		verts.padding_br_circle.size() * sizeof(GPU_Circles_Vertex)));

		//	checkErrStack1(padding_circles_vertex_buff.push(verts.padding_bl_circle.data(),
		//		verts.padding_bl_circle.size() * sizeof(GPU_Circles_Vertex)));
		//}

		//checkErrStack1(border_rect_vertex_buff.flush());
		//checkErrStack1(border_circles_vertex_buff.flush());
		//checkErrStack1(padding_rect_vertex_buff.flush());
		//checkErrStack1(padding_circles_vertex_buff.flush());
	}

	// Rect Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rect_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Rect Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Rects/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(rect_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Circles Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Circles Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Circles/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(circles_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Border Rect Renderpass
	{
		VkAttachmentDescription color_atach = {};
		color_atach.format = swapchain.surface_format.format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_atach_ref = {};
		color_atach_ref.attachment = 0;
		color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_atach_ref;

		std::array<VkAttachmentDescription, 1> attachments = {
			color_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(rect_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(rect_renderpass.setDebugName("rects renderpass"));
	}

	// Border Rect Framebuffers
	{
		border_rect_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			border_color_view.view
		};

		for (auto i = 0; i < border_rect_frames.size(); i++) {

			checkErrStack1(border_rect_frames[i].create(&logical_dev, &rect_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(border_rect_frames[i].setDebugName("border rect"));
		}
	}

	// Rect Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &uniform_descp_layout.descp_layout;

		checkErrStack(rect_pipe_layout.create(&logical_dev, &info), "");
	}

	// Border Rect Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = rect_vert_module.stage;
		shader_stages[0].module = rect_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = rect_frag_module.stage;
		shader_stages[1].module = rect_frag_module.sh_module;
		shader_stages[1].pName = "main";

		// Vertex Input State
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
		pipeline_info.layout = rect_pipe_layout.pipe_layout;
		pipeline_info.renderPass = rect_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(rect_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(rect_pipe.setDebugName("border rect"));
	}

	// Border Circle Renderpass
	{
		VkAttachmentDescription color_atach = {};
		color_atach.format = swapchain.surface_format.format;
		color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_atach_ref = {};
		color_atach_ref.attachment = 0;
		color_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_atach_ref;

		std::array<VkAttachmentDescription, 1> attachments = {
			color_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(circles_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(circles_renderpass.setDebugName("circles renderpass"));
	}

	// Border Circles Framebuffers
	{
		border_circles_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			border_color_view.view
		};

		for (auto i = 0; i < border_circles_frames.size(); i++) {

			checkErrStack1(border_circles_frames[i].create(&logical_dev, &circles_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(border_circles_frames[i].setDebugName("border circles"));
		}
	}

	// Circles Descriptor Layout

	// Circles Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &uniform_descp_layout.descp_layout;

		checkErrStack(circles_pipe_layout.create(&logical_dev, &info), "");
	}

	// Border Circles Pipeline
	{
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

		// Vertex Input State
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
		pipeline_info.renderPass = circles_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(circles_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(circles_pipe.setDebugName("border circles"));
	}

	// Padding Rect Framebuffers
	{
		padding_rect_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			padding_color_view.view
		};

		for (auto& frame : padding_rect_frames) {

			checkErrStack1(frame.create(&logical_dev, &rect_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("padding rect"));
		}
	}

	// Padding Circles Framebuffers
	{
		padding_circles_frames.resize(swapchain.images.size());
		std::vector<VkImageView> attachments = {
			padding_color_view.view
		};

		for (auto& frame : padding_circles_frames) {

			checkErrStack1(frame.create(&logical_dev, &circles_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("padding circles"));
		}
	}

	// Compose Renderpass
	{
		VkAttachmentDescription border_color_atach = {};
		border_color_atach.format = swapchain.surface_format.format;
		border_color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		border_color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		border_color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		border_color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		border_color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		border_color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		border_color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference border_color_read_atach_ref = {};
		border_color_read_atach_ref.attachment = 0;
		border_color_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription padding_color_atach = {};
		padding_color_atach.format = swapchain.surface_format.format;
		padding_color_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		padding_color_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		padding_color_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		padding_color_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		padding_color_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		padding_color_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		padding_color_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference padding_color_read_atach_ref = {};
		padding_color_read_atach_ref.attachment = 1;
		padding_color_read_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkAttachmentReference, 2> input_attach_refs = {
			border_color_read_atach_ref, padding_color_read_atach_ref
		};

		VkAttachmentDescription compose_atach = {};
		compose_atach.format = swapchain.surface_format.format;
		compose_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		compose_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		compose_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		compose_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		compose_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		compose_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		compose_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference compose_atach_ref = {};
		compose_atach_ref.attachment = 2;
		compose_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = input_attach_refs.size();
		subpass.pInputAttachments = input_attach_refs.data();
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &compose_atach_ref;

		std::array<VkAttachmentDescription, 3> attachments = {
			border_color_atach, padding_color_atach, compose_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(compose_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
	}

	// Compose Framebuffers
	{
		compose_frames.resize(swapchain.images.size());	

		for (auto i = 0; i < compose_frames.size(); i++) {

			vks::Framebuffer& frame = compose_frames[i];

			std::vector<VkImageView> attachments = {
				border_color_view.view, padding_color_view.view, compose_color_view.view,
			};

			checkErrStack1(frame.create(&logical_dev, &compose_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("compose"));
		}
	}

	// Compose Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(2);
		// Border Color Image
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Padding Color Image
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(compose_descp_layout.create(&logical_dev, bindings));
		checkErrStack1(compose_descp_layout.setDebugName("compose"));
	}

	// Uniform Descriptor Pool
	{
		std::vector<VkDescriptorPoolSize> sizes(2);
		sizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[0].descriptorCount = 1;

		sizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[1].descriptorCount = 1;

		checkErrStack1(compose_descp_pool.create(&logical_dev, sizes));
		checkErrStack1(compose_descp_pool.setDebugName("compose"));
	}

	// Uniform Descriptor Set
	{
		checkErrStack1(compose_descp_set.create(&logical_dev, &compose_descp_pool, &compose_descp_layout));
		checkErrStack1(compose_descp_set.setDebugName("compose"));

		std::vector<VkWriteDescriptorSet> writes(2);
		// Border Color Image
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = compose_descp_set.descp_set;
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo border_color = {};
		border_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		border_color.imageView = border_color_view.view;
		writes[0].pImageInfo = &border_color;

		// Padding Color Image
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = compose_descp_set.descp_set;
		writes[1].dstBinding = 1;
		writes[1].dstArrayElement = 0;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo padding_color = {};
		padding_color.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		padding_color.imageView = padding_color_view.view;
		writes[1].pImageInfo = &padding_color;

		compose_descp_set.update(writes);
	}

	// Compose Vertex Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/vert.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_vert_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_VERTEX_BIT));
	}

	// Compose Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Compose/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(compose_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Compose Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &compose_descp_layout.descp_layout;

		checkErrStack1(compose_pipe_layout.create(&logical_dev, &info));
	}

	// Compose Pipeline
	{
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
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
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
		pipeline_info.renderPass = compose_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(compose_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(compose_pipe.setDebugName("compose"));
	}

	// Copy Renderpass
	{
		VkAttachmentDescription compose_in_atach = {};
		compose_in_atach.format = swapchain.surface_format.format;
		compose_in_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		compose_in_atach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		compose_in_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		compose_in_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		compose_in_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		compose_in_atach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		compose_in_atach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference compose_in_atach_ref = {};
		compose_in_atach_ref.attachment = 0;
		compose_in_atach_ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription present_atach = {};
		present_atach.format = swapchain.surface_format.format;
		present_atach.samples = VK_SAMPLE_COUNT_1_BIT;
		present_atach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		present_atach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		present_atach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		present_atach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		present_atach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		present_atach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference present_atach_ref = {};
		present_atach_ref.attachment = 1;
		present_atach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 1;
		subpass.pInputAttachments = &compose_in_atach_ref;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &present_atach_ref;

		std::array<VkAttachmentDescription, 2> attachments = {
			compose_in_atach, present_atach
		};

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.pAttachments = attachments.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;

		checkErrStack(copy_renderpass.create(&logical_dev, &info),
			"failed to create renderpass");
		checkErrStack1(copy_renderpass.setDebugName("copy renderpass"));
	}

	// Copy framebuffers
	{
		copy_frames.resize(swapchain.images.size());

		for (auto i = 0; i < copy_frames.size(); i++) {

			vks::Framebuffer& frame = copy_frames[i];

			std::vector<VkImageView> attachments = {
				compose_color_view.view, swapchain.views[i]
			};

			checkErrStack1(frame.create(&logical_dev, &copy_renderpass, attachments,
				swapchain.resolution.width, swapchain.resolution.height));
			checkErrStack1(frame.setDebugName("copy"));
		}
	}

	// Copy Descriptor Layout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(1);
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		checkErrStack1(copy_descp_layout.create(&logical_dev, bindings));
	}

	// Copy Descriptor Pool
	{
		std::vector<VkDescriptorPoolSize> sizes(1);
		sizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		sizes[0].descriptorCount = 1;

		checkErrStack1(copy_descp_pool.create(&logical_dev, sizes));
	}

	// Copy Descriptor Set
	{
		checkErrStack1(copy_descp_set.create(&logical_dev, &copy_descp_pool, &copy_descp_layout));

		std::vector<VkWriteDescriptorSet> writes(1);
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = copy_descp_set.descp_set;
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorImageInfo copy_img_info = {};
		copy_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		copy_img_info.imageView = compose_color_view.view;
		writes[0].pImageInfo = &copy_img_info;

		copy_descp_set.update(writes);
	}

	// Copy Fragment Shader Module
	{
		FileSysPath path;
		checkErrStack1(path.recreateRelative("shaders/Copy/frag.spv"));

		std::vector<char> shader_code;
		checkErrStack1(path.read(shader_code));

		checkErrStack1(copy_frag_module.recreate(&logical_dev, shader_code, VK_SHADER_STAGE_FRAGMENT_BIT));
	}

	// Copy Pipeline Layout
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &copy_descp_layout.descp_layout;

		checkErrStack1(copy_pipe_layout.create(&logical_dev, &info));
	}

	// Copy Pipeline
	{
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
		shader_stages[0] = {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = compose_vert_module.stage;
		shader_stages[0].module = compose_vert_module.sh_module;
		shader_stages[0].pName = "main";

		shader_stages[1] = {};
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = copy_frag_module.stage;
		shader_stages[1].module = copy_frag_module.sh_module;
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
		pipeline_info.layout = copy_pipe_layout.pipe_layout;
		pipeline_info.renderPass = copy_renderpass.renderpass;
		pipeline_info.subpass = 0;

		checkErrStack(copy_pipe.create(&logical_dev, &pipeline_info), "");
		checkErrStack1(copy_pipe.setDebugName("copy"));
	}

	// Command Buffers
	checkErrStack1(render_cmd_buffs.recreate(&logical_dev, &phys_dev, (uint32_t)swapchain.images.size()));

	// Command Buffer Update
	{
		vks::RenderingCmdBuffsUpdateInfo info = {};
		info.width = swapchain.resolution.width;
		info.height = swapchain.resolution.height;

		// Common
		info.border_img = &border_color_img;
		info.padding_img = &padding_color_img;
		info.compose_img = &compose_color_img;

		info.uniform_buff = &uniform_buff;
		info.uniform_descp_set = &uniform_descp_set;

		info.layers = &layers;
		
		// Rect
		info.rect_renderpass = &rect_renderpass;
		info.rect_pipe_layout = &rect_pipe_layout;
		info.rect_pipe = &rect_pipe;

		// Circles
		info.circles_renderpass = &circles_renderpass;
		info.circles_pipe_layout = &circles_pipe_layout;
		info.circles_pipe = &circles_pipe;

		// Border Rect pass	
		info.border_rect_frames = &border_rect_frames;
		info.border_rect_vertex_buff = &border_rect_vertex_buff;

		// Border Circles pass		
		info.border_circles_frames = &border_circles_frames;
		info.border_circles_vertex_buff = &border_circles_vertex_buff;

		// Padding Rect pass	
		info.padding_rect_frames = &padding_rect_frames;
		info.padding_rect_vertex_buff = &padding_rect_vertex_buff;

		// Padding Circles pass
		info.padding_circles_frames = &padding_circles_frames;
		info.padding_circles_vertex_buff = &padding_circles_vertex_buff;

		// Compose pass
		info.compose_renderpass = &compose_renderpass;
		info.compose_frames = &compose_frames;

		info.compose_descp_set = &compose_descp_set;
		info.compose_pipe_layout = &compose_pipe_layout;
		info.compose_pipe = &compose_pipe;

		// Copy
		info.copy_renderpass = &copy_renderpass;
		info.copy_frames = &copy_frames;
		
		info.copy_descp_set = &copy_descp_set;
		info.copy_pipe_layout = &copy_pipe_layout;
		info.copy_pipe = &copy_pipe;

		checkErrStack(render_cmd_buffs.update(info), "failed to update rendering commands");
	}

	// Command Buffer
	{
		auto f = [](vks::CmdBufferTask& task) {

		};
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

ErrStack VulkanRenderer::calc(ui::UserInterface& user_interface)
{
	auto createChamferedRectangle = [&](glm::vec2 origin, float width, float height,
		float tl_radius, float tr_radius, float br_radius, float bl_radius,
		std::array<GPU_Rects_Vertex, 18>& rect_verts)
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
		rect_verts[0].pos = top_left_up;
		rect_verts[1].pos = top_right_up;
		rect_verts[2].pos = top_right_down;

		rect_verts[3].pos = top_left_up;
		rect_verts[4].pos = top_right_down;
		rect_verts[5].pos = bot_right_up;

		rect_verts[6].pos = top_left_up;
		rect_verts[7].pos = bot_right_up;
		rect_verts[8].pos = bot_right_down;

		rect_verts[9].pos = top_left_up;
		rect_verts[10].pos = bot_right_down;
		rect_verts[11].pos = bot_left_down;

		rect_verts[12].pos = top_left_up;
		rect_verts[13].pos = bot_left_down;
		rect_verts[14].pos = bot_left_up;

		rect_verts[15].pos = top_left_up;
		rect_verts[16].pos = bot_left_up;
		rect_verts[17].pos = top_left_down;
	};

	auto createCircle = [](glm::vec2 origin, float radius,
		std::array<GPU_Circles_Vertex, 24>& verts, size_t idx)
	{
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
		verts[idx + 0].pos = top_left;
		verts[idx + 1].pos = top_right;
		verts[idx + 2].pos = bot_right;

		verts[idx + 3].pos = bot_left;
		verts[idx + 4].pos = top_left;
		verts[idx + 5].pos = bot_right;

		// Center and Radius
		glm::vec2 center = vec2_origin;
		center += radius;

		for (auto i = idx; i < idx + 6; i++) {
			verts[i].center = center;
			verts[i].radius = radius;
		}
	};

	this->layers.resize(user_interface.layers.size());
	auto ui_it = user_interface.layers.begin();

	for (auto l = 0; l < user_interface.layers.size(); l++) {

		ui::ElementsLayer& ui_layer = *ui_it;
		GPU_ElementsLayer& gpu_layer = this->layers[l];

		gpu_layer.border_rect.offset = border_rect_vertex_buff.load_size_;
		gpu_layer.border_circles.offset = border_circles_vertex_buff.load_size_;
		gpu_layer.padding_rect.offset = padding_rect_vertex_buff.load_size_;
		gpu_layer.padding_circles.offset = padding_circles_vertex_buff.load_size_;

		for (ui::Element* elem : ui_layer.elems) {

			auto* basic = std::get_if<ui::BasicElement>(&elem->elem);

			// Border Rect
			std::array<GPU_Rects_Vertex, 18> border_rect;
			glm::vec2 border_origin = basic->origin;

			createChamferedRectangle(border_origin, basic->borderbox_width, basic->borderbox_height,
				basic->border_tl_radius, basic->border_tr_radius, basic->border_br_radius, basic->border_bl_radius,
				border_rect);

			for (GPU_Rects_Vertex& vert : border_rect) {
				vert.color = basic->border_color;
			}

			// Border Circles
			std::array<GPU_Circles_Vertex, 24> border_circles;
			{
				glm::vec2 tr_origin = border_origin;
				glm::vec2 br_origin = border_origin;
				glm::vec2 bl_origin = border_origin;

				tr_origin.x += basic->borderbox_width - (basic->border_tr_radius * 2);
				br_origin.x += basic->borderbox_width - (basic->border_br_radius * 2);
				br_origin.y += basic->borderbox_height - (basic->border_br_radius * 2);
				bl_origin.y += basic->borderbox_height - (basic->border_bl_radius * 2);

				createCircle(border_origin, basic->border_tl_radius, border_circles, 0);
				createCircle(tr_origin, basic->border_tr_radius, border_circles, 6);
				createCircle(br_origin, basic->border_br_radius, border_circles, 12);
				createCircle(bl_origin, basic->border_bl_radius, border_circles, 18);

				for (auto& vert : border_circles) {
					vert.color = basic->border_color;
				}
			}

			// Create Padding Box
			std::array<GPU_Rects_Vertex, 18> padding_rect;

			glm::vec2 padding_origin = border_origin;
			padding_origin.x += basic->border_left_thick;
			padding_origin.y += basic->border_top_thick;
			{
				createChamferedRectangle(padding_origin, basic->paddingbox_width, basic->paddingbox_height,
					basic->padding_tl_radius, basic->padding_tr_radius, basic->padding_br_radius, basic->padding_bl_radius,
					padding_rect);

				for (auto& vert : padding_rect) {
					vert.color = basic->background_color;
				}
			}

			// Create Padding Circles
			std::array<GPU_Circles_Vertex, 24> padding_circles;
			{
				glm::vec2 tr_origin = padding_origin;
				glm::vec2 br_origin = padding_origin;
				glm::vec2 bl_origin = padding_origin;

				tr_origin.x += basic->paddingbox_width - (basic->padding_tr_radius * 2);
				br_origin.x += basic->paddingbox_width - (basic->padding_br_radius * 2);
				br_origin.y += basic->paddingbox_height - (basic->padding_br_radius * 2);
				bl_origin.y += basic->paddingbox_height - (basic->padding_bl_radius * 2);

				createCircle(padding_origin, basic->padding_tl_radius, padding_circles, 0);
				createCircle(tr_origin, basic->padding_tr_radius, padding_circles, 6);
				createCircle(br_origin, basic->padding_br_radius, padding_circles, 12);
				createCircle(bl_origin, basic->padding_bl_radius, padding_circles, 18);

				for (auto& vert : padding_circles) {
					vert.color = basic->background_color;
				}
			}

			// Load into buffers
			gpu_layer.border_rect.vertex_count += border_rect.size();
			gpu_layer.border_circles.vertex_count += border_circles.size();
			gpu_layer.padding_rect.vertex_count += padding_rect.size();
			gpu_layer.padding_circles.vertex_count += padding_circles.size();

			checkErrStack1(border_rect_vertex_buff.push(border_rect.data(), border_rect.size() * sizeof(GPU_Rects_Vertex)));
			checkErrStack1(border_circles_vertex_buff.push(border_circles.data(), border_circles.size() * sizeof(GPU_Circles_Vertex)));
			checkErrStack1(padding_rect_vertex_buff.push(padding_rect.data(), padding_rect.size() * sizeof(GPU_Rects_Vertex)));
			checkErrStack1(padding_circles_vertex_buff.push(padding_circles.data(), padding_circles.size() * sizeof(GPU_Circles_Vertex)));
		}

		++ui_it;
	}

	checkErrStack1(border_rect_vertex_buff.flush());
	checkErrStack1(border_circles_vertex_buff.flush());
	checkErrStack1(padding_rect_vertex_buff.flush());
	checkErrStack1(padding_circles_vertex_buff.flush());

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
