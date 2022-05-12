#pragma once
#include "Model.h"
#include <mutex>;

class VulkanRenderer;

class ModelManager
{
public:
	ModelManager(VulkanRenderer& gfxEngine);
	ModelManager(VulkanRenderer& gfxEngine, bool isThreadedImport);

	// ~0u handle will mean deselect all
	void SelectModels(ModelHandle handle);
	const std::set<Model*>& GetSelectedModels();

	void LoadDefaultModels();

	void Duplicate(const Model& model, bool isInstanced);
	void DuplicateWithMaterial(const Model& model, bool isInstanced, uint32_t newMaterialID);

	void BindMesh(const Mesh& mesh);

	inline size_t Size() { return m_Models.size(); };

	Model& operator[](size_t idx);

	void CleanUp();

private:

	// Loads models (Creates only IB and VB)
	void LoadModelsThreaded(std::vector<std::string>* paths);
	void LoadModelsNonThreaded(std::vector<std::string>* paths);
	void LoadModel(std::string& path);

	Model* GetModel(ModelHandle);

	std::mutex m_Mutex;
	std::vector<Model> m_Models;
	// I'm not sure if that's a good idea. Will have to clear this after each change to m_Models that 
	// might cause allocation
	std::set<Model*> m_SelectedModelPtrs;

	// for now create thread pool on demand
	bool m_ThreadedImport;

	VulkanRenderer& m_GfxEngine;
};

