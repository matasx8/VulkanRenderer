#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Utilities.h"

// unique identifier for a Model
typedef unsigned int ModelHandle;
typedef glm::mat4x4 ModelMatrix;

struct ViewProjectionMatrix : UniformProvider
{
	void ProvideUniformData(void* dst) override;
	size_t ProvideUniformDataSize() override;

	glm::mat4 projection;
	glm::mat4 view;
};

class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		std::vector<Vertex>* vertices, std::vector<uint32_t>* indices);

	uint32_t GetMaterialID() const { return m_MaterialId; }
	ModelHandle GetModelHandle() const { return m_ModelHandle; }
	int GetVertexCount() const;
	VkBuffer GetVertexBuffer() const;
	uint32_t GetIndexCount() const;
	VkBuffer GetIndexBuffer() const;

	void destroyBuffers();

private:

	friend class ModelManager;

	void SetModelHandle(ModelHandle handle);
	void SetMaterialID(uint32_t id);

	void createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	uint32_t indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	uint32_t m_MaterialId;
	ModelHandle m_ModelHandle;
	

};

