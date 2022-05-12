#pragma once
#include <vector>
#include "Pipeline.h"
#include "Shader.h"
#include "Texture.h"

struct UniformData;
struct UniformBuffer;
struct ShaderCreateInfo;
enum ShaderCreateInfoFlags : size_t;

class Material
{
public:
	Material(size_t id);

	const char* getVertexShader() const;
	const char* getFragmentShader() const;
	size_t getUboCount() const;
	uint32_t GetInstanceCount() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;
	VkDescriptorSet GetDescriptorSet(int swapchainIndex) const;
	VkDescriptorSetLayout GetTextureDescriptorSetLayout() const;
	VkDescriptorSet GetTextureDescriptorSet() const;
	const Pipeline& GetPipeline() const;
	const Shader& GetShader() const;
	const uint32_t GetId() const;
	const std::vector<TextureCreateInfo>& GetTextureDescriptions() const;

	void SetTextureDescriptions(const std::vector<TextureCreateInfo>& descs);
	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	void SetDescriptorSets(std::vector<VkDescriptorSet>& descriptorSet);
	void SetTextureDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	void SetTextureDescriptorSet(VkDescriptorSet descriptorSet);
	void SetShader(const ShaderCreateInfo& createInfo);
	void SetPipeline(Pipeline pipeline);

	void ChangeTextures(std::vector<TextureCreateInfo>& newTextures);

	bool IsInstanced() const;



	bool operator==(const Material& mat) const;

private:
	friend class MaterialManager;

	void SetNewMaterialID(uint32_t id);

	uint32_t m_ID;
	Shader m_Shader;
	std::vector<TextureCreateInfo> m_TextureDescriptions;
	// no need for vector, will have just one dset for uniforms and one for textures
	VkDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	VkDescriptorSetLayout m_TextureDescriptorSetLayout;
	VkDescriptorSet m_TextureDescriptorSet;

	Pipeline m_Pipeline;

	// probably not needed anymore
	uint32_t m_InstanceCount;
};