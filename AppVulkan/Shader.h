#pragma once

struct ShaderCreateInfo
{
	const char* vertexShader;
	const char* fragmentShader;

	// Uniform data
	size_t uniformCount;
	// must be the same amount as uniformCount
	size_t* uniformSizes;
	// must be the same size as uniformSizes
	void* uniformDataBuffer;

	// push constants
	// if 0, then no push constant
	size_t pushConstantSize;

	// textures.. tbc..
};

class Shader
{
public:
	Shader();
	Shader(ShaderCreateInfo& shaderInfo);

private:
	ShaderCreateInfo m_ShaderInfo;
};

