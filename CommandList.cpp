
// Header
#include "VulkanNext.hpp"


using namespace vnx;


void Renderpass::addReadAttachment(ReadAttachmentInfo& info)
{
	Image* img = info.view->image;

	uint32_t atach_idx = atach_descps.size();
	VkAttachmentDescription& atach_descp = atach_descps.emplace_back();
	atach_descp.flags = 0;
	atach_descp.format = img->format;
	atach_descp.samples = img->samples;
	atach_descp.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	atach_descp.storeOp = info.store_op;
	atach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	atach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	atach_descp.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	atach_descp.finalLayout = info.final_fayout;

	VkAttachmentReference& atach_ref = atach_refs.emplace_back();
	atach_ref.attachment = atach_idx;
	atach_ref.layout = info.in_use_layout;

	img_views.push_back(info.view->view);


}
