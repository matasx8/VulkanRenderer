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

	bool operator==(const TextureCreateInfo& tci) const;
};

class Texture
{
public:
	Texture();

	void AddImage(Image& image);

	void SetSampler(VkSampler sampler);
	void SetTextureDescription(TextureCreateInfo& desc);
	void SetTextureDescription(const TextureCreateInfo& desc);

	VkSampler GetSampler() const;
	Image& getImage() { return m_Image; }
	const Image& getImage() const { return m_Image; }
	
	void DestroyTexture(VkDevice logicalDevice);
	bool operator==(const Texture& texture) const;
	bool operator==(const TextureCreateInfo& texture) const;

private:


	Image m_Image;
	VkSampler m_Sampler;
	TextureCreateInfo m_TextureDescription;
};

