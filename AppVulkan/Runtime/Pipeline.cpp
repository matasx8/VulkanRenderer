#include "Pipeline.h"
#include <stdexcept>

Pipeline::Pipeline(Device device, Camera* camera, size_t swapchainImageCount, DescriptorPool* descriptorPool)
    :device(device), usedThisFrame(false), swapchainImageCount(swapchainImageCount), pushConstantRange({}), camera(camera), m_DescriptorPool(descriptorPool)
{
    if (camera == nullptr)
        throw std::runtime_error("Pointer to camera was null!");
    if(descriptorPool == nullptr)
        throw std::runtime_error("Pointer to Descriptor Pool was null!");
}

Pipeline::Pipeline(Material material, Device device, Camera* camera, size_t swapchainImageCount, DescriptorPool* descriptorPool)
    : device(device), usedThisFrame(false), material(material),
    swapchainImageCount(swapchainImageCount), pushConstantRange({}), camera(camera), m_DescriptorPool(descriptorPool)
{
    if (camera == nullptr)
        throw std::runtime_error("Pointer to camera was null!");
    if (descriptorPool == nullptr)
        throw std::runtime_error("Pointer to Descriptor Pool was null!");
}

void Pipeline::createPipeline(VkExtent2D extent, VkRenderPass renderPass)
{
    //if(material is uninitialized)
    // use default TODO
    VkPipelineShaderStageCreateInfo shaderStages[2];
    createPipelineShaderStageCreateInfo(shaderStages[0], material.getVertexShader(), VK_SHADER_STAGE_VERTEX_BIT);
    createPipelineShaderStageCreateInfo(shaderStages[1], material.getFragmentShader(), VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions(material.IsInstanced() ? 2 : 1);
    VkVertexInputBindingDescription bindingDescription = {};
    createVertexInputBindingDescription(bindingDescription);
    bindingDescriptions[0] = bindingDescription;
    if (material.IsInstanced())
    {
        VkVertexInputBindingDescription instancedBindingDescription = {};
        createVertexInputInstancedBindingDescription(instancedBindingDescription);
        bindingDescriptions[1] = instancedBindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(material.IsInstanced() ? 7 : 3);
    createVertexInputAttributeDescription(attributeDescriptions[0], 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
    createVertexInputAttributeDescription(attributeDescriptions[1], 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm));
    createVertexInputAttributeDescription(attributeDescriptions[2], 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex));
    if (material.IsInstanced())
    {
        createVertexInputAttributeDescription(attributeDescriptions[3], 1, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0);
        createVertexInputAttributeDescription(attributeDescriptions[4], 1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, 16);
        createVertexInputAttributeDescription(attributeDescriptions[5], 1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, 32);
        createVertexInputAttributeDescription(attributeDescriptions[6], 1, 6, VK_FORMAT_R32G32B32A32_SFLOAT, 48);
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    createPipelineVertexInputStateCreateInfo(vertexInputCreateInfo, bindingDescriptions.data(), attributeDescriptions.data());

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    createPipelineInputAssemblyStateCreateInfo(inputAssembly);

    VkViewport viewport = {};
    createViewport(viewport, extent.width, extent.height);
    VkRect2D scissor = {};
    createScissor(scissor, extent);
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};;
    createPipelineViewportStateCreateInfo(viewportStateCreateInfo, &viewport, &scissor);

    // TODO dynamic states
    
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    createPipelineRasterizationStateCreateInfo(rasterizerCreateInfo);

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
    createMSAAStateCreateInfo(multisamplingCreateInfo, camera->getMSAA());

    VkPipelineColorBlendAttachmentState colorState = {};
    createPipelineColorBlendAttachmentState(colorState);
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
    createPipelineColorBlendStateCreateInfo(colorBlendingCreateInfo, colorState);

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    createDepthStencilCreateInfo(depthStencilCreateInfo);

    //TODO: cover flags
    // layout shows the layout for the descriptor sets. If it can be reused, then we don't have to rebind
    // descriptor sets for next pipeline.
    // 1 descriptor set for 1 ubo
    uint32_t shaderFlags = material.getShaderFlags();
    if (material.getUboCount() > 0)
    {
        // TODO: make class for dslayout or something. So we can efficiently reuse them
        // still making one layout for this pipeline
        createDescriptorSetLayout(material.getUboCount());
        // create buffers uboCount x swapchainImageCount
        createUniformBuffers(material.getDataSizes(), material.getUboCount());

        createDescriptorSets(material.getDataSizes().data());
    }
    else
        assert(false);// not implemented yet

    //TODO IMPORTANT
        createTextureSampler(device.logicalDevice);
        createTextureSamplerSetLayout(device.logicalDevice);
        // the default one has 1 texture, will do for now
        //Texture tex = material.textures[0];
        // keep track of index
        //createTextureDescriptor(tex, 0, device.logicalDevice);
        // must pass descriptorsetlayout from scene wtf? this is definitely not good
        //!HERE from shader create info..
        std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout, getTextureDescriptorSetLayout() };
    //TODO: cover else branch
    if(shaderFlags & kUseModelMatrixForPushConstant)
        createPushConstantRange();

    createPipelineLayout(descriptorSetLayouts.data(), descriptorSetLayouts.size(), pushConstantRange.size);

    CreatePipeline(shaderStages, &vertexInputCreateInfo, &inputAssembly, &viewportStateCreateInfo,
        NULL, &rasterizerCreateInfo, &multisamplingCreateInfo, &colorBlendingCreateInfo, &depthStencilCreateInfo,
        pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1,
        VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT, device.logicalDevice); // TODO medium, pipeline cache or at least derivatives

    vkDestroyShaderModule(device.logicalDevice, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(device.logicalDevice, shaderStages[1].module, nullptr);
}

void Pipeline::CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
    VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
    VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
    VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
    VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout, 
    VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex, 
    VkPipelineCreateFlags flags, VkDevice device)
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = inputAssembly;
    pipelineCreateInfo.pViewportState = viewportStateCreateInfo;
    pipelineCreateInfo.pDynamicState = dynamicState;
    pipelineCreateInfo.pRasterizationState = rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = multisamplingCreateInfo;
    pipelineCreateInfo.pColorBlendState = colourBlendingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = depthStencilCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.flags = flags;
    pipelineCreateInfo.pNext = nullptr;
   

    //pipeline derivatives - can create multiple pipelines that derive from one another for optimistions
    pipelineCreateInfo.basePipelineHandle = basePipelineHandle;
    pipelineCreateInfo.basePipelineIndex = basePipelineIndex;

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a graphics pipeline");
    }
}

void Pipeline::createDescriptorSetLayout(size_t UboCount)
{
    // here we show what uniforms we need.
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(UboCount);
    for (size_t i = 0; i < UboCount; i++)
    {
        descriptorSetLayoutBindings[i].binding = i;
        descriptorSetLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBindings[i].descriptorCount = 1;
        descriptorSetLayoutBindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    }

    // create descriptor set layout with given bidnings
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size()); // number of binding infos
    layoutCreateInfo.pBindings = descriptorSetLayoutBindings.data(); //array of binding infos

    //create descriptor set layout
    VkResult result = vkCreateDescriptorSetLayout(device.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

void Pipeline::createUniformBuffers(const std::vector<size_t>& dataSizes, size_t UboCount)
{
    // 1. Need a UniformBuffer for each swapchain image
    // 2. Need to create a buffer for each of them (eg. 3 buffers x 3 swapchain images) 
    UniformBuffers.resize(UboCount);
    for (auto& ubo : UniformBuffers)
    {
        // Uniform buffers and they contain buffers and device memory for each swapchain image
        ubo.buffer.resize(swapchainImageCount);
        ubo.buffer.resize(swapchainImageCount);
        ubo.deviceMemory.resize(swapchainImageCount);
        ubo.deviceMemory.resize(swapchainImageCount);
    }

    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        for (size_t j = 0; j < UboCount; j++)// TODO: create all three in a row?
            createBuffer(device.physicalDevice, device.logicalDevice, dataSizes[j],
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &UniformBuffers[j].buffer[i], &UniformBuffers[j].deviceMemory[i]);
    }
}

void Pipeline::createDescriptorSets(const size_t* dataSizes)
{
    const size_t setSize = swapchainImageCount;
    //resize descriptor set list so one for swapchain image
    descriptorSets.resize(setSize);

    std::vector<VkDescriptorSetLayout> setLayouts(setSize, descriptorSetLayout);
    
    m_DescriptorPool->AllocateDescriptorSets(descriptorSets, setLayouts, kDescriptorTypeUniformBuffer);
    

    std::vector<VkDescriptorBufferInfo> BufferInfos(setSize);
    std::vector<VkWriteDescriptorSet> SetWrites(setSize * UniformBuffers.size());
    size_t index = 0;
    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        for (size_t j = 0; j < UniformBuffers.size(); j++)
        {
            auto bufferInfo = &BufferInfos[j];
            // j - uniform buffers (eg. VP, Lights, Camera pos)
            // i - swapchainImages
            bufferInfo->buffer = UniformBuffers[j].buffer[i];
            bufferInfo->range = dataSizes[j];

            SetWrites[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            SetWrites[index].dstSet = descriptorSets[i]; // descriptor set to update (as many as swapchainImages)
            SetWrites[index].dstBinding = j; // binding to update
            SetWrites[index].dstArrayElement = 0; // index in array to update
            SetWrites[index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
            SetWrites[index].descriptorCount = 1; // amount to update
            SetWrites[index].pBufferInfo = bufferInfo; // info about buffer data to bind
            index++;
        }
    }
    vkUpdateDescriptorSets(device.logicalDevice, index, SetWrites.data(), 0, nullptr);
}

void Pipeline::createTextureSampler(VkDevice logicalDevice)
{
    //sampler creation info
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; //whether coords should be normalized between 0 an 1
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f; // level of details bias for mip level
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.pNext = nullptr;
    samplerCreateInfo.flags = 0;

    VkResult result = vkCreateSampler(logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

void Pipeline::createTextureSamplerSetLayout(VkDevice logicalDevice)
{
    //texture sampler descriptor set layout
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    //create a descriptor set layout with given bindings for texture
    VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
    textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutCreateInfo.bindingCount = 1;
    textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;
    textureLayoutCreateInfo.pNext = nullptr;
    textureLayoutCreateInfo.flags = 0;

    //create desciptor set layout
    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

VkDescriptorSet Pipeline::createTextureDescriptorSet(Texture texture, VkDevice logicalDevice)
{
    std::vector<VkDescriptorSet> descriptorSets(1);
    std::vector<VkDescriptorSetLayout> setLayouts = { samplerSetLayout };

    m_DescriptorPool->AllocateDescriptorSets(descriptorSets, setLayouts, kDescriptorTypeImageSampler);

    //texture image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// image layout when in use
    imageInfo.imageView = texture.getImage(0).getImageView(); // image to bind to set
    imageInfo.sampler = textureSampler; // sampler to use for set

    // descriptor write info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSets[0];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pNext = nullptr;

    // update new descriptor set
    vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);

    // return descriptor set location
    return descriptorSets[0];
}

void Pipeline::update(size_t index)
{
    auto uniformData = material.getUniformData();
    assert(uniformData.size());
    for (size_t i = 0; i < UniformBuffers.size(); i++)
    {
        void* dataMap = nullptr;
        void* data = alloca(uniformData[i].getTotalDataSize());
        size_t size = uniformData[i].getTotalDataSize();
        uniformData[i].getPackedDataBuffer(data);
        assert(data);

        vkMapMemory(device.logicalDevice, UniformBuffers[i].deviceMemory[index], 0, size, 0, &dataMap);
        memcpy(dataMap, data, size);
        vkUnmapMemory(device.logicalDevice, UniformBuffers[i].deviceMemory[index]);
    }
}

bool Pipeline::isMaterialCompatible(Material& mat) const
{
    return this->material == mat;
}

void Pipeline::CleanUp(VkDevice logicalDevice)
{
    vkDestroyDescriptorSetLayout(logicalDevice, samplerSetLayout, nullptr);
    vkDestroySampler(logicalDevice, textureSampler, nullptr);

    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);

    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        UniformBuffers[i].freeBuffer(device.logicalDevice);
    }

    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
}

void Pipeline::createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage) const
{
    auto shaderCode = readFile(shaderFileName);

    VkShaderModule shaderModule;
    createShaderModule(shaderModule, shaderCode);

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage = shaderStage;
    createInfo.module = shaderModule;
    createInfo.pName = "main";
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pSpecializationInfo = nullptr;
}

void Pipeline::createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());//pointer to code
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.flags = 0;

    VkResult result = vkCreateShaderModule(device.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a shader module!");
    }
}

void Pipeline::createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription) const
{
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void Pipeline::createVertexInputInstancedBindingDescription(VkVertexInputBindingDescription& bindingDescription) const
{
    bindingDescription.binding = 1;
    bindingDescription.stride = sizeof(InstanceData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
}

void Pipeline::createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) const
{
    attributeDescription.binding = binding;
    attributeDescription.location = location;
    attributeDescription.format = format;
    attributeDescription.offset = offset;
}

void Pipeline::createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions) const
{
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = material.IsInstanced() ? 2 : 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = material.IsInstanced() ? 7 : 3;
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;
    vertexInputCreateInfo.pNext = nullptr;
    vertexInputCreateInfo.flags = 0;
}

void Pipeline::createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssembly) const
{
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
}

void Pipeline::createViewport(VkViewport& viewport, float width, float height) const
{
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
}

void Pipeline::createScissor(VkRect2D& scissor, VkExtent2D extent) const
{
    scissor.offset = { 0,0 };
    scissor.extent = extent;
}

void Pipeline::createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& viewportStateCreateInfo, VkViewport* viewports, VkRect2D* scissors) const
{
    // Note: pass array of data and count here / array of data with count if i later want multiple viewports for some reason
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = viewports;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = scissors;
    viewportStateCreateInfo.pNext = nullptr;
    viewportStateCreateInfo.flags = 0;
}

void Pipeline::createPipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& rasterizerCreateInfo) const
{
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.pNext = nullptr;
    rasterizerCreateInfo.flags = 0;
}

void Pipeline::createMSAAStateCreateInfo(VkPipelineMultisampleStateCreateInfo& multisamplingCreateInfo, VkSampleCountFlagBits msaaSamples) const
{
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = msaaSamples;
    multisamplingCreateInfo.pNext = nullptr;
    multisamplingCreateInfo.pSampleMask = nullptr;
    multisamplingCreateInfo.flags = 0;
    multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;
}

void Pipeline::createPipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& colourState) const
{
    colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourState.blendEnable = VK_TRUE;
    colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colourState.colorBlendOp = VK_BLEND_OP_ADD;
    colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourState.alphaBlendOp = VK_BLEND_OP_ADD;
}

void Pipeline::createPipelineColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo& colorBlendingCreateInfo, VkPipelineColorBlendAttachmentState& colorState) const
{
    colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingCreateInfo.logicOpEnable = VK_FALSE; // alternative to calculations is to use logical operations
    colorBlendingCreateInfo.attachmentCount = 1;
    colorBlendingCreateInfo.pAttachments = &colorState;
    colorBlendingCreateInfo.pNext = nullptr;
    colorBlendingCreateInfo.flags = 0;
}

void Pipeline::createPushConstantRange()
{
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ModelMatrix);
}

void Pipeline::createDepthStencilCreateInfo(VkPipelineDepthStencilStateCreateInfo& depthStencilCreateInfo)
{
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // potential for cool effects, coparison op that allows overwrite
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilCreateInfo.pNext = nullptr;
    depthStencilCreateInfo.flags = 0;
  //  depthStencilCreateInfo.
}

void Pipeline::createPipelineLayout(VkDescriptorSetLayout* descriptorSetLayouts, uint32_t dSetLayoutCount, size_t pushSize)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = dSetLayoutCount;
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushSize ? 1 : 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushSize ? &pushConstantRange : nullptr;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;

    VkResult result = vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }
}
