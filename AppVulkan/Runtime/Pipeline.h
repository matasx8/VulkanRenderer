#pragma once
#include "vulkan.h"
#include "Utilities.h"
#include <vector>
#include "Material.h"
//#include "Camera.h"
#include "Model.h"
//#include "Light.h"
//#include "DescriptorPool.h"
//TODO: create default pipeline and make function for user that creates a pipeline using derivatives
struct UniformBuffer
{
    std::vector<VkBuffer> buffer;
    std::vector<VkDeviceMemory> deviceMemory;

    void freeBuffer(VkDevice logicalDevice)
    {
        for (size_t i = 0; i < buffer.size(); i++)
        {
            vkDestroyBuffer(logicalDevice, buffer[i], nullptr);
            vkFreeMemory(logicalDevice, deviceMemory[i], nullptr);
        }
    }
};
class Pipeline
{
public:
    Pipeline();

    void createPipeline(VkExtent2D extent, RenderPass renderPass, const Material& material);
    // temporary.. for creation of initial pipeline
	void CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
        VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
        VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
        VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex,
        VkPipelineCreateFlags flags, VkDevice device);


    VkPipeline getPipeline() { return pipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    uint32_t getPushConstantSize() const { return sizeof(ModelMatrix); }

    void CleanUp(VkDevice logicalDevice);
private:
    void createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage) const;
    void createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code) const;
    void createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
    void createVertexInputInstancedBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
    void createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) const;
    void createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions, bool isInstanced) const;
    void createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssembly) const;
    void createViewport(VkViewport& viewport, float width, float height) const;
    void createScissor(VkRect2D& scissor, VkExtent2D extent) const;
    void createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& viewportStateCreateInfo, VkViewport* viewports, VkRect2D* scissors) const;
    void createPipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& rasterizerCreateInfo) const;
    void createMSAAStateCreateInfo(VkPipelineMultisampleStateCreateInfo& multisamplingCreateInfo, VkSampleCountFlagBits msaaSamples) const;
    void createPipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& colourState) const;
    void createPipelineColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo& colorBlendingCreateInfo, VkPipelineColorBlendAttachmentState& colorState) const;
    void createPushConstantRange();
    void createDepthStencilCreateInfo(VkPipelineDepthStencilStateCreateInfo& depthStencilCreateInfo);
    void createPipelineLayout(VkDescriptorSetLayout* descriptorSetLayouts, uint32_t dSetLayoutCount, size_t pushSize);
    
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkPushConstantRange pushConstantRange;
};

