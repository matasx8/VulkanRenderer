#include "RenderPassManager.h"

RenderPassManager::RenderPassManager()
{
}

void RenderPassManager::AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass)
{
	m_RenderPassMap.insert(std::make_pair(desc, renderPass));
}

void RenderPassManager::AddExampleRenderPass()
{
	ShaderCreateInfo shaderInfo = { "Shaders/shader.spv", "Shaders/shader.spv" };
	shaderInfo.uniformCount = 0;
	shaderInfo.pushConstantSize = 0;
	shaderInfo.shaderFlags = 0;
	shaderInfo.isInstanced = false;

	Material quadMaterial = Material(shaderInfo);

	RenderPassDesc DrawOpaques =
	{
		8,	// msaaCount
		kRenderPassPlace_Opaques, // place
		kTargetSwapchain,
		1,
		VK_FORMAT_B8G8R8A8_UNORM,
		kLoadOpClear,
		kStoreOpStore,					// what happens if its dont care on swapchain?
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		kLoadOpClear,
		kStoreOpStore
	};

	// create renderpass
	RenderPass rp;
	rp.CreateRenderPass(DrawOpaques);
	
	// add the renderpass, now we have initial one for now
	AddRenderPass(DrawOpaques, rp);
}

VkRenderPass RenderPassManager::GetRenderPass()
{
	// fix this later
	// seems like I have no way now to specify what renderpass I need.
	// so temporarily do this to retrieve the one and only rp I need.
	for (auto& a : m_RenderPassMap)
		return a.second.GetVkRenderPass();
}

void RenderPassManager::CleanUp()
{
	for (auto& a : m_RenderPassMap)
		a.second.Destroy();
}
