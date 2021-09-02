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
	Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t swapchainImageCount, VkExtent2D extent, VkSampleCountFlagBits msaaSamples, DescriptorPool* descriptorPool);
	// Add and load model to currently used scene
	// Will create a new pipeline if needed
	void addModel(std::string fileName, Material material, VkRenderPass renderPass);
	void addLight();

	std::vector<Model>& getModels(); // TODO: probably dont pass by ref
		std::vector<glm::mat4>* getModelMatrices() { return &ModelMatrices; }
	Texture& getTexture(int index) { return Textures[index]; }
	// Returns pipeline, if index not found will return default one(index 0) 
	Pipeline getPipeline(int index) const;
	VkDescriptorSet getTextureDescriptorSet(size_t index) const { return Textures[index].descriptorSet; }
	void* getViewProjectionPtr() const { return (void*)(&viewProjection); }
	Light& getLight(size_t index) { return Lights[index]; }
	Camera& getCamera() { return camera; }

	// Updates components (eg. Lights, Camera..)
	void updateScene(size_t index);


	void onFrameEnded();
	void CleanUp(VkDevice logicalDevice);

	// -- temporary Input
	void cameraKeyControl(bool* keys, float dt);
	void cameraMouseControl(float xChange, float yChange);
	// -- temporary Input

private:
	ViewProjectionMatrix viewProjection;
	std::vector<Model> Models;
		// trying out dots
		std::vector<glm::mat4> ModelMatrices;
	std::vector<Texture> Textures;
	std::vector<Pipeline> Pipelines;
	std::vector<Light> Lights;
	Camera camera;


	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	DescriptorPool* m_DescriptorPool;
	VkExtent2D extent;
	size_t swapchainImageCount;


	void updateModelPipesFrom(int index);
	void updateModelMatrixIndices(int index);

	void insertModel(Model& model);

	size_t getNewModelMatrixIndex();
	// Uses material to find out if we need to create a new pipeline.
    // If yes, then creates a new one and appends to Pipelines.
    // If no, finds out which pipeline do we need to reuse.
	// returns the index of the pipeline
	int setupPipeline(Material& material, std::vector<Texture>& Textures, uint32_t texturesFrom
		, VkExtent2D extent, VkRenderPass renderPass);
};

