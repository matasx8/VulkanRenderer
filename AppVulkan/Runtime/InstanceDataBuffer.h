#pragma once
#include "NonCopyable.h"
#include "Utilities.h"
#include <stdexcept>
#include <malloc.h>

class InstanceDataBuffer : NonCopyable
{
public:
	InstanceDataBuffer();

	void Create(Device device);

	void InsertBuffer(void* buffer, size_t size);
	// Note: does not deallocate anything
	void Reset();

	size_t GetCurrentSize() const { return m_CurrentSize; }
	VkBuffer GetInstanceData();

	~InstanceDataBuffer();
private:

	void* m_Buffer;
	void* m_BufferPointer;
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkMemory;

	size_t m_CurrentCapacity;
	size_t m_CurrentSize;
	size_t m_ElementCount;
	size_t m_Stride;

	Device m_Device;

	void Grow();
	void CreateVkBuffer();
	void DestroyVkBuffer();
};

