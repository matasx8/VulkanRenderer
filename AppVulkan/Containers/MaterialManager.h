#pragma once

class VulkanRenderer;

class MaterialManager
{
public:
	MaterialManager(VulkanRenderer& gfxEngine);

	void InitializeDefaultMaterials();


private:
	VulkanRenderer& m_GfxEngine;

	uint32_t m_AllTimeMaterialCount;
};
