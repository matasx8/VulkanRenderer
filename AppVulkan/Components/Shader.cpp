#include "Shader.h"

Shader::Shader()
{
}

Shader::Shader(ShaderCreateInfo& shaderInfo)
	:m_ShaderInfo(shaderInfo)
{
}

bool Shader::operator==(const Shader& shader) const
{
	return m_ShaderInfo.vertexShader == shader.m_ShaderInfo.vertexShader && 
		m_ShaderInfo.fragmentShader == shader.m_ShaderInfo.fragmentShader;
}
