#include "Material.h"

Material::Material()
{

}

Material::Material(ShaderCreateInfo& shaderInfo)
{
	m_Shader = Shader(shaderInfo);
}

const char* Material::getVertexShader() const
{
	return m_Shader.m_ShaderInfo.vertexShader;
}

const char* Material::getFragmentShader() const
{
	return m_Shader.m_ShaderInfo.fragmentShader;
}

const std::vector<UniformData>& Material::getUniformData() const
{
	return m_Shader.m_ShaderInfo.uniformData;
}

const std::vector<size_t> Material::getDataSizes() const
{
	return m_Shader.m_ShaderInfo.getUniformDataTotalSizes();
}

size_t Material::getUboCount() const
{
	return m_Shader.m_ShaderInfo.uniformCount;
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

const bool Material::hasPushConstant() const
{
	return getPushConstantSize() || (m_Shader.m_ShaderInfo.shaderFlags & kUseModelMatrixForPushConstant);
}

const bool Material::hasFlag(ShaderCreateInfoFlags flag) const
{
	return m_Shader.m_ShaderInfo.shaderFlags & flag;
}

bool Material::operator==(const Material& mat) const
{
	return mat.m_Shader == m_Shader;
}
