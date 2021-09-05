#include "InstanceDataBuffer.h"

InstanceDataBuffer::InstanceDataBuffer()
{
	m_CurrentCapacity = 1024 * 20;
	m_Buffer = malloc(m_CurrentSize);
	if (m_Buffer == nullptr)
		throw std::runtime_error("Failed to allocate memory for 'InstanceDataBuffer'");
	m_BufferPointer = m_Buffer;
	m_CurrentSize = 0;
	m_ElementCount = 0;
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
}

void InstanceDataBuffer::Reset()
{
	m_BufferPointer = m_Buffer;
	m_ElementCount = 0;
	m_Stride = 0;
	m_CurrentSize = 0;
}

InstanceDataBuffer::~InstanceDataBuffer()
{
	free(m_Buffer);
}

void InstanceDataBuffer::Grow()
{
	m_CurrentCapacity *= 2;
	void* newBuffer = malloc(m_CurrentCapacity);

	if (m_Buffer == nullptr)
		throw std::runtime_error("Failed to allocate memory for 'InstanceDataBuffer' when growing it");

	memcpy(newBuffer, m_Buffer, m_CurrentSize);

	unsigned char* tmp = static_cast<unsigned char*>(newBuffer);
	tmp += m_CurrentSize;
	m_BufferPointer = static_cast<void*>(tmp);

	free(m_Buffer);
	m_Buffer = newBuffer;
}
