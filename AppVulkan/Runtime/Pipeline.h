#pragma once
#include "vulkan.h"
#include "Utilities.h"
#include <vector>
#include "Material.h"
#include "Camera.h"
#include "Mesh.h"
#include "Light.h"
//TODO: create default pipeline and make function for user that creates a pipeline using derivatives

class Pipeline
{
public:
	Pipeline(Device device, Camera* camera);
    Pipeline(Material material, Device device, Camera* camera);

    void createPipeline(VkExtent2D extent, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
    // temporary.. for creation of initial pipeline
	void CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
        VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
        VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
        VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex,
        VkPipelineCreateFlags flags, VkDevice device);
    void createTextureSampler(VkDevice logicalDevice);
    void createTextureSamplerSetLayout(VkDevice logicalDevice);
    void createTextureDescriptorPool(VkDevice logicalDevice);
    int createTextureDescriptor(VkImageView textureImage, VkDevice logicalDevice);

    VkPipeline getPipeline() { usedThisFrame = true;  return pipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSet getTextureDescriptureSet(int i) const;
    VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return samplerSetLayout; }

    bool wasUsedThisFrame() const { return usedThisFrame; }
    bool isMaterialCompatible(Material& mat) const;

    void CleanUp(VkDevice logicalDevice);
	~Pipeline();
private:
    void createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage) const;
    void createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code) const;
    void createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
    void createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t location, VkFormat format, uint32_t offset) const;
    void createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription& bindingDescription, VkVertexInputAttributeDescription* attributeDescriptions) const;
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
    void createPipelineLayout(VkDescriptorSetLayout* descriptorSetLayouts, uint32_t dSetLayoutCount);
    
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    Device device;

    VkDescriptorSetLayout samplerSetLayout;
    std::vector<VkDescriptorSet> samplerDescriptorSets;
    VkDescriptorPool samplerDescriptorPool;
    VkSampler textureSampler;
    VkPushConstantRange pushConstantRange;

    bool usedThisFrame;

    Material material;
    Camera* camera;
};

