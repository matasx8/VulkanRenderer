#pragma once
#include <vector>

class VulkanRenderer;
class Material;
struct ShaderCreateInfo;

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

	std::vector<VkDescriptorSet> UniformTypesToDescriptorSets(const std::vector<uint8_t>& types) const;

	const Material& GetMaterial(uint32_t idx) const;
												
	void UpdateUniforms();

	void BindMaterial(size_t frameIndex, uint32_t materialId);
	void ForceNextBind();
	void PushConstants(const ModelMatrix& modelMatrix, uint32_t materialId);

private:
	void CreateMaterial(const ShaderCreateInfo& shaderCreateInfo);
	void KeepTrackOfDirtyUniforms(const std::vector<uint8_t>& types);

	Material& GetMaterial(uint32_t idx);

	VulkanRenderer& m_GfxEngine;

	std::array<size_t, kUniformTypeTotalCount> m_DirtyUniformTrackingCache;
	std::array<UniformBuffer, kUniformTypeTotalCount> m_UniformCache;
	std::array<VkDescriptorSet, kUniformTypeTotalCount> m_DescriptorSetCache;
	//TODO: make into unordered map probably? Though this gives O(1) time get, probably
	// best, because my engine wont have many materials anyway
	std::vector<Material> m_Materials;

	uint32_t m_BoundMaterial;

	uint32_t m_AllTimeMaterialCount;
};
