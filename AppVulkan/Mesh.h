#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

struct ModelMatrix {
	glm::mat4 model;
		float time;
};

class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, int newTexId);

	void setModel(glm::mat4 newModel);
	ModelMatrix getModel();
	int getTexId() { return texId; }

	int getVertexCount();
	VkBuffer getVertexBuffer();

	int getIndexCount();
	VkBuffer getIndexBuffer();

	void destroyBuffers();

	~Mesh();

private:
	ModelMatrix model;

	int texId;

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};

