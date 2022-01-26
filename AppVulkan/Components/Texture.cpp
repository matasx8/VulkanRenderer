#include "Texture.h"

Texture::Texture()
    : m_Image()
{
}

void Texture::createTexture(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    throw std::runtime_error("deprecated");
}

void Texture::DestroyTexture(VkDevice logicalDevice)
{
    
}
