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

	m_Buffer = malloc(m_CurrentSize);
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

InstanceDataBuffer::~InstanceDataBuffer()
{
	free(m_Buffer);
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
	createBuffer(m_Device.physicalDevice, m_Device.logicalDevice, m_CurrentCapacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_VkBuffer, &m_VkMemory);

	//createBuffer

	//if (m_VkBuffer == VK_NULL_HANDLE || m_VkMemory == VK_NULL_HANDLE)
	//	throw std::runtime_error("Failed to create VkBuffer for instance data");

	////get size of buffer needed for verts
	//VkDeviceSize bufferSize = 64;

	////temporary buffer to stage vertex data before transfering to gpu
	//VkBuffer stagingBuffer;
	//VkDeviceMemory stagingBufferMemory;

	////create buffer and allocate memory to it
	//createBuffer(m_Device.physicalDevice, m_Device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	&stagingBuffer, &stagingBufferMemory);

	////map memory to vertex buffer
	//auto a = glm::mat4(1.0f);
	//void* data; //create a pointer to a point in memory
	//vkMapMemory(m_Device.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data); //map the vertex buffer memory to that point
	//memcpy(data, &a, (size_t)bufferSize); // copy memory from vertices vector to the point
	//vkUnmapMemory(m_Device.logicalDevice, stagingBufferMemory); //unmap the vertex buffer memory

	////create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex_buffer)
	////buffer memory is to be device_local_bit meaning memory is on the gpu and only accesible by it and not cpu
	//createBuffer(m_Device.physicalDevice, m_Device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_VkBuffer, &m_VkMemory);

	////copy staging buffer to vertex buffer on gpu
	//copyBuffer(m_Device.logicalDevice, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	//// clean up staging buffer parts
	//vkDestroyBuffer(device, stagingBuffer, nullptr);
	//vkFreeMemory(device, stagingBufferMemory, nullptr); //why is this not working?
}

void InstanceDataBuffer::DestroyVkBuffer()
{
	vkDestroyBuffer(m_Device.logicalDevice, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device.logicalDevice, m_VkMemory, nullptr);
}
