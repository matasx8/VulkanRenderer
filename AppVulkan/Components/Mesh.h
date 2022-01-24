#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Utilities.h"

class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		std::vector<Vertex>* vertices, std::vector<uint32_t>* indices);

	int getVertexCount();
	VkBuffer getVertexBuffer();

	uint32_t getIndexCount();
	VkBuffer getIndexBuffer();

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

