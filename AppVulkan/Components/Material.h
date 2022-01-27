#pragma once
#include <vector>

class Shader;
class Texture;
struct UniformData;
struct UniformBuffer;
enum ShaderCreateInfoFlags;

class Material
{
public:
	Material(size_t id);

	const char* getVertexShader() const;
	const char* getFragmentShader() const;
	const std::vector<UniformData>& getUniformData() const;
	const std::vector<size_t> getDataSizes() const;
	size_t getUboCount() const;
	uint32_t getShaderFlags() const;
	const void* getPushConstantDataBuffer() const;
	const size_t getPushConstantSize() const;
	uint32_t GetInstanceCount() const;
	const std::vector<Texture>& GetTextures() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;

	void SetTextures(std::vector<Texture>& textures);
	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

	const bool hasPushConstant() const;
	const bool hasFlag(ShaderCreateInfoFlags flag) const;
	bool IsInstanced() const;



	bool operator==(const Material& mat) const;

#ifdef _DEBUG
	ShaderCreateInfo m_ShaderCreateInfo;
#endif
private:
	uint32_t m_ID;
	Shader m_Shader;
	std::vector<Texture> m_Textures;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	std::vector<UniformBuffer> m_UniformBuffers;

	// probably not needed anymore
	uint32_t m_InstanceCount;
};