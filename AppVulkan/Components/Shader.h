#pragma once
#include <numeric>
#include <vector>
#include <string>
#include <assert.h>
#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "Texture.h"

enum ShaderCreateInfoFlags : size_t
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

struct ShaderCreateInfo
{
	std::string vertexShader;
	std::string fragmentShader;

	std::vector<uint8_t> uniforms;
	uint8_t textureCount;

	bool isInstanced;
	bool isDepthTestEnabled;
	bool isViewportDynamic;
};

// hm it's a bit weird that shadercreateinfo and shader are seperate classes. There's no more need for that?
class Shader
{
public:
	Shader();
	Shader(const ShaderCreateInfo& shaderInfo);

	bool operator==(const Shader& shader) const;
	
	ShaderCreateInfo m_ShaderInfo;
};

