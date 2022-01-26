#pragma once

class VulkanRenderer;
class Material;

class MaterialManager
{
public:
	MaterialManager(VulkanRenderer& gfxEngine);

	void InitializeDefaultMaterials();
												
												

private:
	void CreateMaterial(const ShaderCreateInfo& shaderCreateInfo);

	VulkanRenderer& m_GfxEngine;

	uint32_t m_AllTimeMaterialCount;
};
