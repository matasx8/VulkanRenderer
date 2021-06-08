#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "vulkan.h"
#include "STB/stb_image.h"
// Contains data of models and components of the scene
// TODO: read scene data from a .MRscene file

class Scene
{
public:
	Scene() {};
	Scene(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	// Add and load model to currently used scene
	void AddModel(std::string path, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool);
	std::vector<Model>& GetModels();
	static VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkSampleCountFlagBits numSamples, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);
	void CleanUp();

	//void AddCamera();
	//void AddLight();
	// TODO: async model upload
	//void AddModels(paths);

	~Scene();
//private:
	std::vector<Model> Models;
	//assets
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;//would be better to have a single buffer, and images reference offsets
	std::vector<VkImageView> textureImageViews;
	VkDescriptorSetLayout samplerSetLayout;
	std::vector<VkDescriptorSet> samplerDescriptorSets;
	VkDescriptorPool samplerDescriptorPool;
	VkSampler textureSampler;
private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	// TODO: transfer all this to Texture....cpp
	int createTexture(std::string filename, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool);
	int createTextureImage(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool);
	void createTextureSampler();
	int createTextureDescriptor(VkImageView textureImage);


	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
};

