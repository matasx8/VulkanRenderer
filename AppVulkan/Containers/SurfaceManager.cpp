#include "VulkanRenderer.h"

SurfaceManager::SurfaceManager(VulkanRenderer& engine)
	: m_GfxEngine(engine), m_WindowSurface(), m_Swapchain(), m_SurfacePool(), m_SwapchainSurfaces()
{
}

void SurfaceManager::CreateSwapchainSurfaces(VkSurfaceKHR windowSurface, SwapchainInfo swapchain)
{
	m_WindowSurface = windowSurface;
	m_Swapchain = swapchain;

	m_SwapchainSurfaces = m_GfxEngine.CreateSwapchainSurfaces(m_Swapchain);
}

VkExtent2D SurfaceManager::GetSwapchainExtent() const
{
	return m_Swapchain.swapchainExtent;
}

VkSwapchainKHR SurfaceManager::GetSwapchain() const
{
	return m_Swapchain.swapchain;
}

VkFramebuffer SurfaceManager::GetFramebuffer(const RenderPass& rp, uint32_t swapchainIndex)
{
	// TODO: Could store framebuffers in an unordered map. Then try to query with VkRp and check if it still fits.
	const auto requestedSurfaceDescs = rp.GetSurfaceDescriptions();
	const auto surfaceViews = GetAndEnsureRequestedSurfacesesViews(requestedSurfaceDescs, rp, swapchainIndex);

	VkFramebuffer fb = CreateFramebuffer(surfaceViews, rp);

	return fb;
}

Surface SurfaceManager::GetSurface(uint32_t surfaceSlot, uint32_t swapchainIndex)
{
	if (surfaceSlot == 0)
		return m_SwapchainSurfaces[swapchainIndex];
	return m_SurfacePool[surfaceSlot];
}

void SurfaceManager::CombForUnusedSurfaces()
{
}

	// Remember, managers shouldn't be able to communicate with eachother.
	// Somehow must make surface manager not communicate with renderpass manager, but still
	// create resources easily in-sync with renderpass creation. Think about that and resume completing this.
	// after that continue with trying to make present blit.

	// Current idea:
	// Only create the swapchain and backbuffers.
	// Use 'temporary surfaces' for any other surfaces.
	// must think of a system to handle temp surfaces and pass them over

	// Mater Plan #1:
	// Have a pool of surfaces*3 that is a map*1 and key is just an uint.
	//		have a few predefined uints like 'backbuffer'*2 , 'main' and etc. For example:
	//		GetRenderSurface(kSurfaceMain, surfaceDesc); -- and that gets or creates a surface. 
	//		Error if present surface is not same as desc.
	// RenderPass needs a frame buffer, so check first if the FB that the current RenderPass has is still the same
	//		(might have been too old and been discarded. It's a pool afterall)
	//		if it's still same then lets bind that RenderPass and good to go.
	//	*1 - Would need to iterate over the hash table each frame and check for old ones. If it's a sorted
	// map then should be all good? I also remember something about a DS that certain values stay up
	//	*2 - I'm not sure if backbuffer should be in that container
	//	*3 - Should I do something like RenderTargets? Maybe just store surfaces and query a request for an array of surfaces

void SurfaceManager::CleanUp()
{
	// todo:
}

Surface SurfaceManager::CreateSurface(const SurfaceDesc& desc)
{
	Surface surf = m_GfxEngine.CreateSurface(desc);
	return surf;
}

VkFramebuffer SurfaceManager::CreateFramebuffer(const std::vector<VkImageView>& imageViews, const RenderPass& rp)
{
	VkFramebuffer fb = m_GfxEngine.CreateFramebuffer(imageViews, rp);
	return fb;
}

std::vector<VkImageView> SurfaceManager::GetAndEnsureRequestedSurfacesesViews(const std::vector<std::pair<uint32_t, SurfaceDesc>>& requestedSurfaces, const RenderPass& rp, uint32_t swapchainIndex)
{
	std::vector<VkImageView> views(requestedSurfaces.size());
	for (int i = 0; i < requestedSurfaces.size(); i++)
	{
		// first lets try to find in pool by given 'slot/name'
		const auto it = m_SurfacePool.find(requestedSurfaces[i].first);

		if (it == m_SurfacePool.end() && requestedSurfaces[i].first != 0) // 0 reserved for backbuffer
		{
			Surface surf = CreateSurface(requestedSurfaces[i].second);
			surf.UpdateLastUsed(m_GfxEngine.GetCurrentFrame());

			m_SurfacePool.insert(std::make_pair(requestedSurfaces[i].first, surf));
			views[i] = surf.GetImage().getImageView();
		}
		else if (requestedSurfaces[i].first == 0) // 0 reserved for backbuffer
		{
			views[i] = m_SwapchainSurfaces[swapchainIndex].GetImage().getImageView();
		}
		else
		{
			if ((*it).second.GetDesc() != requestedSurfaces[i].second)
			{
				// we found a surface in that slot, but the desc doesn't match
				// lets overwrite it.
				// TODOOOOOOOO: destroy old surface!!!! Also find out if I actually can straight up destroy here
				// probably should add it to some flush buffer that will enable it to being destroyed when frame ends
				Surface surf = CreateSurface(requestedSurfaces[i].second);
				surf.UpdateLastUsed(m_GfxEngine.GetCurrentFrame());

				m_SurfacePool.insert(std::make_pair(requestedSurfaces[i].first, surf));
				views[i] = surf.GetImage().getImageView();
			}
			else
			{
				(*it).second.UpdateLastUsed(m_GfxEngine.GetCurrentFrame());
				views[i] = (*it).second.GetImage().getImageView();
			}
		}
	}

	return views;
}
