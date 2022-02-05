#include "Shader.h"

Shader::Shader()
{
}

Shader::Shader(const ShaderCreateInfo& shaderInfo)
{
	m_ShaderInfo = shaderInfo;
}

bool Shader::operator==(const Shader& shader) const
{
	const auto& lhs = m_ShaderInfo;
	const auto& rhs = shader.m_ShaderInfo;
	if (lhs.vertexShader != rhs.vertexShader)
		return false;
	if (lhs.fragmentShader != rhs.fragmentShader)
		return false;
	if (lhs.uniforms != rhs.uniforms)
		return false;
	if (lhs.isInstanced != rhs.isInstanced)
		return false;
	return true;
}
