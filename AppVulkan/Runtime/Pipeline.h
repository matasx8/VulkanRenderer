#pragma once
#include "vulkan.h"
#include "Utilities.h"
#include <vector>
#include "Material.h"
//TODO: create default pipeline and make function for user that creates a pipeline using derivatives

class Pipeline
{
public:
	Pipeline(Device device);
    Pipeline(Material material, Device device);

    void createPipeline(VkExtent2D extent);
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

    VkPipeline getPipeline() { usedThisFrame = true;  return pipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSet getTextureDescriptureSet(int i) const;
    VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return samplerSetLayout; }

    bool wasUsedThisFrame() const { return usedThisFrame; }
    bool isMaterialCompatible(const Material& mat) const;

    void CleanUp(VkDevice logicalDevice);
	~Pipeline();
private:
    void createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage);
    void createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code);
    void createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription);
    void createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t location, VkFormat format, uint32_t offset);
    void createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription& bindingDescription, VkVertexInputAttributeDescription* attributeDescriptions);
    void createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssembly);
    void createViewport(VkViewport& viewport, float width, float height);
    void createScissor(VkRect2D& scissor, VkExtent2D extent);
    void createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& viewportStateCreateInfo, VkViewport* viewports, VkRect2D* scissors);
    
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    Device device;

    VkDescriptorSetLayout samplerSetLayout;
    std::vector<VkDescriptorSet> samplerDescriptorSets;
    VkDescriptorPool samplerDescriptorPool;
    VkSampler textureSampler;

    bool usedThisFrame;

    const Material material;
};

