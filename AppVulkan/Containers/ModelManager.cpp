#include "ModelManager.h"
#include "OSUtilities.h"
#include "Debug.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <thread-pool/thread_pool.hpp>

ModelManager::ModelManager()
	:m_Models(), m_ThreadedImport(true)
{
}

ModelManager::ModelManager(bool isThreadedImport)
	: m_Models(), m_ThreadedImport(isThreadedImport)
{
}

void ModelManager::LoadDefaultModels()
{
    auto paths = OS::GetAllFileNamesInDirectory("Models/DefaultModels");
    if (paths == nullptr)
    {
        Debug::LogMsg("Failed to get any paths in Models/DefaultModels\n");
        throw std::runtime_error("Failed to get any paths in Models/DefaultModels");
    }

    if (m_ThreadedImport)
        LoadModelsThreaded(paths);
    else
        LoadModelsNonThreaded(paths);

    delete paths;
}

void ModelManager::LoadModelsThreaded(std::vector<std::string>* paths)
{
    thread_pool tp;
    tp.parallelize_loop(0, paths->size(), LoadModel);
}

void ModelManager::LoadModelsNonThreaded(std::vector<std::string>* paths)
{
}

void ModelManager::LoadModel(std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + path + ")");
    }

    //!HERE:
    // current plan is to move models to model manager, this will be the container for our models
    // will make stuff more loosely coupled and make development easier.
    // make vulkanrenderer behave similarly like gfxdevice so we dont have to keep api logic in model class
    // now just load the mesh, dont want to deal withmaterial loading.
    std::vector<Mesh> modelMeshes = Model::LoadNode(physicalDevice, logicalDevice, graphicsQueue, graphicsCommandPool,
        scene->mRootNode, scene, matToTex);

    Model meshModel(modelMeshes, material.IsInstanced());
    std::unique_lock<std::mutex> lock(m_Mutex);
}
