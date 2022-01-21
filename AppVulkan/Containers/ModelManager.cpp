#include "VulkanRenderer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <thread-pool/thread_pool.hpp>

ModelManager::ModelManager(VulkanRenderer& gfxEngine)
    :m_Mutex(), m_Models(), m_ThreadedImport(true), m_GfxEngine(gfxEngine)
{
}

ModelManager::ModelManager(VulkanRenderer& gfxEngine, bool isThreadedImport)
	: m_Mutex(), m_Models(), m_ThreadedImport(isThreadedImport), m_GfxEngine(gfxEngine)
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
    for (auto& path : *paths)
    {
        LoadModel(path);
    }
}

void ModelManager::LoadModel(std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + path + ")");
    }

    std::vector<Mesh> modelMeshes;
    m_GfxEngine.LoadNode(modelMeshes, scene->mRootNode, scene);

    Model meshModel(modelMeshes, false);
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Models.push_back(meshModel);
}
