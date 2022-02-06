#include "RenderPassManager.h"
#include "Shader.h"

RenderPassManager::RenderPassManager()
{
}

void RenderPassManager::AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass)
{
	m_RenderPassMap.insert(std::make_pair(desc, renderPass));
}

void RenderPassManager::AddColorCodingRenderPass()
{
	// 1. Do RenderPass before color pass
	// 2. Somehow sinchronize and vkCmdCopyImageToBuffer
	// 3. Get the id of object we are selecting
	// 4. Do Color pass
	// 5. Do outline pass

	RenderPassDesc ColorCode =
	{
		1,	// msaaCount
		kRenderPassPlace_ColorCode, // place
		kTargetColor, 
		1, // collor att count
		VK_FORMAT_B8G8R8A8_UNORM,
		kLoadOpClear,
		kStoreOpStore,					
		VK_FORMAT_D16_UNORM,
		kLoadOpClear,
		kStoreOpDontCare
	};

	RenderPass rp;
	rp.CreateRenderPass(ColorCode);
	AddRenderPass(ColorCode, rp);
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
