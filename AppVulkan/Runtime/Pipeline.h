#pragma once
#include "vulkan.h"
#include "Utilities.h"
#include <vector>
#include "Material.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "DescriptorPool.h"
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
	Pipeline(Device device, Camera* camera, size_t swapchainImageCount, DescriptorPool* descriptorPool);
    Pipeline(Material material, Device device, Camera* camera, size_t swapchainImageCount, DescriptorPool* descriptorPool);

    void createPipeline(VkExtent2D extent, VkRenderPass renderPass);
    // temporary.. for creation of initial pipeline
	void CreatePipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputCreateInfo,
        VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
        VkPipelineMultisampleStateCreateInfo* multisamplingCreateInfo, VkPipelineColorBlendStateCreateInfo* colourBlendingCreateInfo,
        VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo, VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, uint32_t basePipelineIndex,
        VkPipelineCreateFlags flags, VkDevice device);
    void createDescriptorSetLayout(size_t UboCount);
    void createDescriptorSets(const size_t* dataSizes);
    void createUniformBuffers(const std::vector<size_t>& dataSizes, size_t UboCount);
    void createTextureSampler(VkDevice logicalDevice);
    void createTextureSamplerSetLayout(VkDevice logicalDevice);
    VkDescriptorSet createTextureDescriptorSet(Texture texture, VkDevice logicalDevice);

    VkPipeline getPipeline() { usedThisFrame = true;  return pipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return samplerSetLayout; }
    uint32_t getPushConstantSize() const { return material.getPushConstantSize(); }
    const void* getPushConstantDataBuffer() const { return material.getPushConstantDataBuffer(); }
    VkDescriptorSet getDescriptorSet(size_t index) const { return descriptorSets[index]; }

    bool hasPushConstant() const { return material.hasPushConstant(); }
    bool useModelMatrixForPushConstant() const { return material.hasFlag(kUseModelMatrixForPushConstant); }
    void update(size_t index);
    bool wasUsedThisFrame() const { return usedThisFrame; }
    bool isMaterialCompatible(Material& mat) const;

    void CleanUp(VkDevice logicalDevice);
private:
    void createPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo, const char* shaderFileName, VkShaderStageFlagBits shaderStage) const;
    void createShaderModule(VkShaderModule& shaderModule, const std::vector<char>& code) const;
    void createVertexInputBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
    void createVertexInputInstancedBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
    void createVertexInputAttributeDescription(VkVertexInputAttributeDescription& attributeDescription, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) const;
    void createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputCreateInfo, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions) const;
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

    Device device;
    size_t swapchainImageCount;

    VkDescriptorSetLayout descriptorSetLayout;
    DescriptorPool* m_DescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<UniformBuffer> UniformBuffers;
    VkDescriptorSetLayout samplerSetLayout;
    VkSampler textureSampler;
    VkPushConstantRange pushConstantRange;

    bool usedThisFrame;

    Material material;
    Camera* camera;
};

