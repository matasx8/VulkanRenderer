#pragma once
#include "Model.h"
#include <mutex>;

class VulkanRenderer;

class ModelManager
{
public:
	ModelManager(VulkanRenderer& gfxEngine);
	ModelManager(VulkanRenderer& gfxEngine, bool isThreadedImport);

	void LoadDefaultModels();


private:

	// Loads models (Creates only IB and VB)
	void LoadModelsThreaded(std::vector<std::string>* paths);
	void LoadModelsNonThreaded(std::vector<std::string>* paths);
	void LoadModel(std::string& path);

	std::mutex m_Mutex;
	std::vector<Model> m_Models;
	// for now create thread pool on demand
	bool m_ThreadedImport;

	VulkanRenderer& m_GfxEngine;
};

