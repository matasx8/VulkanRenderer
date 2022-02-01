#include "Material.h"
#include "Texture.h"
#include "Shader.h"

Material::Material(size_t id)
	: m_ID(id)
{
}

const char* Material::getVertexShader() const
{
	return m_Shader.m_ShaderInfo.vertexShader;
}

const char* Material::getFragmentShader() const
{
	return m_Shader.m_ShaderInfo.fragmentShader;
}

size_t Material::getUboCount() const
{
	return m_Shader.m_ShaderInfo.uniforms.size();
}

uint32_t Material::getShaderFlags() const
{
	return m_Shader.m_ShaderInfo.shaderFlags;
}

const void* Material::getPushConstantDataBuffer() const
{
	return m_Shader.m_ShaderInfo.pushConstantDataBuffer;
}

const size_t Material::getPushConstantSize() const
{
	return m_Shader.m_ShaderInfo.pushConstantSize;
}

uint32_t Material::GetInstanceCount() const
{
	return m_InstanceCount;
}

const std::vector<Texture>& Material::GetTextures() const
{
	return m_Textures;
}

VkDescriptorSetLayout Material::GetDescriptorSetLayout() const
{
	return m_DescriptorSetLayout;
}

const Pipeline& Material::GetPipeline() const
{
	return m_Pipeline;
}

const Shader& Material::GetShader() const
{
	return m_Shader;
}

void Material::SetTextures(std::vector<Texture>& textures)
{
	m_Textures = textures;
}

void Material::SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	m_DescriptorSetLayout = descriptorSetLayout;
}

void Material::SetShader(const ShaderCreateInfo& createInfo)
{
	m_Shader = Shader(createInfo);
}

void Material::SetPipeline(Pipeline pipeline)
{
	m_Pipeline = pipeline;
}

const bool Material::hasPushConstant() const
{
	return getPushConstantSize() || (m_Shader.m_ShaderInfo.shaderFlags & kUseModelMatrixForPushConstant);
}

const bool Material::hasFlag(ShaderCreateInfoFlags flag) const
{
	return m_Shader.m_ShaderInfo.shaderFlags & flag;
}

bool Material::IsInstanced() const
{
	return m_Shader.m_ShaderInfo.isInstanced;
}

bool Material::operator==(const Material& mat) const
{
	return mat.m_Shader == m_Shader;
}
