#pragma once
#include "Utilities.h"
#include "Debug.h"

class Image
{
public:
	Image():image(0), imageView(0), imageMemory(0) {};
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkSampleCountFlagBits numSamples, VkMemoryPropertyFlags propFlags, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);

	VkImage getImage() const { return image; }
	VkImageView getImageView() const { return imageView; }

	void destroyImage(VkDevice logicalDevice);
	~Image() {};

private:
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
};

