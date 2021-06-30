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
    viewProjection.projection = glm::perspective(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 500.0f);
    viewProjection.view = glm::lookAt(glm::vec3(0.0f, 30.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    viewProjection.projection[1][1] *= -1;//invert matrix

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSetLayout();
    createDescriptorSets();
    
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
    //temporary i hope
        int texID = meshModel.getMesh(0)->getTexId();

        material.texture = Textures[texID];
    // use material to find out if we need to create a new pipeline
    // if yes, then create one and append to pipelines. Also insert into model vector
    // if no, find out which pipeline do we need to reuse.
    meshModel.setPipelineIndex(setupPipeline(material, extent, renderPass));
    insertModel(meshModel);
}

void Scene::createUniformBuffers()
{
    // ViewProjection buffer size
    VkDeviceSize vpBufferSize = sizeof(UboViewProjection);
    VkDeviceSize lightsBufferSize = Light::getDataSize();
    VkDeviceSize cameraBufferSize = sizeof(glm::vec3);

    // model buffer size
    //VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

    // one uniform buffer for each image (and by extension, comman buffer)
    vpUniformBuffer.resize(swapchainImageCount);
    vpUniformBufferMemory.resize(swapchainImageCount);
    lightsUniformBuffer.resize(swapchainImageCount);
    lightsUniformBufferMemory.resize(swapchainImageCount);
    cameraUniformBuffer.resize(swapchainImageCount);
    cameraUniformBufferMemory.resize(swapchainImageCount);

    //create uniform buffers
    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        createBuffer(physicalDevice, logicalDevice, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);

        createBuffer(physicalDevice, logicalDevice, lightsBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &lightsUniformBuffer[i], &lightsUniformBufferMemory[i]);

        createBuffer(physicalDevice, logicalDevice, cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraUniformBuffer[i], &cameraUniformBufferMemory[i]);
    }
}

void Scene::createDescriptorPool()
{
    //type of descriptors
//view projection pool
    VkDescriptorPoolSize vpPoolSize = {};
    vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size());

    VkDescriptorPoolSize lightsPoolSize = {};
    lightsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightsPoolSize.descriptorCount = static_cast<uint32_t>(lightsUniformBuffer.size());

    VkDescriptorPoolSize cameraPoolSize = {};
    cameraPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraPoolSize.descriptorCount = static_cast<uint32_t>(cameraUniformBuffer.size());

    /* VkDescriptorPoolSize modelPoolSize = {};
     modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
     modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());*/

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize, lightsPoolSize, cameraPoolSize };

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(swapchainImageCount); //max number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()); // amount of pool sizes being passed
    poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

    // create descriptor pool
    VkResult result = vkCreateDescriptorPool(logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor pool!");
    }
}

void Scene::createDescriptorSetLayout()
{
    //uniform values descripor set layout
// uboViewProjection binding info
    VkDescriptorSetLayoutBinding vpLayoutBinding = {};
    vpLayoutBinding.binding = 0; // binding point in shader
    vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor (uniform, dynamic uniform)
    vpLayoutBinding.descriptorCount = 1; // number of descriptors for bidning
    vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // shader stage to bind to
    vpLayoutBinding.pImmutableSamplers = nullptr; // for texture- can make the sampler immutable, the imageview it samples from can still be changed

    VkDescriptorSetLayoutBinding lightsLayoutBinding = {};
    lightsLayoutBinding.binding = 1;
    lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightsLayoutBinding.descriptorCount = 1;
    lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    lightsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding cameraLayoutBinding = {};
    cameraLayoutBinding.binding = 2;
    cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    cameraLayoutBinding.pImmutableSamplers = nullptr;


    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding, lightsLayoutBinding, cameraLayoutBinding };

    // create descriptor set layout with given bidnings
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size()); // number of binding infos
    layoutCreateInfo.pBindings = layoutBindings.data(); //array of binding infos

    //create descriptor set layout
    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

void Scene::createDescriptorSets()
{
    //resize descriptor set list so one for every buffer
    descriptorSets.resize(swapchainImageCount);

    std::vector<VkDescriptorSetLayout> setLayouts(swapchainImageCount, descriptorSetLayout);
    // descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = descriptorPool; //pool to allocate descriptor set from
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImageCount); // number of sets to allocate
    setAllocInfo.pSetLayouts = setLayouts.data(); // layouts to use to allocate sets (1:1 relationship)

    //allocate descriptor sets(multiple)
    VkResult result = vkAllocateDescriptorSets(logicalDevice, &setAllocInfo, descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    //update all of descriptor set buffer bindings
    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        // buffer info and data offset info
        VkDescriptorBufferInfo vpBufferInfo = {};
        vpBufferInfo.buffer = vpUniformBuffer[i]; // buffer to get data from
        vpBufferInfo.offset = 0; // position of start of data
        vpBufferInfo.range = sizeof(UboViewProjection); // sizeof data

        //data about connection between binding and buffer
        VkWriteDescriptorSet vpSetWrite = {};
        vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        vpSetWrite.dstBinding = 0; // binding to update
        vpSetWrite.dstArrayElement = 0; // index in array to update
        vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        vpSetWrite.descriptorCount = 1; // amount to update
        vpSetWrite.pBufferInfo = &vpBufferInfo; // info about buffer data to bind
//-----
        VkDescriptorBufferInfo lightsBufferInfo = {};
        lightsBufferInfo.buffer = lightsUniformBuffer[i]; // buffer to get data from
        lightsBufferInfo.offset = 0; // position of start of data
        lightsBufferInfo.range = sizeof(Light); // sizeof data

        //data about connection between binding and buffer
        VkWriteDescriptorSet lightsSetWrite = {};
        lightsSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightsSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        lightsSetWrite.dstBinding = 1; // binding to update
        lightsSetWrite.dstArrayElement = 0; // index in array to update
        lightsSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        lightsSetWrite.descriptorCount = 1; // amount to update
        lightsSetWrite.pBufferInfo = &lightsBufferInfo; // info about buffer data to bind
        //-------------
        VkDescriptorBufferInfo cameraBufferInfo = {};
        cameraBufferInfo.buffer = cameraUniformBuffer[i]; // buffer to get data from
        cameraBufferInfo.offset = 0; // position of start of data
        cameraBufferInfo.range = sizeof(glm::vec3); // sizeof data

        //data about connection between binding and buffer
        VkWriteDescriptorSet cameraSetWrite = {};
        cameraSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        cameraSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        cameraSetWrite.dstBinding = 2; // binding to update
        cameraSetWrite.dstArrayElement = 0; // index in array to update
        cameraSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        cameraSetWrite.descriptorCount = 1; // amount to update
        cameraSetWrite.pBufferInfo = &cameraBufferInfo; // info about buffer data to bind

        ////model descriptor
        //// model buffer binding info
        //VkDescriptorBufferInfo modelBufferInfo = {};
        //modelBufferInfo.buffer = modelDUniformBuffer[i];
        //modelBufferInfo.offset = 0;
        //modelBufferInfo.range = modelUniformAlignment;

       /* VkWriteDescriptorSet modelSetWrite = {};
        modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        modelSetWrite.dstSet = descriptorSets[i];
        modelSetWrite.dstBinding = 1;
        modelSetWrite.dstArrayElement = 0;
        modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        modelSetWrite.descriptorCount = 1;
        modelSetWrite.pBufferInfo = &modelBufferInfo;*/

        // list of descriptor set writes
        std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite, lightsSetWrite, cameraSetWrite };

        //update the descriptor sets with new buffer/binding info
        vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

std::vector<Model>& Scene::getModels()
{
    return Models;
}

Scene::~Scene()
{
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

void Scene::insertModel(Model& model)
{
    if (Models.size() == 0)
        Models.push_back(model);
    // TODO medium annoying: binary search
    // currently O(n^2) and nobody likes that :)
    for (auto it = Models.begin(); it != Models.end(); it++)
    {
        if ((*it).getPipelineIndex() >= model.getPipelineIndex())
        {
            Models.insert(it, model);
        }
    }
}

int Scene::setupPipeline(Material& material, VkExtent2D extent, VkRenderPass renderPass)
{
    for (int i = 0; i < Pipelines.size(); i++)
    {
        if (Pipelines[i].isMaterialCompatible(material))
            return i; // we can reuse this pipeline, hooray
    }
    // we did not find any compatible pipelines, let's create a new one
    Device device = { physicalDevice, logicalDevice };//TODO
    Pipeline newPipe = Pipeline(material, device, &camera);
    newPipe.createPipeline(extent, renderPass, descriptorSetLayout);
    Pipelines.push_back(newPipe);
}



Pipeline Scene::getPipeline(int index) const
{
    if (index >= Pipelines.size())
        return Pipelines[0];

    return Pipelines[index];
}

void Scene::updateUniformBuffers(size_t index)
{
    //do the transformations here or before calling this func
    viewProjection.view = camera.calculateViewMatrix();
    Debug::Log(camera);
    // copy vp data
    void* data;
    vkMapMemory(logicalDevice, vpUniformBufferMemory[index], 0, sizeof(UboViewProjection), 0, &data);
    memcpy(data, &viewProjection, sizeof(UboViewProjection));
    vkUnmapMemory(logicalDevice, vpUniformBufferMemory[index]);

    data = alloca(Light::getDataSize());
    auto light = Light();
    vkMapMemory(logicalDevice, lightsUniformBufferMemory[index], 0, Light::getDataSize(), 0, &data);
    light.getData(data);
    vkUnmapMemory(logicalDevice, lightsUniformBufferMemory[index]);

    vkMapMemory(logicalDevice, cameraUniformBufferMemory[index], 0, sizeof(glm::vec3), 0, &data);
    memcpy(data, &camera.getCameraPosition(), sizeof(glm::vec3));//hmm
    vkUnmapMemory(logicalDevice, cameraUniformBufferMemory[index]);
    /*
    //copy model data
    //this prevents always reallocating the buffer, because we allocate it once
    //and then update the buffer
    //we can do this because of MAX_OBJECTS
    for (size_t i = 0; i < meshList.size(); i++)
    {
        Model* thisModel = (Model*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment)); // moving the pointer in the buffer
        *thisModel = meshList[i].getModel();
    }

    vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[index], 0, modelUniformAlignment * meshList.size(), 0, &data);
    memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
    vkUnmapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[index]);*/
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
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyBuffer(logicalDevice, vpUniformBuffer[i], nullptr);
        vkFreeMemory(logicalDevice, vpUniformBufferMemory[i], nullptr);
        vkDestroyBuffer(logicalDevice, lightsUniformBuffer[i], nullptr);
        vkFreeMemory(logicalDevice, lightsUniformBufferMemory[i], nullptr);
        vkDestroyBuffer(logicalDevice, cameraUniformBuffer[i], nullptr);
        vkFreeMemory(logicalDevice, cameraUniformBufferMemory[i], nullptr);
        // vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
        // vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
    }

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


