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

};

class Texture
{
public:
	Texture() {};
	void createTexture(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

	Image& getImage(int index) { return Images[0]; }

	VkDescriptorSet descriptorSet;
	
	void DestroyTexture(VkDevice logicalDevice);

private:
	std::vector<Image> Images; // TODO: implement queue of images

	int createTextureImage(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
};

