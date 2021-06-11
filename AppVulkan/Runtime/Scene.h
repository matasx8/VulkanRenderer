#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "vulkan.h"
#include "Texture.h"
// Contains data of models and components of the scene
// TODO: read scene data from a .MRscene file

class Scene
{
public:
	Scene();
	Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	// Add and load model to currently used scene
	// Will create a new pipeline if needed
	void AddModel(std::string fileName);

	std::vector<Model>& GetModels();
	Texture& getTexture(int index) { return Textures[index]; }

	void CleanUp(VkDevice logicalDevice);

	//void AddCamera();
	//void AddLight();
	// TODO: async model upload
	//void AddModels(paths);

	~Scene();
private:
	std::vector<Model> Models;
	std::vector<Texture> Textures;
	

	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	void AddModelInitial(std::string path);
};

