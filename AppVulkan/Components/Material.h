#pragma once
#include <string>
#include "Texture.h"
#include "Shader.h"

class Material
{
public:
	Material(size_t id, ShaderCreateInfo& shaderInfo);

	const char* getVertexShader() const;
	const char* getFragmentShader() const;
	const std::vector<UniformData>& getUniformData() const;
	const std::vector<size_t> getDataSizes() const;
	size_t getUboCount() const;
	uint32_t getShaderFlags() const;
	const void* getPushConstantDataBuffer() const;
	const size_t getPushConstantSize() const;
	uint32_t GetInstanceCount() const;

	const bool hasPushConstant() const;
	const bool hasFlag(ShaderCreateInfoFlags flag) const;
	bool IsInstanced() const;



	bool operator==(const Material& mat) const;

	// info variables
private:
	uint32_t m_ID;
	Shader m_Shader;
	std::vector<Texture> textures;

	uint32_t m_InstanceCount;
};