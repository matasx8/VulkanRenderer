#pragma once
#include <vector>
#include "Pipeline.h"

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
	size_t getUboCount() const;
	uint32_t getShaderFlags() const;
	const void* getPushConstantDataBuffer() const;
	const size_t getPushConstantSize() const;
	uint32_t GetInstanceCount() const;
	const std::vector<Texture>& GetTextures() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;

	void SetTextures(std::vector<Texture>& textures);
	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	void SetShader(const ShaderCreateInfo& createInfo);
	void SetPipeline(Pipeline pipeline);

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
	Pipeline m_Pipeline;

	// probably not needed anymore
	uint32_t m_InstanceCount;
};