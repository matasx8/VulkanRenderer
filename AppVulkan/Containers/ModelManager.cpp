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
    timer tmr;
    tmr.start();
    auto paths = OS::GetAllFileNamesInDirectory("Models\\DefaultModels");
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

    tmr.stop();
    Debug::LogMsg("Uploading DefaultModels took: ");
    Debug::LogMsg(std::to_string(tmr.ms()).c_str());
    Debug::LogMsg(" ms.\n");
}

void ModelManager::Duplicate(const Model& model, bool isInstanced)
{
    m_Models.emplace_back(model.Duplicate(isInstanced));
}

void ModelManager::BindMesh(const Mesh& mesh)
{
    m_GfxEngine.BindMesh(mesh);
}

void ModelManager::LoadModelsThreaded(std::vector<std::string>* paths)
{
    // if nr of models is low then it might be slower or have not much benefit over
    // the single threaded impl.
    // one of the reasons is because of the very simple implementation of this and low level of paralelization
    auto loop = [this, paths](int st, int end) {
        for (int i = st; i < end; i++)
            LoadModel((*paths)[i]);
    };
    thread_pool tp;
    tp.parallelize_loop(0, paths->size(), loop);
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
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + path + ")");
    }

    std::vector<Mesh> modelMeshes;

    std::unique_lock<std::mutex> lock(m_Mutex);
    m_GfxEngine.LoadNode(modelMeshes, scene->mRootNode, scene);

    Model meshModel(modelMeshes, false);
    m_Models.push_back(meshModel);
}

Model& ModelManager::operator[](size_t idx)
{
    return m_Models[idx];
}
