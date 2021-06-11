#include "Scene.h"

Scene::Scene()
{
}

Scene::Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
    :graphicsQueue(graphicsQueue), graphicsCommandPool(graphicsCommandPool), physicalDevice(physicalDevice), logicalDevice(logicalDevice)
{
    // Fallback Texture
    Texture tex;
    tex.createTexture("plain.png", graphicsQueue, graphicsCommandPool, physicalDevice, logicalDevice);
    Textures.push_back(tex);

    // Initial model
    AddModelInitial("Models/12140_Skull_v3_L2.obj");
}

void Scene::AddModel(std::string fileName)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + fileName + ")");
    }

    //get vector of all materials with 1:1 ID placement
    std::vector<std::string> textureNames = Model::LoadMaterials(scene);

    // conversion from the materials list IDs to our Descriptor Arrays IDs
    std::vector<int> matToTex(textureNames.size());

    // loop over textureNames and create textures for them
    for (size_t i = 0; i < textureNames.size(); i++)
    {//if material had no texture, set 0 to indicate no texture, texture 0 will be reserved for a default texture
        if (textureNames[i].empty())
        {
            matToTex[i] = 0;
        }
        else
        {// otherwise, create texture and set value to index of new texture
            Texture texture;
            texture.createTexture(textureNames[i], graphicsQueue, graphicsCommandPool, physicalDevice, logicalDevice);
            matToTex[i] = Textures.size();
            Textures.push_back(texture);
        }
    }

    // load in all our meshes
    std::vector<Mesh> modelMeshes = Model::LoadNode(physicalDevice, logicalDevice, graphicsQueue, graphicsCommandPool,
        scene->mRootNode, scene, matToTex);

    Model meshModel = Model(modelMeshes);
    Models.push_back(meshModel);
}

void Scene::AddModelInitial(std::string path)
{

    // import model "scene"
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + path + ")");
    }

    //get vector of all materials with 1:1 ID placement
    std::vector<std::string> textureNames = Model::LoadMaterials(scene);

    // conversion from the materials list IDs to our Descriptor Arrays IDs
    std::vector<int> matToTex(textureNames.size());

    // loop over textureNames and create textures for them
    for (size_t i = 0; i < textureNames.size(); i++)
    {//if material had no texture, set 0 to indicate no texture, texture 0 will be reserved for a default texture
        if (textureNames[i].empty())
        {
            matToTex[i] = 0;
        }
        else
        {// otherwise, create texture and set value to index of new texture
            Texture texture;
            texture.createTexture(textureNames[i], graphicsQueue, graphicsCommandPool, physicalDevice, logicalDevice);
            matToTex[i] = Textures.size();
            Textures.push_back(texture);
        }
    }

    // load in all our meshes
    std::vector<Mesh> modelMeshes = Model::LoadNode(physicalDevice, logicalDevice, graphicsQueue, graphicsCommandPool,
        scene->mRootNode, scene, matToTex);

    Model meshModel = Model(modelMeshes);
    Models.push_back(meshModel);
}

std::vector<Model>& Scene::GetModels()
{
	// TODO: insert return statement here
    return Models;
}

Scene::~Scene()
{
}



void Scene::CleanUp(VkDevice logicalDevice)
{
    for (auto& model : Models)
    {
        model.destroyMeshModel();
    }

    for (auto& texture : Textures)
    {
        texture.DestroyTexture(logicalDevice);
    }

}


