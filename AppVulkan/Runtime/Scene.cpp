#include "Scene.h"

Scene::Scene()
{
}

Scene::Scene(VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, size_t swapchainImageCount, VkExtent2D extent, VkSampleCountFlagBits msaaSamples)
    :graphicsQueue(graphicsQueue), graphicsCommandPool(graphicsCommandPool), physicalDevice(physicalDevice), logicalDevice(logicalDevice), camera(Camera())
    , swapchainImageCount(swapchainImageCount), extent(extent)
{
    // Fallback Texture
    Texture tex;
    tex.createTexture("plain.png", graphicsQueue, graphicsCommandPool, physicalDevice, logicalDevice);
    Textures.push_back(tex);

    camera.setMSAA(msaaSamples);

    //TODO put in initilizer list
    viewProjection.projection = glm::perspective(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 1000.0f);
    viewProjection.view = glm::lookAt(glm::vec3(0.0f, 30.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    viewProjection.projection[1][1] *= -1;//invert matrix
}

void Scene::addModel(std::string fileName, Material material, VkRenderPass renderPass)
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

    auto oldTexturesSize = Textures.size();

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

    uint32_t texturesFrom = (oldTexturesSize != Textures.size()) ? oldTexturesSize : 0;
    if (Textures[0].descriptorSet == 0) texturesFrom = 0;

    // use material to find out if we need to create a new pipeline
    // if yes, then create one and append to pipelines. Also insert into model vector
    // if no, find out which pipeline do we need to reuse.
    meshModel.setPipelineIndex(setupPipeline(material, Textures, texturesFrom, extent, renderPass));
    insertModel(meshModel);
}

void Scene::addLight()
{
    Lights.emplace_back(glm::vec4(0.0f, 100.0f, 0.0f, 0.0f));
}


std::vector<Model>& Scene::getModels()
{
    return Models;
}

void Scene::updateModelPipesFrom(int index)
{
    for (auto& model : Models)
    {
        if (model.getPipelineIndex() >= index)
        {
            model.updatePipelineIndex();
        }
    }
}

void Scene::updateModelMatrixIndices(int index)
{
    throw std::runtime_error("Not implemented yet!");
}

void Scene::insertModel(Model& model)
{
    if (Models.size() == 0 || Models.back().getPipelineIndex() <= model.getPipelineIndex())
    {
        model.setModelMatrix(getNewModelMatrixIndex());
        Models.push_back(model);
        return;
    }
    // TODO medium annoying: binary search
    // currently O(n^2) and nobody likes that :)
    for (auto it = Models.begin(); it != Models.end();)
    {
        if ((*it).getPipelineIndex() >= model.getPipelineIndex())
        {
            model.setModelMatrix(getNewModelMatrixIndex());
            Models.insert(it, model);
            return;
        }
    }
}

size_t Scene::getNewModelMatrixIndex()
{
    ModelMatrices.emplace_back(1.0f);
    return ModelMatrices.size() - 1;
}

int Scene::setupPipeline(Material& material, std::vector<Texture>& Textures, uint32_t texturesFrom
    , VkExtent2D extent, VkRenderPass renderPass)
{
    for (int i = 0; i < Pipelines.size(); i++)
    {
        if (Pipelines[i].isMaterialCompatible(material))
        {
            // create texture descriptor sets.. Temporary implementation, please remake to dynamic buffers
            for (size_t j = texturesFrom; j < Textures.size(); j++)
            {
                Textures[j].descriptorSet = Pipelines[i].createTextureDescriptorSet(Textures[j], logicalDevice);
            }
            return i; // we can reuse this pipeline, hooray
        }
    }
    // we did not find any compatible pipelines, let's create a new one
    Device device = { physicalDevice, logicalDevice };//TODO
    Pipeline newPipe = Pipeline(material, device, &camera, swapchainImageCount);
    newPipe.createPipeline(extent, renderPass);
    Pipelines.push_back(newPipe);

    for (size_t i = texturesFrom; i < Textures.size(); i++)
    {
        Textures[i].descriptorSet = Pipelines.back().createTextureDescriptorSet(Textures[i], logicalDevice);
    }

    return Pipelines.size() - 1;
}



Pipeline Scene::getPipeline(int index) const
{
    if (index >= Pipelines.size())
        return Pipelines[0];

    return Pipelines[index];
}

void Scene::updateScene(size_t index)
{
    //Lights[0].randomize();
    // do the transformations here or before calling this func
    viewProjection.view = camera.calculateViewMatrix();

    for (auto& pipe : Pipelines)
        pipe.update(index);
}

void Scene::onFrameEnded()
{
    int i = 0;
    for (auto it = Pipelines.begin(); it != Pipelines.end(); it++)
    {
        i++;
        if ((*it).wasUsedThisFrame())
        {
            Debug::LogMsg("a pipeline was thrown out because it has not been used\0");
            Pipelines.erase(it);
            // update the models from the index because we need to update their indices of the pipeline
            updateModelPipesFrom(i);
            continue;
        }
    }
}

void Scene::CleanUp(VkDevice logicalDevice)
{
    for (auto& pipeline : Pipelines)
        pipeline.CleanUp(logicalDevice);

    for (auto& model : Models)
    {
        model.destroyMeshModel();
    }

    for (auto& texture : Textures)
    {
        texture.DestroyTexture(logicalDevice);
    }

}

void Scene::cameraKeyControl(bool* keys, float dt)
{
    camera.keyControl(keys, dt);
}

void Scene::cameraMouseControl(float xChange, float yChange)
{
    camera.mouseControl(xChange, yChange);
}


