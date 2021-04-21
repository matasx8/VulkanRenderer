#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices)
{
    vertexCount = vertices->size();
    indexCount = indices->size();
    physicalDevice = newPhysicalDevice;
    device = newDevice;
    createVertexBuffer(transferQueue, transferCommandPool, vertices);
    createIndexBuffer(transferQueue, transferCommandPool, indices);
}


int Mesh::getVertexCount()
{
    return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
    return vertexBuffer;
}

int Mesh::getIndexCount()
{
    return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
    return indexBuffer;
}

void Mesh::destroyBuffers()
{
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
}

Mesh::~Mesh()
{
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
{
    //get size of buffer needed for verts
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    //temporary buffer to stage vertex data before transfering to gpu
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create buffer and allocate memory to it
    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    //map memory to vertex buffer
    void* data; //create a pointer to a point in memory
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); //map the vertex buffer memory to that point
    memcpy(data, vertices->data(), (size_t)bufferSize); // copy memory from vertices vector to the point
    vkUnmapMemory(device, stagingBufferMemory); //unmap the vertex buffer memory

    //create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex_buffer)
    //buffer memory is to be device_local_bit meaning memory is on the gpu and only accesible by it and not cpu
    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

    //copy staging buffer to vertex buffer on gpu
    copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

    // clean up staging buffer parts
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr); //why is this not working?
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
    //get size of buffer needed for indices
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        //map memory to index buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices->data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    //create buffer for index data on gpu access only area
    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

    //copy from staging buffer to gpu access buffer
    copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
