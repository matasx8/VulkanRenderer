#pragma once
#include "NonCopyable.h"
#include "vulkan.h"
#include <stdint.h>
#include <vector>
#include <array>

static const unsigned int kDescriptorPoolTypeCount = 2;
enum DescriptorPoolFlags : uint32_t
{
	kDescriptorTypeImageSampler = 1,
	kDescriptorTypeUniformBuffer
};

struct DescriptorPoolSingle
{
	DescriptorPoolFlags type;
	VkDescriptorPool pool;

	uint32_t poolMaxSets;
};

// have two pools for samplers and uniform buffers
class DescriptorPool : NonCopyable
{
public:
	DescriptorPool();

	void CreateDescriptorPool(VkDevice logicalDevice, uint32_t descriptorPoolFlags = 3);
	void AllocateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSetsDst, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, DescriptorPoolFlags descriptorSetType);

	void DestroyDescriptorPool();
private:

	VkDescriptorPool GetDescriptorPool(DescriptorPoolFlags descriptorPoolType, uint32_t index) const;
	uint32_t GetPoolIndexFromType(DescriptorPoolFlags descriptorPoolType) const;
	VkDescriptorType GetVkDescriptorTypeFromFlag(DescriptorPoolFlags descriptorPoolType) const;

	void GrowPool(DescriptorPoolFlags descriptorPoolType);
	// index the same order as 'DescriptorPoolFlags'
	std::array<std::vector<DescriptorPoolSingle>, kDescriptorPoolTypeCount> m_DescriptorPools;

	VkDevice m_LogicalDevice;
};

