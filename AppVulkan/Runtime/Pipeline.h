#pragma once
#include "vulkan.h"
#include "Utilities.h"
#include <vector>
//TODO: create default pipeline and make function for user that creates a pipeline using derivatives

class Pipeline
{
public:
	Pipeline();

	void CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
        VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
        VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
        VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex,
        VkPipelineCreateFlags flags,
        VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkDevice device);
    void createTextureSampler(VkDevice logicalDevice);
    void createTextureSamplerSetLayout(VkDevice logicalDevice);
    void createTextureDescriptorPool(VkDevice logicalDevice);
    int createTextureDescriptor(VkImageView textureImage, VkDevice logicalDevice);

    VkPipeline getPipeline() const { return pipeline; }
    VkDescriptorSet getTextureDescriptureSet(int i) const;
    VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return samplerSetLayout; }

    void CleanUp(VkDevice logicalDevice);
	~Pipeline();
private:
    VkPipeline pipeline;

    VkDescriptorSetLayout samplerSetLayout;
    std::vector<VkDescriptorSet> samplerDescriptorSets;
    VkDescriptorPool samplerDescriptorPool;
    VkSampler textureSampler;
};

