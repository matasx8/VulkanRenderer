#include "InstanceDataBuffer.h"
#include <glm/glm.hpp>

InstanceDataBuffer::InstanceDataBuffer()
{
	m_CurrentCapacity = 1024;
	m_Buffer = nullptr;
	m_BufferPointer = m_Buffer;
	m_VkBuffer = VK_NULL_HANDLE;
	m_VkMemory = VK_NULL_HANDLE;

	m_CurrentSize = 0;
	m_ElementCount = 0;
	m_Stride = 0;
	m_Device = {};
}

void InstanceDataBuffer::Create(Device device)
{
	m_Device = device;

	m_Buffer = malloc(m_CurrentCapacity);
	if (m_Buffer == nullptr)
		throw std::runtime_error("Failed to allocate memory for 'InstanceDataBuffer'");
	m_BufferPointer = m_Buffer;

	CreateVkBuffer();
}

void InstanceDataBuffer::InsertBuffer(void* buffer, size_t size)
{
	if (buffer == nullptr || size <= 0)
		throw std::runtime_error("Invalid parameters were passed into 'InstanceDataBuffer::InsertBuffer'");

	if(m_ElementCount > 0 && m_Stride != size)
		throw std::runtime_error("Invalid parameters were passed into 'InstanceDataBuffer::InsertBuffer'\n stride should be same always");

	if (m_CurrentSize + size > m_CurrentCapacity)
		Grow();

	memcpy(m_BufferPointer, buffer, size);

	m_Stride = size;
	m_ElementCount++;
	m_CurrentSize += size;
	auto ptr = static_cast<char*>(m_BufferPointer);
	ptr += size;
	m_BufferPointer = ptr;
}

void InstanceDataBuffer::Reset()
{
	m_BufferPointer = m_Buffer;
	m_ElementCount = 0;
	m_Stride = 0;
	m_CurrentSize = 0;
}

VkBuffer InstanceDataBuffer::GetInstanceData()
{
	void* data = alloca(m_CurrentSize);
	VkResult result = vkMapMemory(m_Device.logicalDevice, m_VkMemory, 0, m_CurrentSize, 0, &data);
	memcpy(data, m_Buffer, m_CurrentSize);
	//m_Buffer = alloca(64);
	//memset(m_Buffer, 0, 64);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to map instance data memory");
	vkUnmapMemory(m_Device.logicalDevice, m_VkMemory);

	return m_VkBuffer;
}

void InstanceDataBuffer::Destroy()
{
	free(m_Buffer);
	vkDestroyBuffer(m_Device.logicalDevice, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device.logicalDevice, m_VkMemory, nullptr);
}

void InstanceDataBuffer::Grow()
{
	m_CurrentCapacity *= 2;
	void* newBuffer = malloc(m_CurrentCapacity);
	if(newBuffer == nullptr)
		throw std::runtime_error("Failed to allocate memory for 'InstanceDataBuffer' when growing it");

	if (m_Buffer == nullptr)
		throw std::runtime_error("Failed to allocate memory for 'InstanceDataBuffer' when growing it");

	memcpy(newBuffer, m_Buffer, m_CurrentSize);

	unsigned char* tmp = static_cast<unsigned char*>(newBuffer);
	tmp += m_CurrentSize;
	m_BufferPointer = static_cast<void*>(tmp);

	DestroyVkBuffer();
	CreateVkBuffer();

	free(m_Buffer);
	m_Buffer = newBuffer;
}

void InstanceDataBuffer::CreateVkBuffer()
{
	createBuffer(m_Device.physicalDevice, m_Device.logicalDevice, m_CurrentCapacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&m_VkBuffer, &m_VkMemory);
}

void InstanceDataBuffer::DestroyVkBuffer()
{
	vkDestroyBuffer(m_Device.logicalDevice, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device.logicalDevice, m_VkMemory, nullptr);
}
