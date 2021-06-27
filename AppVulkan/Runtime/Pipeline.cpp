#include "Pipeline.h"
#include <stdexcept>

Pipeline::Pipeline(Device device)
{
    this->device = device;
    usedThisFrame = false;
}

Pipeline::Pipeline(const Material material, Device device)
    :material(material), device(device)
{
}

void Pipeline::createPipeline(VkExtent2D extent)
{
    //if(material is uninitialized)
    // use default TODO
    VkPipelineShaderStageCreateInfo shaderStages[2];
    createPipelineShaderStageCreateInfo(shaderStages[0], material.vertexShader.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    createPipelineShaderStageCreateInfo(shaderStages[0], material.fragmentShader.c_str(), VK_SHADER_STAGE_VERTEX_BIT);

    VkVertexInputBindingDescription bindingDescription;
    createVertexInputBindingDescription(bindingDescription);

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
    createVertexInputAttributeDescription(attributeDescriptions[0], 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
    createVertexInputAttributeDescription(attributeDescriptions[1], 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm));
    createVertexInputAttributeDescription(attributeDescriptions[2], 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex));
    
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    createPipelineVertexInputStateCreateInfo(vertexInputCreateInfo, bindingDescription, attributeDescriptions.data());

    VkViewport viewport;
    createViewport(viewport, extent.width, extent.height);
    VkRect2D scissor;
    createScissor(scissor, extent);
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    createPipelineViewportStateCreateInfo(viewportStateCreateInfo, &viewport, &scissor);

    // TODO dynamic states
    

}

void Pipeline::CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
    VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
    VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
    VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
    VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout, 
    VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex, 
    VkPipelineCreateFlags flags,
    VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkDevice device)
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
   

    //pipeline derivatives - can create multiple pipelines that derive from one another for optimistions
    pipelineCreateInfo.basePipelineHandle = basePipelineHandle;
    pipelineCreateInfo.basePipelineIndex = basePipelineIndex;

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a graphics pipeline");
    }
    //destroy shader modules
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
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

    //create desciptor set layout!HERE
    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

int Pipeline::createTextureDescriptor(VkImageView textureImage, VkDevice logicalDevice)
{
    VkDescriptorSet descriptorSet;

    // descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = samplerDescriptorPool;
    setAllocInfo.pSetLayouts = &samplerSetLayout;
    setAllocInfo.descriptorSetCount = 1;
    

    // allocate descriptor sets
    VkResult result = vkAllocateDescriptorSets(logicalDevice, &setAllocInfo, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Texture Descriptor Sets");
    }

    //texture image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// image layout when in use
    imageInfo.imageView = textureImage; // image to bind to set
    imageInfo.sampler = textureSampler; // sampler to use for set

    // descriptor write info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    //update new descriptor set
    vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);

    // add descriptor set to list
    samplerDescriptorSets.push_back(descriptorSet);

    // return descriptor set location
    return samplerDescriptorSets.size() - 1;
}

VkDescriptorSet Pipeline::getTextureDescriptureSet(int i) const
{
    return samplerDescriptorSets[i];
}

void Pipeline::createTextureDescriptorPool(VkDevice logicalDevice)
{
    //create sampler descriptor pool
    // texture sampler pool
    VkDescriptorPoolSize samplerPoolSize = {};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = MAX_OBJECTS;

    VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
    samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
    samplerPoolCreateInfo.poolSizeCount = 1;
    samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

    VkResult result = vkCreateDescriptorPool(logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor pool");
    }
}




bool Pipeline::isMaterialCompatible(const Material& mat) const
{
    return this->material == mat;
}

void Pipeline::CleanUp(VkDevice logicalDevice)
{
    vkDestroyDescriptorPool(logicalDevice, samplerDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, samplerSetLayout, nullptr);
    vkDestroySampler(logicalDevice, textureSampler, nullptr);

    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
}

Pipeline::~Pipeline() {}

void Pipeline::createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage)
{
    auto shaderCode = readFile(shaderFileName);

    VkShaderModule shaderModule;
    createShaderModule(shaderModule, shaderCode);

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage = shaderStage;
    createInfo.module = shaderModule; //does it copy?
    createInfo.pName = "main";
}

void Pipeline::createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());//pointer to code

    VkResult result = vkCreateShaderModule(device.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a shader module!");
    }
}

void Pipeline::createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription)
{
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void Pipeline::createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t location, VkFormat format, uint32_t offset)
{
    attributeDescription.binding = 0;
    attributeDescription.location = location;
    attributeDescription.format = format;
    attributeDescription.offset = offset;
}

void Pipeline::createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription& bindingDescription, VkVertexInputAttributeDescription* attributeDescriptions)
{
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 3;
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;
}

void Pipeline::createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssembly)
{
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void Pipeline::createViewport(VkViewport& viewport, float width, float height)
{
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
}

void Pipeline::createScissor(VkRect2D& scissor, VkExtent2D extent)
{
    scissor.offset = { 0,0 };
    scissor.extent = extent;
}

void Pipeline::createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& viewportStateCreateInfo, VkViewport* viewports, VkRect2D* scissors)
{
    // Note: pass array of data and count here / array of data with count if i later want multiple viewports for some reason
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = viewports;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = scissors;
}

void Pipeline::createPipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& rasterizerCreateInfo)
{
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
}
