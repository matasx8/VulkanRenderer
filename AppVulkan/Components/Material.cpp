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

bool Material::operator==(const Material& mat) const
{
	return mat.m_Shader == m_Shader;
}
