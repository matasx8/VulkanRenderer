#pragma once
#include <vector>

class VulkanRenderer;
class Material;

class MaterialManager
{
public:
	MaterialManager(VulkanRenderer& gfxEngine);

	void InitializeDefaultMaterials();

	size_t UniformTypeToSize(uint8_t type) const;
	std::vector<size_t> UniformsTypesToSizes(const std::vector<uint8_t>& types) const;
												
												

private:
	void CreateMaterial(const ShaderCreateInfo& shaderCreateInfo);

	VulkanRenderer& m_GfxEngine;

	uint32_t m_AllTimeMaterialCount;
};
