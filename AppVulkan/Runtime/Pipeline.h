#pragma once
#include "vulkan.h"
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

    VkPipeline getPipeline() const { return pipeline; }
	~Pipeline();
private:
    VkPipeline pipeline;
};

