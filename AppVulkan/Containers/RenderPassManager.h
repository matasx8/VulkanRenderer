#pragma once
#include "RenderPass.h"
#include "Utilities.h"
#include <unordered_map>

class RenderPassManager
{
public:
	RenderPassManager();

	void AddRenderPass(RenderPassDesc& desc, RenderPass& renderPass);
	void AddExampleRenderPass();

	// temporary, dont use this actually
	VkRenderPass GetRenderPass();

	void CleanUp();
private:
	// TODOOO: use ordered map obviously and order by render queue
	std::unordered_map<RenderPassDesc, RenderPass, RenderPassDescHasher> m_RenderPassMap;
};
