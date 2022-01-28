#include "RenderPassManager.h"
#include "Shader.h"

RenderPassManager::RenderPassManager()
{
}

void RenderPassManager::AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass)
{
	m_RenderPassMap.insert(std::make_pair(desc, renderPass));
}

void RenderPassManager::AddExampleRenderPass()
{
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

RenderPass RenderPassManager::GetRenderPass()
{
	// fix this later
	// seems like I have no way now to specify what renderpass I need.
	// so temporarily do this to retrieve the one and only rp I need.
	for (auto& a : m_RenderPassMap)
		return a.second;
}

void RenderPassManager::CleanUp()
{
	for (auto& a : m_RenderPassMap)
		a.second.Destroy();
}
