#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Utilities.h"

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

	void SetMaterialID(uint32_t id);
	uint32_t GetMaterialID() const { return m_MaterialId; };

	int GetVertexCount() const;
	VkBuffer GetVertexBuffer() const;

	uint32_t GetIndexCount() const;
	VkBuffer GetIndexBuffer() const;

	void destroyBuffers();

private:

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	uint32_t indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	uint32_t m_MaterialId;
	
	void createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};

