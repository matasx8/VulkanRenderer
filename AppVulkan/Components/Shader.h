#pragma once
#include <numeric>
#include <vector>
#include <string>
#include <assert.h>
#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "Texture.h"

enum ShaderCreateInfoFlags
{
	kDefault = 0,
	kUseViewProjAsFirstUBO = 1,
	kUseDefaultLightAsSecondUBO = 2,
	kUseModelMatrixForPushConstant = 4
};

struct UniformData
{
	std::vector<size_t> sizes;
	// make sure that dataBuffers passed here are from NonCopyables
	std::vector<void*> dataBuffers;
	std::string name;

	size_t getTotalDataSize() const
	{//!HERE this is called before anything here.
		size_t totalSize = 0;
		for (size_t i = 0; i < sizes.size(); i++)
		{
			totalSize += sizes[i];
		}
		return totalSize;
	}

	// Copies all of the data of this uniform
	// make sure dst has allocated memory that is
	// at least of getTotalDataSize
	void getPackedDataBuffer(void* dst)
	{
		for (size_t i = 0; i < dataBuffers.size(); i++)
		{
			assert(dataBuffers[i]);
			memcpy(dst, dataBuffers[i], sizes[i]);
			dst = static_cast<char*>(dst) + sizes[i];
		}
	}
};

struct PushConstantData
{

};

struct ShaderCreateInfo
{
	// TODO: use gfx caps for push constants at least
	const char* vertexShader;
	const char* fragmentShader;

	std::vector<uint8_t> uniforms;
	std::vector<TextureCreateInfo> textureCreateInfos;

	// push constants
	// if 0, then no push constant
	// TODO: something like with uniformdata..
	size_t pushConstantSize;
	void* pushConstantDataBuffer;

	// to save time, size of instance data will be a mat4 for now
	bool isInstanced;
	unsigned int shaderFlags;

};

class Shader
{
public:
	Shader();
	Shader(const ShaderCreateInfo& shaderInfo);

	bool operator==(const Shader& shader) const;
	
	// Don't need private?
	ShaderCreateInfo m_ShaderInfo;
};

