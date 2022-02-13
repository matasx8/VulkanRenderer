#pragma once
#include <vector>
#include <unordered_map>

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

enum DefaultMaterials
{
	kMaterialDefault,
	kMaterialSelected,
	kDefaultMaterialCount
};

class MaterialManager
{
public:
	MaterialManager(VulkanRenderer& gfxEngine);

	void InitializeDefaultMaterials();

	size_t UniformTypeToSize(uint8_t type) const;
	std::vector<size_t> UniformsTypesToSizes(const std::vector<uint8_t>& types) const;
	std::vector<UniformBuffer> UniformTypesToUniforms(const std::vector<uint8_t>& types) const;

	const Material& GetMaterial(uint32_t idx) const;
												
	void UpdateUniforms();

	void BindMaterial(size_t frameIndex, uint32_t materialId);
	void ForceNextBind();
	void PushConstants(const ModelMatrix& modelMatrix, uint32_t materialId);
	
	uint32_t CreateMaterial(Material& material);

private:

	void CreateMaterial(const ShaderCreateInfo& shaderCreateInfo, const std::vector<TextureCreateInfo>& textureCreateInfos);
	std::vector<Texture> CreateTextures(const std::vector<TextureCreateInfo>& textureCreateInfos);
	void KeepTrackOfDirtyUniforms(const std::vector<uint8_t>& types);
	void EnsureUniformDescriptorSets(Material& material);
	void EnsureTextureDescriptorSets(Material& material);
	void EnsurePipeline(Material& material);
	std::vector<uint8_t> UBOsThatNeedCreation(const std::vector<uint8_t>& requestedUbos) const;
	void CacheUBOs(const std::vector<uint8_t>& types, std::vector<UniformBuffer>& UBOs);
	std::vector<TextureCreateInfo> TexturesThatNeedCreation(const std::vector<TextureCreateInfo>& requestedTextures) const;
	std::vector<Texture> FindTexturesFromDescriptions(const std::vector<TextureCreateInfo>& requestedTextures);

	Material& GetMaterial(uint32_t idx);

	VulkanRenderer& m_GfxEngine;

	std::array<size_t, kUniformTypeTotalCount> m_DirtyUniformTrackingCache;
	std::array<UniformBuffer, kUniformTypeTotalCount> m_UniformCache;

	// again for now have a vector. When I do UI make into data structure and figure out the whole
	// multiple images, view and etc.
	std::vector<Texture> m_TextureCache;
	// To save time leave this as a vector for now
	std::vector<Material> m_Materials;

	uint32_t m_BoundMaterial;

	uint32_t m_AllTimeMaterialCount;
};
