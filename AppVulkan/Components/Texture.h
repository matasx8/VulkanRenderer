#pragma once
#include "Image.h"
#include "STB/stb_image.h"

enum Texture2dFormat : uint8_t
{
	TextureFormatNone = 0,
	TextureFormatRGBA8888 = VK_FORMAT_R8G8B8A8_UNORM,
	TextureFormatD32S8 = VK_FORMAT_D32_SFLOAT_S8_UINT
};

struct TextureCreateInfo
{
	std::string fileName;
	VkFilter filtering;
	VkSamplerAddressMode wrap;
};

class Texture
{
public:
	Texture();

	void AddImage(Image& image);

	void SetDescriptorSet(VkDescriptorSet descriptorSet);
	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	void SetSampler(VkSampler sampler);

	VkDescriptorSet GetDescriptorSet() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;
	VkSampler GetSampler() const;
	Image& getImage() { return m_Image; }
	
	void DestroyTexture(VkDevice logicalDevice);

private:
	Image m_Image;
	VkDescriptorSet m_DescriptorSet;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkSampler m_Sampler;
};

