#pragma once
#include "Image.h"
#include "STB/stb_image.h"


class Texture
{
public:
	Texture() {};
	void createTexture(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

	Image& getImage(int index) { return Images[0]; }
	
	void DestroyTexture(VkDevice logicalDevice);
	~Texture() {};

private:
	std::vector<Image> Images; // TODO: implement queue of images

	int createTextureImage(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
};

