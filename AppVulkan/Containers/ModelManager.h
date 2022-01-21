#pragma once
#include "Model.h"
#include <mutex>;

class ModelManager
{
public:
	ModelManager();
	ModelManager(bool isThreadedImport);

	void LoadDefaultModels();


private:

	void LoadModelsThreaded(std::vector<std::string>* paths);
	void LoadModelsNonThreaded(std::vector<std::string>* paths);
	void LoadModel(std::string& path);

	std::mutex m_Mutex;
	std::vector<Model> m_Models;
	// for now create threads on demand
	bool m_ThreadedImport;
};

