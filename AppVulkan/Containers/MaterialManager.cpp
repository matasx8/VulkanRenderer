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
	// TODO: change this to behave like uniforms
	defaultShader.pushConstantSize = 0;
	defaultShader.shaderFlags = kUseModelMatrixForPushConstant;
	defaultShader.isInstanced = false;

	std::vector<TextureCreateInfo> textureInfos;
	TextureCreateInfo tci;
	tci.fileName = "plain.png";
	tci.filtering = VK_FILTER_NEAREST;
	tci.wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureInfos.push_back(tci);
	defaultShader.textureCreateInfos = textureInfos;

	// default material should be 0
	CreateMaterial(defaultShader);

}

void MaterialManager::BindMaterial(size_t frameIndex, uint32_t id)
{
	if (m_BoundMaterial == id)
		return;

	const Material& material = GetMaterial(id);
	const Pipeline& pipeline = material.GetPipeline();

	m_GfxEngine.BindPipeline(pipeline.GetVkPipeline());

	const auto& textures = material.GetTextures();
	const Shader& shader = material.GetShader();
	std::vector<VkDescriptorSet> descriptorSets(2);
	int i = 0;
	//for (auto ubo : shader.m_ShaderInfo.uniforms)
	// descriptor set for all uniforms used
	// have to redo the cache
	descriptorSets[i++] = m_DescriptorSetCache[frameIndex];

	//for (auto& texture : textures)
	// this wont work, need descriptor set for all textures to be used
	descriptorSets[i++] = textures[0].GetDescriptorSet();


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

size_t MaterialManager::UniformTypeToSize(uint8_t type) const
{
	switch (type)
	{
	case kUniformSun:
		return sizeof(glm::vec4) * 2;
	case kUniformCameraPosition:
		return sizeof(glm::vec4);
	case kUniformViewProjectionMatrix:
		return sizeof(ViewProjectionMatrix);
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

std::vector<VkDescriptorSet> MaterialManager::UniformTypesToDescriptorSets(const std::vector<uint8_t>& types) const
{
	std::vector<VkDescriptorSet> descriptorSets(types.size());
	for (int i = 0; i < types.size(); i++)
	{
		descriptorSets[i] = m_DescriptorSetCache[types[i]];
	}
	return descriptorSets;
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

void MaterialManager::CreateMaterial(const ShaderCreateInfo& shaderCreateInfo)
{
	assert(shaderCreateInfo.textureCreateInfos.size());
	Material material(m_AllTimeMaterialCount);

	material.SetShader(shaderCreateInfo);

	// Create textures and etc.
	std::vector<Texture> textures(shaderCreateInfo.textureCreateInfos.size());
	for (int i = 0; i < shaderCreateInfo.textureCreateInfos.size(); i++)
	{
		// 'legacy' super not flexible creation, good enough for now.
		int width, height;
		VkDeviceSize imageSize;
		stbi_uc* imageData = LoadTextureFile(shaderCreateInfo.textureCreateInfos[i].fileName, &width, &height, &imageSize);
	
		Image image = m_GfxEngine.UploadImage(width, height, imageSize, imageData);
		stbi_image_free(imageData);

		textures[i].AddImage(image);
		textures[i].SetSampler(m_GfxEngine.CreateTextureSampler(shaderCreateInfo.textureCreateInfos[i]));
		textures[i].SetDescriptorSetLayout(m_GfxEngine.CreateTextureDescriptorSetLayout());
		textures[i].SetDescriptorSet(m_GfxEngine.CreateTextureDescriptorSet(textures[i]));
	}
	material.SetTextures(textures);

	// Create UBOs
	auto descriptorSetLayout = m_GfxEngine.CreateDescriptorSetLayout(shaderCreateInfo.uniforms.size());
	auto sizes = UniformsTypesToSizes(shaderCreateInfo.uniforms);

	// for now lets create these here. Should actually look at what type of uniforms are being asked, 
	// then check if it exists and then create and cache it. Should exist only 1 uniform buffer and 1 descriptor set
	// for each type of uniform buffer.
	auto UniformBuffers = m_GfxEngine.CreateUniformBuffers(sizes, shaderCreateInfo.uniforms.size());
	auto descriptorSets = m_GfxEngine.CreateDescriptorSets(sizes.data(), UniformBuffers, descriptorSetLayout);

	KeepTrackOfDirtyUniforms(shaderCreateInfo.uniforms);

	// temporary! Make into what I commented above later!! --------------
	for (auto i = 0; i < shaderCreateInfo.uniforms.size(); i++)
	{
		m_UniformCache[shaderCreateInfo.uniforms[i]] = UniformBuffers[i];
		m_DescriptorSetCache[shaderCreateInfo.uniforms[i]] = descriptorSets[i];
	}
	// ------------------------------------------------------------------

	material.SetDescriptorSetLayout(descriptorSetLayout);

	// create pipeline
	// pass in something that will indicate renderpass manager what renderpass to use
	Pipeline pipeline = m_GfxEngine.CreatePipeline(material);
	material.SetPipeline(pipeline);

	m_Materials.push_back(material);

	m_AllTimeMaterialCount++;
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
