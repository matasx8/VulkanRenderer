#pragma once
#include <vector>

class VulkanRenderer;
class Material;

enum UniformType : uint8_t
{
	kUniformSun,
	kUniformCameraPosition,
	kUniformViewProjectionMatrix,
	kUniformTypeTotalCount
};

class MaterialManager
{
public:
	MaterialManager(VulkanRenderer& gfxEngine);

	void InitializeDefaultMaterials();

	size_t UniformTypeToSize(uint8_t type) const;
	std::vector<size_t> UniformsTypesToSizes(const std::vector<uint8_t>& types) const;
												
	void UpdateUniforms();

private:
	void CreateMaterial(const ShaderCreateInfo& shaderCreateInfo);
	void KeepTrackOfDirtyUniforms(const std::vector<uint8_t>& types);

	VulkanRenderer& m_GfxEngine;

	std::array<size_t, kUniformTypeTotalCount> m_DirtyUniformTrackingCache;
	std::array<UniformBuffer, kUniformTypeTotalCount> m_UniformCache;
	std::array<VkDescriptorSet, kUniformTypeTotalCount> m_DescriptorSetCache;

	uint32_t m_AllTimeMaterialCount;
};
