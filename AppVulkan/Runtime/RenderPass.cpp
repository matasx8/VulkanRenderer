#include "RenderPass.h"

void RenderPass::CreateRenderPass(const RenderPassDesc& desc)
{
	// setup color attachments
	std::vector<VkAttachmentDescription> colorDescriptions;
	if (desc.colorFormat)
	{
		colorDescriptions.resize(desc.colorAttachmentCount);
		for (int i = 0; i < desc.colorAttachmentCount; i++)
		{
			colorDescriptions[i].flags = 1;
			colorDescriptions[i].format = static_cast<VkFormat>(desc.colorFormat);
			colorDescriptions[i].samples = static_cast<VkSampleCountFlagBits>(desc.msaaCount);
			colorDescriptions[i].loadOp = static_cast<VkAttachmentLoadOp>(desc.colorLoadOp);
			colorDescriptions[i].storeOp = static_cast<VkAttachmentStoreOp>(desc.colorStoreOp);
			if(desc.msaaCount)
				colorDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else
				colorDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
	}

	// setup depth attachments
	VkAttachmentDescription depthAttachment = { };
	if (desc.depthFormat)
	{
		depthAttachment.format = static_cast<VkFormat>(desc.depthFormat);
		depthAttachment.samples = static_cast<VkSampleCountFlagBits>(desc.msaaCount);
		depthAttachment.loadOp = static_cast<VkAttachmentLoadOp>(desc.depthLoadOp);
		depthAttachment.storeOp = static_cast<VkAttachmentStoreOp>(desc.depthStoreOp);
		depthAttachment.stencilLoadOp = static_cast<VkAttachmentLoadOp>(desc.depthLoadOp);
		depthAttachment.stencilStoreOp = static_cast<VkAttachmentStoreOp>(desc.depthStoreOp);
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentDescription colorAttachmentResolve{};
	if (desc.msaaCount > 1)
	{
		if (desc.colorFormat == 0)
		{
			printf("Yup, that's bad. Currently we expect that if msaa is enabled then color must exist");
		}
		colorAttachmentResolve.format = static_cast<VkFormat>(desc.colorFormat);
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	
	std::vector<VkAttachmentReference> colorAttachments;
	colorAttachments.resize(desc.colorAttachmentCount);
	for (int i = 0; i < desc.colorAttachmentCount; i++)
	{
		colorAttachments[i].attachment = 0;
		colorAttachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depthAttachmentReference;
	if (desc.depthFormat)
	{
		depthAttachmentReference.attachment = desc.colorAttachmentCount; // depth is indexed after color
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference colorAttachmentResolveReference;
	if (desc.msaaCount)
	{
		if(desc.depthFormat)
			colorAttachmentResolveReference.attachment = desc.colorAttachmentCount + 1;
		else
			colorAttachmentResolveReference.attachment = desc.colorAttachmentCount;
		colorAttachmentResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = desc.colorAttachmentCount;
	subpass.pColorAttachments = desc.colorAttachmentCount ? colorAttachments.data() : nullptr;
	subpass.pDepthStencilAttachment = desc.depthFormat ? &depthAttachmentReference : nullptr;
	subpass.pResolveAttachments = desc.msaaCount ? &colorAttachmentResolveReference : nullptr;
	
	// so far we have 1 subpass supported so leave this
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// reuse the vector we have to store all descriptions
	if(desc.depthFormat)
		colorDescriptions.push_back(std::move(depthAttachment));

	if (desc.msaaCount)
		colorDescriptions.push_back(std::move(colorAttachmentResolve));

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = colorDescriptions.size();
	renderPassCreateInfo.pAttachments = colorDescriptions.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	auto device = GetGraphicsDevice();
	VkResult result = vkCreateRenderPass(device.logicalDevice, &renderPassCreateInfo, nullptr, &m_RenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass");
	}
}

void RenderPass::Destroy()
{
	auto device = GetGraphicsDevice();
	vkDestroyRenderPass(device.logicalDevice, m_RenderPass, nullptr);
}

void RenderPass::DrawQuad(Material material)
{
	// should make a material manager and I should only record the index (or something like that) of the material
	// will probably be have to have fast random access
	mat = material;
}
