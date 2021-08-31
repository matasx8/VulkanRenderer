#include "Texture.h"

void Texture::createTexture(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    int textureImageLoc = createTextureImage(fileName, graphicsQueue, graphicsCommandPool, physicalDevice, logicalDevice);


    //create imageview and add to list
    Images[textureImageLoc].createImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, logicalDevice);
}

void Texture::DestroyTexture(VkDevice logicalDevice)
{
    for (auto& image : Images)
    {
        image.destroyImage(logicalDevice);
    }
}


int Texture::createTextureImage(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    // load image file
    int width, height;
    VkDeviceSize imageSize;
    stbi_uc* imageData = loadTextureFile(fileName, &width, &height, &imageSize);

    // create staging buffer to hold loaded data, ready to copy to device
    VkBuffer imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;
    createBuffer(physicalDevice, logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &imageStagingBuffer, &imageStagingBufferMemory);

    // copy image data to staging buffer
    void* data;
    vkMapMemory(logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(logicalDevice, imageStagingBufferMemory);

    //free original image data
    stbi_image_free(imageData);

    //create image to hold final texture
    Image texImage;
    texImage.createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice, logicalDevice);

    //transition image to be dst for copy operation
    transitionImageLayout(logicalDevice, graphicsQueue, graphicsCommandPool, texImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //copy data to image
    copyImageBuffer(logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage.getImage(), width, height);

    transitionImageLayout(logicalDevice, graphicsQueue, graphicsCommandPool, texImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //add texture data to vector for reference
    Images.push_back(texImage);

    //destroy staging buffers
    vkDestroyBuffer(logicalDevice, imageStagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, imageStagingBufferMemory, nullptr);

    //return index of new texture image
    return Images.size() - 1;
}



stbi_uc* Texture::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
    // number of channels image uses
    int channels;

    //load pixel data for image
    std::string fileLoc = "Textures/" + fileName;
    stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb);
        if (!image)
            throw std::runtime_error("Failed to load a Texture file: " + fileName + "\n");
    }

    *imageSize = *width * *height * 4;
    return image;
}
