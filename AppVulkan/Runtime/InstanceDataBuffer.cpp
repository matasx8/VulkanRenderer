#include "InstanceDataBuffer.h"
#include <glm/glm.hpp>

InstanceDataBuffer::InstanceDataBuffer()
{
	constexpr int starting_cap = 0;
	m_Capacity = starting_cap;
	m_Buffer = std::vector<InstanceData>(starting_cap);
	m_VkBuffer = VK_NULL_HANDLE;
	m_VkMemory = VK_NULL_HANDLE;

	m_Device = {};
}

void InstanceDataBuffer::Create(Device device)
{
	m_Device = device;

	//CreateVkBuffer();
}

void InstanceDataBuffer::InsertBuffer(InstanceData& element)
{
	m_Buffer.push_back(element);
	if (m_Capacity < m_Buffer.capacity())
		Grow();
}

void InstanceDataBuffer::MoveInBuffer(InstanceData&& element)
{
	m_Buffer.push_back(element);
	if (m_Capacity < m_Buffer.capacity())
		Grow();
}

void InstanceDataBuffer::AddInstances(int numInstances)
{
	for (int i = 0; i < numInstances; i++)
	{
		InstanceData data = {};
		MoveInBuffer(std::move(data));
	}
}

void InstanceDataBuffer::Reset()
{
	m_Buffer.clear();
}

VkBuffer InstanceDataBuffer::GetInstanceData()
{
	void* data;
	VkResult result = vkMapMemory(m_Device.logicalDevice, m_VkMemory, 0, GetCurrentSize(), 0, &data);
	memcpy(data, m_Buffer.data(), GetCurrentSize());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to map instance data memory");
	vkUnmapMemory(m_Device.logicalDevice, m_VkMemory);

	return m_VkBuffer;
}

void InstanceDataBuffer::Destroy()
{
	vkDestroyBuffer(m_Device.logicalDevice, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device.logicalDevice, m_VkMemory, nullptr);
}

void InstanceDataBuffer::Grow()
{
	m_Capacity = m_Buffer.capacity();

	DestroyVkBuffer();
	CreateVkBuffer();
}

void InstanceDataBuffer::CreateVkBuffer()
{
	createBuffer(m_Device.physicalDevice, m_Device.logicalDevice, m_Buffer.capacity() * sizeof(InstanceData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&m_VkBuffer, &m_VkMemory);
}

void InstanceDataBuffer::DestroyVkBuffer()
{
	vkDestroyBuffer(m_Device.logicalDevice, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device.logicalDevice, m_VkMemory, nullptr);
}
