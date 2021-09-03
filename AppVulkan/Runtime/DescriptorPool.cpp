#include "DescriptorPool.h"
#include <vulkan.h>
#include <stdexcept>
#include <vector>

DescriptorPool::DescriptorPool()
{

}

void DescriptorPool::CreateDescriptorPool(VkDevice logicalDevice, uint32_t descriptorPoolFlags)
{
	std::array<VkDescriptorPoolSize, kDescriptorPoolTypeCount> poolSizes;
	m_LogicalDevice = logicalDevice;

	// first lets fill the array that will indicate what type of pools will we need
	for (uint32_t i = 0; i < kDescriptorPoolTypeCount; i++)
	{
		switch (descriptorPoolFlags & 1 << i)
		{
		case kDescriptorTypeImageSampler:
			poolSizes[i] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
			break;

		case kDescriptorTypeUniformBuffer:
			poolSizes[i] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
			break;

		default:
			throw std::runtime_error("Failed to create descriptor pool. Something got messed up with the order of flags.");
		}
	}

	// next create the descriptor pools and put them in their slots in the array
	for (uint32_t i = 0; i < kDescriptorPoolTypeCount; i++)
	{
		if (poolSizes[i].descriptorCount > 0)
		{
			const uint32_t kPoolMaxSets = 10;
			VkDescriptorPoolCreateInfo poolCreateInfo = {

				VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				nullptr,
				0,
				kPoolMaxSets, // grow this dynamically
				1, // poolsize count
				&poolSizes[i]
			};
			DescriptorPoolSingle pool = {};
			pool.pool = 0;
			pool.poolMaxSets = kPoolMaxSets;
			pool.type = static_cast<DescriptorPoolFlags>(1 << i);
			m_DescriptorPools[i].push_back(pool);

			VkResult result = vkCreateDescriptorPool(logicalDevice, &poolCreateInfo, nullptr, &m_DescriptorPools[i].front().pool);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a descriptor pool!");
			}
		}
	}
}

void DescriptorPool::AllocateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSetsDst, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, DescriptorPoolFlags descriptorSetType)
{
	VkResult result = VK_RESULT_MAX_ENUM;
	uint32_t index = 0;
	auto& pools = m_DescriptorPools[GetPoolIndexFromType(descriptorSetType)];

	bool poolsWereGrown = false;

	while (result != VK_SUCCESS)
	{
		if (poolsWereGrown)
			throw std::runtime_error("A new Descriptor Pool was created but Descriptor Set allocation still failed");

		if (index > 0 && index >= pools.size())
		{// we ended up here, that means the pool is full (or fragmented) and we need another pool
			GrowPool(descriptorSetType);
			poolsWereGrown = true;
		}
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = GetDescriptorPool(descriptorSetType, index);
		descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		result = vkAllocateDescriptorSets(m_LogicalDevice, &descriptorSetAllocateInfo, descriptorSetsDst.data());

		index++;
	}
}

void DescriptorPool::DestroyDescriptorPool()
{
	for (auto& poolSlot : m_DescriptorPools)
	{
		for (auto& pool : poolSlot)
		{
			vkDestroyDescriptorPool(m_LogicalDevice, pool.pool, nullptr);
		}
	}
}

VkDescriptorPool DescriptorPool::GetDescriptorPool(DescriptorPoolFlags descriptorPoolType, uint32_t index) const
{
	// TODO: if requested for pool that does not exist - create it?
	return m_DescriptorPools[GetPoolIndexFromType(descriptorPoolType)][index].pool;
}

uint32_t DescriptorPool::GetPoolIndexFromType(DescriptorPoolFlags descriptorPoolType) const
{
	switch (descriptorPoolType)
	{
	case kDescriptorTypeImageSampler:
		return 0;

	case kDescriptorTypeUniformBuffer:
		return 1;
	}
}

VkDescriptorType DescriptorPool::GetVkDescriptorTypeFromFlag(DescriptorPoolFlags descriptorPoolType) const
{
	switch (descriptorPoolType)
	{
	case kDescriptorTypeImageSampler:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	case kDescriptorTypeUniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}

void DescriptorPool::GrowPool(DescriptorPoolFlags descriptorPoolType)
{
	uint32_t poolIndex = GetPoolIndexFromType(descriptorPoolType);
	auto& pool = m_DescriptorPools[poolIndex].back();
	uint32_t newMaxSets = pool.poolMaxSets * 2;

	// double the max sets of the last pool
	VkDescriptorPoolSize poolSize = { GetVkDescriptorTypeFromFlag(pool.type), 1 };

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = newMaxSets;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;

	DescriptorPoolSingle singlePool = {};
	singlePool.pool = 0;
	singlePool.poolMaxSets = newMaxSets;
	singlePool.type = descriptorPoolType;
	m_DescriptorPools[poolIndex].push_back(pool);
	//m_DescriptorPools[poolIndex].emplace_back(descriptorPoolType, 0, newMaxSets);

	VkResult result = vkCreateDescriptorPool(m_LogicalDevice, &poolCreateInfo, nullptr, &m_DescriptorPools[poolIndex].back().pool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a descriptor pool!");
	}
}
