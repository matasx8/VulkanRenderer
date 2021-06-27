#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "vulkan.h"
#include "Texture.h"
#include "Material.h"
#include "Pipeline.h"
// Contains data of models and components of the scene
// TODO: read scene data from a .MRscene file

class Scene
{
public:
	Scene();
	Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	// Add and load model to currently used scene
	// Will create a new pipeline if needed
	void addModel(std::string fileName, Material material);

	std::vector<Model>& getModels(); // TODO: probably dont pass by ref
	Texture& getTexture(int index) { return Textures[index]; }
	// Returns pipeline, if index not found will return default one(index 0) 
	Pipeline getPipeline(int index) const;

	void onFrameEnded();
	void CleanUp(VkDevice logicalDevice);

	//void AddCamera();
	//void AddLight();
	// TODO: async model upload
	//void AddModels(paths);

	~Scene();
private:
	std::vector<Model> Models;
	std::vector<Texture> Textures;
	std::vector<Pipeline> Pipelines;

	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	void addModelInitial(std::string path);
	void updateModelPipesFrom(int index);
	// Uses material to find out if we need to create a new pipeline.
    // If yes, then creates a new one and appends to Pipelines.
    // If no, finds out which pipeline do we need to reuse.
	// returns the index of the pipeline
	int setupPipeline(const Material& material);
};

