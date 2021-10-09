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

	void InsertBuffer(InstanceData& element);
	void MoveInBuffer(InstanceData&& element);

	void AddInstances(int numInstances);

	void Reset();

	size_t GetCurrentSize() const { return m_Buffer.size() * sizeof(InstanceData); }
	size_t GetElementCount() const { return m_Buffer.size(); }
	InstanceData& GetElement(size_t index) { return m_Buffer[index]; }
	VkBuffer GetInstanceData();

	// Applies a function to each element
	template<typename Func, typename... Args>
	void ApplyFunctionForEach(Func function, Args&... args);

	// TODO:
	template<typename Func>
	void ApplyFunctionForEach_Parallel(Func function);

	void Destroy();
private:

	std::vector<InstanceData> m_Buffer;
	size_t m_Capacity;

	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkMemory;

	Device m_Device;

	void Grow();
	void CreateVkBuffer();
	void DestroyVkBuffer();
};

template<typename Func, typename... Args>
inline void InstanceDataBuffer::ApplyFunctionForEach(Func function, Args&... args)
{
	for (auto& element : m_Buffer)
		function(element, args...);
}