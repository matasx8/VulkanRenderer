#include "Pipeline.h"
#include "RenderPass.h"
#include <stdexcept>

Pipeline::Pipeline()
{
}

void Pipeline::createPipeline(VkExtent2D extent, RenderPass renderPass, const Material& material)
{
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
    createPipelineVertexInputStateCreateInfo(vertexInputCreateInfo, bindingDescriptions.data(), attributeDescriptions.data(), material.IsInstanced());

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
    createMSAAStateCreateInfo(multisamplingCreateInfo, static_cast<VkSampleCountFlagBits>(renderPass.GetRenderPassDesc().msaaCount));

    VkPipelineColorBlendAttachmentState colorState = {};
    createPipelineColorBlendAttachmentState(colorState);
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
    createPipelineColorBlendStateCreateInfo(colorBlendingCreateInfo, colorState);

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    createDepthStencilCreateInfo(depthStencilCreateInfo);

    createPushConstantRange();

    auto descriptorSetLayout = material.GetDescriptorSetLayout();
    auto textureDescritptorSetLayout = material.GetTextureDescriptorSetLayout();
    std::array<VkDescriptorSetLayout, 2> layouts = { descriptorSetLayout, textureDescritptorSetLayout };
    createPipelineLayout(layouts.data(), 2, pushConstantRange.size);

    CreatePipeline(shaderStages, &vertexInputCreateInfo, &inputAssembly, &viewportStateCreateInfo,
        NULL, &rasterizerCreateInfo, &multisamplingCreateInfo, &colorBlendingCreateInfo, &depthStencilCreateInfo,
        pipelineLayout, renderPass.GetVkRenderPass(), 0, VK_NULL_HANDLE, -1,
        VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT, s_DevicePtr.logicalDevice); // TODO pipeline cache or at least derivatives

    vkDestroyShaderModule(s_DevicePtr.logicalDevice, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(s_DevicePtr.logicalDevice, shaderStages[1].module, nullptr);
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

void Pipeline::CleanUp(VkDevice logicalDevice)
{
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

    VkResult result = vkCreateShaderModule(s_DevicePtr.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
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

void Pipeline::createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions, bool isInstanced) const
{
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = isInstanced ? 2 : 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = isInstanced ? 7 : 3;
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

    VkResult result = vkCreatePipelineLayout(s_DevicePtr.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }
}
