#include "Material.h"
#include "Texture.h"
#include "Shader.h"

Material::Material(size_t id)
	: m_ID(id)
{
	// TODO: init vars
}

const char* Material::getVertexShader() const
{
	return m_Shader.m_ShaderInfo.vertexShader.c_str();
}

const char* Material::getFragmentShader() const
{
	return m_Shader.m_ShaderInfo.fragmentShader.c_str();
}

size_t Material::getUboCount() const
{
	return m_Shader.m_ShaderInfo.uniforms.size();
}

uint32_t Material::GetInstanceCount() const
{
	return m_InstanceCount;
}

VkDescriptorSetLayout Material::GetDescriptorSetLayout() const
{
	return m_DescriptorSetLayout;
}

VkDescriptorSet Material::GetDescriptorSet(int swapchainIndex) const
{
	return m_DescriptorSets[swapchainIndex];
}

VkDescriptorSetLayout Material::GetTextureDescriptorSetLayout() const
{
	return m_TextureDescriptorSetLayout;
}

VkDescriptorSet Material::GetTextureDescriptorSet() const
{
	return m_TextureDescriptorSet;
}

const Pipeline& Material::GetPipeline() const
{
	return m_Pipeline;
}

const Shader& Material::GetShader() const
{
	return m_Shader;
}

const uint32_t Material::GetId() const
{
	return m_ID;
}

const std::vector<TextureCreateInfo>& Material::GetTextureDescriptions() const
{
	return m_TextureDescriptions;
}

void Material::SetTextureDescriptions(const std::vector<TextureCreateInfo>& descs)
{
	m_TextureDescriptions = descs;
}

void Material::SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	m_DescriptorSetLayout = descriptorSetLayout;
}

void Material::SetDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets)
{
	m_DescriptorSets = descriptorSets;
}

void Material::SetTextureDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	m_TextureDescriptorSetLayout = descriptorSetLayout;
}

void Material::SetTextureDescriptorSet(VkDescriptorSet descriptorSet)
{
	m_TextureDescriptorSet = descriptorSet;
}

void Material::SetShader(const ShaderCreateInfo& createInfo)
{
	m_Shader = Shader(createInfo);
}

void Material::SetPipeline(Pipeline pipeline)
{
	m_Pipeline = pipeline;
}

void Material::ChangeTextures(std::vector<TextureCreateInfo>& newTextures)
{
	m_TextureDescriptions = newTextures;
}

bool Material::IsInstanced() const
{
	return m_Shader.m_ShaderInfo.isInstanced;
}

bool Material::operator==(const Material& mat) const
{
	return mat.m_Shader == m_Shader && mat.m_TextureDescriptions == m_TextureDescriptions;
}

void Material::SetNewMaterialID(uint32_t id)
{
	m_ID = id;
}
