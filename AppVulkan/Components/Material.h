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
	uint32_t getShaderFlags() const;
	const void* getPushConstantDataBuffer() const;
	const size_t getPushConstantSize() const;
	uint32_t GetInstanceCount() const;
	const std::vector<Texture>& GetTextures() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;
	const std::vector<VkDescriptorSet>& GetDescriptorSets() const;
	VkDescriptorSet GetDescriptorSet(int swapchainIndex) const;
	const Pipeline& GetPipeline() const;
	const Shader& GetShader() const;

	void SetTextures(std::vector<Texture>& textures);
	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	void SetDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets);
	void SetShader(const ShaderCreateInfo& createInfo);
	void SetPipeline(Pipeline pipeline);

	const bool hasPushConstant() const;
	const bool hasFlag(ShaderCreateInfoFlags flag) const;
	bool IsInstanced() const;



	bool operator==(const Material& mat) const;

private:
	uint32_t m_ID;
	Shader m_Shader;
	std::vector<Texture> m_Textures;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	Pipeline m_Pipeline;

	// probably not needed anymore
	uint32_t m_InstanceCount;
};