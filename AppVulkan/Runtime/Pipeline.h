#pragma once
#include "vulkan/vulkan.h"

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
        VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkDevice device);

    VkPipeline getPipeline() const { return pipeline; }
	~Pipeline();
private:
    VkPipeline pipeline;
};

