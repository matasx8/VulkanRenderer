#include "VulkanRenderer.h"
#include "Shader.h"

MaterialManager::MaterialManager(VulkanRenderer& gfxEngine)
	: m_GfxEngine(gfxEngine), m_BoundMaterial(~0u), m_AllTimeMaterialCount(0)
{

}

void MaterialManager::InitializeDefaultMaterials()
{
	ShaderCreateInfo defaultShader = { "Shaders/shader_vert.spv", "Shaders/shader_frag.spv" };
	constexpr size_t kUniformCount = 3;

	std::vector<uint8_t> Uniforms(kUniformCount);
	Uniforms[0] = kUniformViewProjectionMatrix;
	Uniforms[1] = kUniformSun;
	Uniforms[2] = kUniformCameraPosition;

	defaultShader.uniforms = std::move(Uniforms);
	defaultShader.isInstanced = false;

	std::vector<TextureCreateInfo> textureInfos;
	TextureCreateInfo tci;
	tci.fileName = "plain.png";
	tci.filtering = VK_FILTER_NEAREST;
	tci.wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureInfos.push_back(tci);

	// default material should be 0
	CreateMaterial(defaultShader, textureInfos);

}

void MaterialManager::BindMaterial(size_t frameIndex, uint32_t id)
{
	if (m_BoundMaterial == id)
		return;

	m_BoundMaterial = id;

	const Material& material = GetMaterial(id);
	const Pipeline& pipeline = material.GetPipeline();

	m_GfxEngine.BindPipeline(pipeline.GetVkPipeline());

	const Shader& shader = material.GetShader();
	std::vector<VkDescriptorSet> descriptorSets(2);
	int i = 0;

	descriptorSets[i++] = material.GetDescriptorSet(frameIndex);
	descriptorSets[i++] = material.GetTextureDescriptorSet();

	m_GfxEngine.BindDescriptorSets(descriptorSets.data(), descriptorSets.size(), pipeline.getPipelineLayout());
}		

void MaterialManager::ForceNextBind()
{
	m_BoundMaterial = ~0u;
}

void MaterialManager::PushConstants(const ModelMatrix& modelMatrix, uint32_t materialId)
{
	const VkPipelineLayout layout = GetMaterial(materialId).GetPipeline().getPipelineLayout();
	m_GfxEngine.PushConstants(modelMatrix, layout);
}

uint32_t MaterialManager::CreateMaterial(Material& material)
{
	// first check are we sure we don't have the same material yet. Don't plan to have many materials yet
	// so this linear algorithm will do
	for (auto& mat : m_Materials)
	{
		if (mat == material)
			return mat.GetId();
	}

	// Didn't find any, lets reuse what we can and create a new one
	// Request descriptor sets, let material manager figure out uniform buffers, we only need the descriptor sets
	const auto& si = material.GetShader().m_ShaderInfo;
	EnsureUniformDescriptorSets(material);

	EnsureTextureDescriptorSets(material);

	EnsurePipeline(material);

	material.SetNewMaterialID(m_AllTimeMaterialCount++);

	m_Materials.push_back(material);
	return material.GetId();
}

size_t MaterialManager::UniformTypeToSize(uint8_t type) const
{
	switch (type)
	{
	case kUniformSun:
		return sizeof(glm::vec4) * 2;
	case kUniformCameraPosition:
		return sizeof(glm::vec4);
	case kUniformViewProjectionMatrix:
		return sizeof(glm::mat4) * 2;
	default:
		Debug::LogMsg("Invalid uniform type is being used");
		return 0;
	}
}

std::vector<size_t> MaterialManager::UniformsTypesToSizes(const std::vector<uint8_t>& types) const
{
	std::vector<size_t> sizes(types.size());
	for (int i = 0; i < types.size(); i++)
	{
		sizes[i] = UniformTypeToSize(types[i]);
	}
	return sizes;
}

std::vector<UniformBuffer> MaterialManager::UniformTypesToUniforms(const std::vector<uint8_t>& types) const
{
	std::vector<UniformBuffer> UBOs(types.size());
	for (int i = 0; i < types.size(); i++)
	{
		UBOs[i] = m_UniformCache[types[i]];
	}
	return UBOs;
}

const Material& MaterialManager::GetMaterial(uint32_t idx) const
{
	return m_Materials[idx];
}

stbi_uc* LoadTextureFile(const std::string& fileName, int* width, int* height, VkDeviceSize* imageSize)
{
	// TODO: expand this

	// number of channels image uses
	int channels;

	//load pixel data for image
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image)
	{
		image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb);
		if (!image)
			throw std::runtime_error("Failed to load a Texture file: " + fileName + "\n");
	}

	*imageSize = *width * *height * 4;
	return image;
}

void MaterialManager::CreateMaterial(const ShaderCreateInfo& shaderCreateInfo, const std::vector<TextureCreateInfo>& textureCreateInfos)
{
	assert(textureCreateInfos.size());
	Material material(m_AllTimeMaterialCount);

	material.SetShader(shaderCreateInfo);

	std::vector<Texture> textures = CreateTextures(textureCreateInfos);
	material.SetTextureDescriptorSetLayout(m_GfxEngine.CreateDescriptorSetLayout(textures.size(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
	material.SetTextureDescriptorSet(m_GfxEngine.CreateTextureDescriptorSet(material.GetTextureDescriptorSetLayout(), textures));
	material.SetTextureDescriptions(textureCreateInfos);
	
	m_TextureCache.insert(m_TextureCache.end(), textures.begin(), textures.end());

	// Create UBOs
	auto descriptorSetLayout = m_GfxEngine.CreateDescriptorSetLayout(shaderCreateInfo.uniforms.size(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	auto sizes = UniformsTypesToSizes(shaderCreateInfo.uniforms);

	// Since this is initialization, just create everything without checking caches
	// Should exist only 1 uniform buffer for each type of uniform.
	auto UniformBuffers = m_GfxEngine.CreateUniformBuffers(sizes, shaderCreateInfo.uniforms.size());
	auto DescriptorSets = m_GfxEngine.CreateDescriptorSets(sizes.data(), UniformBuffers, descriptorSetLayout);
	material.SetDescriptorSets(DescriptorSets);

	KeepTrackOfDirtyUniforms(shaderCreateInfo.uniforms);

	CacheUBOs(shaderCreateInfo.uniforms, UniformBuffers);

	material.SetDescriptorSetLayout(descriptorSetLayout);

	// create pipeline
	// pass in something that will indicate renderpass manager what renderpass to use
	Pipeline pipeline = m_GfxEngine.CreatePipeline(material);
	material.SetPipeline(pipeline);

	m_Materials.push_back(material);

	m_AllTimeMaterialCount++;
}

std::vector<Texture> MaterialManager::CreateTextures(const std::vector<TextureCreateInfo>& textureCreateInfos)
{
	// Create textures and etc.
	std::vector<Texture> textures(textureCreateInfos.size());
	for (int i = 0; i < textureCreateInfos.size(); i++)
	{
		// 'legacy' super not flexible creation, good enough for now.
		int width, height;
		VkDeviceSize imageSize;
		stbi_uc* imageData = LoadTextureFile(textureCreateInfos[i].fileName, &width, &height, &imageSize);

		Image image = m_GfxEngine.UploadImage(width, height, imageSize, imageData);
		stbi_image_free(imageData);

		textures[i].AddImage(image);
		textures[i].SetSampler(m_GfxEngine.CreateTextureSampler(textureCreateInfos[i]));
		textures[i].SetTextureDescription(textureCreateInfos[i]);
	}
	return textures;
}

void MaterialManager::KeepTrackOfDirtyUniforms(const std::vector<uint8_t>& types)
{
	for (int i = 0; i < types.size(); i++)
	{
		// we're creating a material with these uniforms. Means we will have to update them.
		// increment so we know how many materials are using. When 0 means we're not using anymore
		m_DirtyUniformTrackingCache[types[i]] += 1;
	}
}

void MaterialManager::EnsureUniformDescriptorSets(Material& material)
{
	// create ubos that need to create, reuse those that we can reuse
	const auto& si = material.GetShader().m_ShaderInfo;

	const auto& requestedUniforms = si.uniforms;

	const auto uniformsToBeCreated = UBOsThatNeedCreation(requestedUniforms);
	std::vector<size_t> sizesOfUBOsToBeCreated = UniformsTypesToSizes(uniformsToBeCreated);
	std::vector<size_t> sizesOfRequested = UniformsTypesToSizes(requestedUniforms);
	std::vector<UniformBuffer> UniformBuffers = m_GfxEngine.CreateUniformBuffers(sizesOfUBOsToBeCreated, sizesOfUBOsToBeCreated.size());
	
	KeepTrackOfDirtyUniforms(requestedUniforms);

	// cache new created ubos
	CacheUBOs(uniformsToBeCreated, UniformBuffers);

	auto requestedActualUniforms = UniformTypesToUniforms(requestedUniforms);

	auto descriptorSetLayout = m_GfxEngine.CreateDescriptorSetLayout(requestedUniforms.size(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	auto DescriptorSets = m_GfxEngine.CreateDescriptorSets(sizesOfRequested.data(), requestedActualUniforms, descriptorSetLayout);
	material.SetDescriptorSetLayout(descriptorSetLayout);
	material.SetDescriptorSets(DescriptorSets);
}

void MaterialManager::EnsureTextureDescriptorSets(Material& material)
{
	// create textures that need to create, reuse those that we can reuse
	const std::vector<TextureCreateInfo>& requestedTextures = material.GetTextureDescriptions();

	const std::vector<TextureCreateInfo> texturesToBeCreated = TexturesThatNeedCreation(requestedTextures);
	const auto createdTextures = CreateTextures(texturesToBeCreated);
	
	m_TextureCache.insert(m_TextureCache.end(), createdTextures.begin(), createdTextures.end());

	const std::vector<Texture> requestedActualTextures = FindTexturesFromDescriptions(requestedTextures);

	auto descriptorSetLayout = m_GfxEngine.CreateDescriptorSetLayout(requestedTextures.size(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	auto descriptorSet = m_GfxEngine.CreateTextureDescriptorSet(descriptorSetLayout, requestedActualTextures);
	material.SetTextureDescriptorSetLayout(descriptorSetLayout);
	material.SetTextureDescriptorSet(descriptorSet);
}

void MaterialManager::EnsurePipeline(Material& material)
{
	// for now just create one. Later use derivatives or whatever a pipeline cache is
	Pipeline pipeline = m_GfxEngine.CreatePipeline(material);
	material.SetPipeline(pipeline);
}

std::vector<uint8_t> MaterialManager::UBOsThatNeedCreation(const std::vector<uint8_t>& requestedUbos) const
{
	std::vector<uint8_t> needCreation;
	for (int i = 0; i < requestedUbos.size(); i++)
	{
		// means this uniform is not being used - so not created
		if (m_DirtyUniformTrackingCache[requestedUbos[i]] == 0)
			needCreation.emplace_back(requestedUbos[i]);
	}
	return needCreation;

}

void MaterialManager::CacheUBOs(const std::vector<uint8_t>& types, std::vector<UniformBuffer>& UBOs)
{
	for (uint8_t i = 0; i < types.size(); i++)
	{
		m_UniformCache[types[i]] = UBOs[i];
	}
}

std::vector<TextureCreateInfo> MaterialManager::TexturesThatNeedCreation(const std::vector<TextureCreateInfo>& requestedTextures) const
{
	std::vector<TextureCreateInfo> needCreation;
	for (int i = 0; i < requestedTextures.size(); i++)
	{
		if (std::find(m_TextureCache.begin(), m_TextureCache.end(), requestedTextures[i]) == m_TextureCache.end())
			needCreation.emplace_back(requestedTextures[i]);
	}
	return needCreation;
}

std::vector<Texture> MaterialManager::FindTexturesFromDescriptions(const std::vector<TextureCreateInfo>& requestedTextures)
{
	std::vector<Texture> textures;
	for (int i = 0; i < requestedTextures.size(); i++)
	{
		const auto it = std::find(m_TextureCache.begin(), m_TextureCache.end(), requestedTextures[i]);
		
		if (it == m_TextureCache.end())
		{
			Debug::LogMsg("Couldn't find requested texture amongst cached ones.");
			throw std::runtime_error("Texture creation errror detected");
		}

		textures.emplace_back(*it);
	}
	return textures;
}

Material& MaterialManager::GetMaterial(uint32_t idx)
{
	return m_Materials[idx];
}

template<typename T>
void UpdateUniform(T& uniformProvider, VulkanRenderer& engine, VkDeviceMemory memory)
{
	void* dataMap = nullptr;
	const size_t size = uniformProvider.ProvideUniformDataSize();
	void* data = alloca(size);
	uniformProvider.ProvideUniformData(data);
	assert(data);

	// had to make this function public because of this
	engine.UpdateMappedMemory(memory, size, data);
}

void MaterialManager::UpdateUniforms()
{
	for (auto i = 0; i < kUniformTypeTotalCount; i++)
	{
		if (m_DirtyUniformTrackingCache[i] == 0)
			continue;	// nobody is using this, no need to update

		const auto idx = m_GfxEngine.GetSwapchainIndex();

		switch (i)
		{
		case kUniformSun:
			UpdateUniform(m_GfxEngine.getActiveScene().GetLight(), m_GfxEngine, m_UniformCache[kUniformSun].deviceMemory[idx]);
			break;
		case kUniformCameraPosition:
			UpdateUniform(m_GfxEngine.getActiveScene().GetCamera(), m_GfxEngine, m_UniformCache[kUniformCameraPosition].deviceMemory[idx]);
			break;
		case kUniformViewProjectionMatrix:
			UpdateUniform(m_GfxEngine.getActiveScene().GetViewProjectionMatrix(), m_GfxEngine, m_UniformCache[kUniformViewProjectionMatrix].deviceMemory[idx]);
			break;
		}
	}
}
