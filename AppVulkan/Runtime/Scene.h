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
#include "Containers/RenderPassManager.h"

class Scene
{
public:
	Scene();
	Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t swapchainImageCount, VkExtent2D extent, VkSampleCountFlagBits msaaSamples, DescriptorPool* descriptorPool);
	
	// Add and load model to currently used scene
	// Will create a new pipeline if needed
	// returns handle for the model added
	ModelHandle AddModel(std::string fileName, Material material);
	void addLight();

	Model& GetModel(ModelHandle handle);
	// The original Model must exist during runtime, the duplicates will be removed with the original
	Model GetModelAndDuplicate(ModelHandle handle, bool instanced = false);
	void DuplicateModelInstanced(ModelHandle handle, int numInstances);

	std::vector<Model>& getModels(); // TODO: probably dont pass by ref
	Texture& getTexture(int index) { return Textures[index]; }
	// Returns pipeline, if index not found will return default one(index 0) 
	Pipeline& getPipeline(int index);
	VkDescriptorSet getTextureDescriptorSet(size_t index) const { return Textures[index].descriptorSet; }
	void* getViewProjectionPtr() const { return (void*)(&viewProjection); }
	Light& getLight(size_t index) { return Lights[index]; }
	Camera& getCamera() { return camera; }

	// Updates components (eg. Lights, Camera..)
	void updateScene(size_t index);
	// Duplicates Model specified by handle
	// If duplication failed, returns the same hadle to indicate failure
	ModelHandle DuplicateModel(ModelHandle handle, bool instanced = false);


	void onFrameEnded();
	void CleanUp(VkDevice logicalDevice);

	// -- temporary Input
	void cameraKeyControl(bool* keys, float dt);
	void cameraMouseControl(float xChange, float yChange);
	// -- temporary Input

private:
	
	friend class VulkanRenderer;

	ViewProjectionMatrix viewProjection;
	std::vector<Model> Models;
	// trying out dots
	std::vector<Texture> Textures;
	std::vector<Pipeline> Pipelines;
	std::vector<Light> Lights;
	Camera camera;

	VkRenderPass tmp_RenderPass; // this will not be here soon
	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	DescriptorPool* m_DescriptorPool;
	VkExtent2D extent;
	size_t swapchainImageCount;



	void updateModelPipesFrom(int index);
	void updateModelMatrixIndices(int index);

	// Inserts model so Models remain sorted by pipeline
	// returns index where the model was placed
	uint32_t insertModel(Model& model);
	// I've no idea why this needs renderPass but I will change this for sure later
	ModelHandle AddModel(std::string fileName, Material material, VkRenderPass renderPass);

	size_t getNewModelMatrixIndex();
	// Uses material to find out if we need to create a new pipeline.
    // If yes, then creates a new one and appends to Pipelines.
    // If no, finds out which pipeline do we need to reuse.
	// returns the index of the pipeline
	int setupPipeline(Material& material, std::vector<Texture>& Textures, uint32_t texturesFrom
		, VkExtent2D extent, VkRenderPass renderPass);
};

