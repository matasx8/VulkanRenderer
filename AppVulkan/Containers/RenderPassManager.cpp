#include "RenderPassManager.h"
#include "Shader.h"

RenderPassManager::RenderPassManager()
	: m_RenderPassCache()
{
}

void RenderPassManager::InitRenderPasses()
{
	AddOpaqueColorPass();
	AddPresentBlitPass();
}

void RenderPassManager::AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass)
{
	m_RenderPassCache[renderPass.GetRenderPassDesc().renderpassPlace] = renderPass;
}

void RenderPassManager::AddColorCodingRenderPass()
{
	// 1. Do RenderPass before color pass
	// 2. Somehow sinchronize and vkCmdCopyImageToBuffer
	// 3. Get the id of object we are selecting
	// 4. Do Color pass
	// 5. Do outline pass

	//RenderPassDesc ColorCode =
	//{
	//	1,	// msaaCount
	//	kRenderPassPlace_ColorCode, // place
	//	kTargetColor, 
	//	1, // collor att count
	//	VK_FORMAT_B8G8R8A8_UNORM,
	//	kLoadOpClear,
	//	kStoreOpStore,					
	//	VK_FORMAT_D16_UNORM,
	//	kLoadOpClear,
	//	kStoreOpDontCare
	//};

	/*RenderPass rp;
	rp.CreateRenderPass(ColorCode);
	AddRenderPass(ColorCode, rp);*/
}

void RenderPassManager::AddOpaqueColorPass()
{
	RenderPassDesc DrawOpaques =
	{
			8,	// msaaCount
			kRenderPassPlace_Opaques, // place
			1,
			VK_FORMAT_B8G8R8A8_UNORM,
			kLoadOpClear,
			kStoreOpStore,
			VK_FORMAT_D32_SFLOAT,
			kLoadOpClear,
			kStoreOpDontCare
	};
	const uint32_t GameViewColorSurface = 1;
	const SurfaceDesc GameViewColorSurfaceDesc = {
		0, // dont care yet, engine will fill with up when needed
		0,
		VK_FORMAT_B8G8R8A8_UNORM,	// actually should be able to not specify this and let engine pick or something
		8
	};

	const uint32_t GameViewDepthSurface = 2;
	const SurfaceDesc GameViewDepthSurfaceDesc = {
		0, 
		0,
		VK_FORMAT_D32_SFLOAT,
		8
	};

	// resolve surface
	const uint32_t ResolveSurface = 3;
	const SurfaceDesc ResolveSurfaceDesc = {
	0,
	0,
	VK_FORMAT_B8G8R8A8_UNORM,
	1
	};

	std::vector<std::pair<uint32_t, SurfaceDesc>> surfaceDescriptions;
	surfaceDescriptions.emplace_back(GameViewColorSurface, GameViewColorSurfaceDesc);
	surfaceDescriptions.emplace_back(GameViewDepthSurface, GameViewDepthSurfaceDesc);
	surfaceDescriptions.emplace_back(ResolveSurface, ResolveSurfaceDesc);

	// create renderpass
	RenderPass rp;
	rp.CreateRenderPass(DrawOpaques, surfaceDescriptions);
	
	// add the renderpass, now we have initial one for now
	AddRenderPass(DrawOpaques, rp);
}

void RenderPassManager::AddPresentBlitPass()
{
	RenderPassDesc presentBlit =
	{
			1,	// msaaCount
			kRenderPassPlace_PresentBlit, // place
			1,
			VK_FORMAT_B8G8R8A8_UNORM,
			kLoadOpClear,
			kStoreOpStore,
			VK_FORMAT_UNDEFINED, // no depth
			kStoreOpDontCare,
			kStoreOpDontCare
	};
	const uint32_t GameViewColorSurface = 3;
	const SurfaceDesc GameViewColorSurfaceDesc = {
		0, // dont care yet, engine will fill with up when needed
		0,
		VK_FORMAT_B8G8R8A8_UNORM,	// actually should be able to not specify this and let engine pick or something
		1
	};

	std::vector<std::pair<uint32_t, SurfaceDesc>> surfaceDescriptions;
	surfaceDescriptions.emplace_back(GameViewColorSurface, GameViewColorSurfaceDesc);

	// create renderpass
	RenderPass rp;
	rp.CreateRenderPass(presentBlit, surfaceDescriptions);

	// add the renderpass, now we have initial one for now
	AddRenderPass(presentBlit, rp);
}

const RenderPass& RenderPassManager::GetRenderPass(uint8_t renderPassPlace) const
{
	return m_RenderPassCache[renderPassPlace];
}

void RenderPassManager::CleanUp()
{
	/*for (auto& a : m_RenderPassCache)
		a.second.Destroy();*/
}
