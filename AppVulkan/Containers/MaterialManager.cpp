#include "VulkanRenderer.h"
#include "Shader.h"

enum UniformType : uint8_t
{
	kUniformSun,
	kUniformCameraPosition,
	kUniformViewProjectionMatrix,
	kUniformTypeTotalCount
};

MaterialManager::MaterialManager(VulkanRenderer& gfxEngine)
	: m_GfxEngine(gfxEngine), m_AllTimeMaterialCount(0)
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

	// default material should be 0
	//assert(m_AllTimeMaterialCount == 0);
	//Material defaultMaterial(m_AllTimeMaterialCount, );
	CreateMaterial(defaultShader);

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

#ifdef _DEBUG
	material.m_ShaderCreateInfo = shaderCreateInfo;
#endif

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
	auto UniformBuffers = m_GfxEngine.CreateUniformBuffers(sizes, shaderCreateInfo.uniforms.size());
	auto descriptorSets = m_GfxEngine.CreateDescriptorSets(sizes.data(), UniformBuffers, descriptorSetLayout);

	material.SetDescriptorSetLayout(descriptorSetLayout);
	material.SetUniformBuffers(UniformBuffers);
	material.SetDescriptorSets(descriptorSets);





	m_AllTimeMaterialCount++;
}
