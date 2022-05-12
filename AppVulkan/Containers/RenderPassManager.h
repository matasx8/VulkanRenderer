#pragma once
#include "RenderPass.h"
#include "Utilities.h"
#include <unordered_map>

class RenderPassManager
{
public:
	RenderPassManager();

	void InitRenderPasses();

	const RenderPass& GetRenderPass(uint8_t renderPassPlace) const;

	void CleanUp();
private:

	void AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass);
	void AddColorCodingRenderPass();
	void AddOpaqueColorPass();
	void AddPresentBlitPass();

	std::array<RenderPass, KRenderPassPlace_RenderpassCount> m_RenderPassCache;
};

