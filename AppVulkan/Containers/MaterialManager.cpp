#include "VulkanRenderer.h"
#include "Shader.h"

enum UniformType : uint8_t
{
	kUniformSun,
	kUniformCameraPosition,
	kUniformViewProjectionMatrix
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
	m_GfxEngine.CreateDescriptorSetLayout(shaderCreateInfo.uniforms.size());
	// Next step - instead of letting user make their own uniforms I introduced a an enum for a set of predefined
	// uniforms. I need to implement this into uniform creation and then some kind of smart updating for uniforms.
	// also could check if the layout of uniforms matches so we can reuse them and not create new ones.
	// for creation we need only size, so at least that part will be easy.
	m_GfxEngine.CreateUniformBuffers()

	m_AllTimeMaterialCount++;
}
