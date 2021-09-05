#pragma once
#include "NonCopyable.h"
#include <stdexcept>
#include <malloc.h>

class InstanceDataBuffer : NonCopyable
{
public:
	InstanceDataBuffer();

	void InsertBuffer(void* buffer, size_t size);
	void Reset();

	size_t GetCurrentSize() const { return m_CurrentSize; }

	~InstanceDataBuffer();
private:

	void* m_Buffer;
	void* m_BufferPointer;
	size_t m_CurrentCapacity;
	size_t m_CurrentSize;
	size_t m_ElementCount;
	size_t m_Stride;

	void Grow();
};

