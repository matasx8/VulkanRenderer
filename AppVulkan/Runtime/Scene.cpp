#include "Scene.h"

Scene::Scene(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    this->physicalDevice = physicalDevice;
    this->logicalDevice = logicalDevice;
}

void Scene::AddModel(std::string path, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool)
{
    // import model "scene"
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + path + ")");
    }

    //get vector of all materials with 1:1 ID placement
    std::vector<std::string> textureNames = Model::LoadMaterials(scene);

    // conversion from the materials list IDs to our Descriptor Arrays IDs
    std::vector<int> matToTex(textureNames.size());

    // loop over textureNames and create textures for them
    for (size_t i = 0; i < textureNames.size(); i++)
    {//if material had no texture, set 0 to indicate no texture, texture 0 will be reserved for a default texture
        if (textureNames[i].empty())
        {
            matToTex[i] = 0;
        }
        else
        {// otherwise, create texture and set value to index of new texture
            matToTex[i] = createTexture(textureNames[i], graphicsQueue, graphicsCommandPool);
        }
    }

    // load in all our meshes
    std::vector<Mesh> modelMeshes = Model::LoadNode(physicalDevice, logicalDevice, graphicsQueue, graphicsCommandPool,
        scene->mRootNode, scene, matToTex);

    Model meshModel = Model(modelMeshes);
    Models.push_back(meshModel);
}

std::vector<Model>& Scene::GetModels()
{
	// TODO: insert return statement here
    return Models;
}

Scene::~Scene()
{
}

int Scene::createTexture(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool)
{
    int textureImageLoc = createTextureImage(fileName, graphicsQueue, graphicsCommandPool);

    //create texture sampler
    createTextureSampler();

    //create imageview and add to list
    VkImageView imageView = createImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, logicalDevice);
    textureImageViews.push_back(imageView);

    //create texture descriptor
    int descriptorLoc = createTextureDescriptor(imageView);


    return descriptorLoc;
}

int Scene::createTextureImage(std::string fileName, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool)
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
    VkImage texImage;
    VkDeviceMemory texImageMemory;
    texImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory, physicalDevice, logicalDevice);

    //transition image to be dst for copy operation
    transitionImageLayout(logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //copy data to image
    copyImageBuffer(logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage, width, height);

    transitionImageLayout(logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //add texture data to vector for reference
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);

    //destroy staging buffers
    vkDestroyBuffer(logicalDevice, imageStagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, imageStagingBufferMemory, nullptr);

    //return index of new texture image
    return textureImages.size() - 1;
}

VkImage Scene::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkSampleCountFlagBits numSamples, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    //Create Image
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //layout of image data on creation
    imageCreateInfo.usage = useFlags; // bit flags defining what image will be used for
    imageCreateInfo.samples = numSamples;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    VkResult result = vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image");
    }


    // Create Memory for image

    // get memory requirements for a type of image
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

    result = vkAllocateMemory(logicalDevice, &memoryAllocInfo, nullptr, imageMemory);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate memory for image!");
    }

    // connect memory to image
    vkBindImageMemory(logicalDevice, image, *imageMemory, 0);

    return image;
}


VkImageView Scene::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice)
{
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image; // image to create view for
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format; //format of image data
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //allows remapping of rgba components to other rgba vlues
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    //subresources allow the view to view only a part of an image
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags; //which aspect of image to view (e.g. color_bit for viewing color)
    viewCreateInfo.subresourceRange.baseMipLevel = 0; //start mipmap level to view from
    viewCreateInfo.subresourceRange.levelCount = 1; //number of mipmap levels to view
    viewCreateInfo.subresourceRange.baseArrayLayer = 0; //start array level to view from
    viewCreateInfo.subresourceRange.layerCount = 1; //number of array levels to view

    VkImageView imageView;
    VkResult result = vkCreateImageView(logicalDevice, &viewCreateInfo, nullptr, &imageView);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image view");
    }

    return imageView;
}

void Scene::CleanUp()
{
    for (auto& model : Models)
    {
        model.destroyMeshModel();
    }

    //cleanup scene--!HERE
    vkDestroyDescriptorPool(logicalDevice, samplerDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, samplerSetLayout, nullptr);

    vkDestroySampler(logicalDevice, textureSampler, nullptr);

    //_aligned_free(modelTransferSpace);
    for (size_t i = 0; i < textureImages.size(); i++)
    {
        vkDestroyImageView(logicalDevice, textureImageViews[i], nullptr);
        vkDestroyImage(logicalDevice, textureImages[i], nullptr);
        vkFreeMemory(logicalDevice, textureImageMemory[i], nullptr);
    }
}

void Scene::createTextureSampler()
{
    //sampler creation info
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; //whether coords should be normalized between 0 an 1
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f; // level of details bias for mip level
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;

    VkResult result = vkCreateSampler(logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

int Scene::createTextureDescriptor(VkImageView textureImage)
{
    VkDescriptorSet descriptorSet;

    // descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = samplerDescriptorPool;
    setAllocInfo.pSetLayouts = &samplerSetLayout;
    setAllocInfo.descriptorSetCount = 1;

    // allocate descriptor sets
    VkResult result = vkAllocateDescriptorSets(logicalDevice, &setAllocInfo, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Texture Descriptor Sets");
    }

    //texture image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// image layout when in use
    imageInfo.imageView = textureImage; // image to bind to set
    imageInfo.sampler = textureSampler; // sampler to use for set

    // descriptor write info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    //update new descriptor set
    vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);

    // add descriptor set to list
    samplerDescriptorSets.push_back(descriptorSet);

    // return descriptor set location
    return samplerDescriptorSets.size() - 1;

}

stbi_uc* Scene::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
    // number of channels image uses
    int channels;

    //load pixel data for image
    std::string fileLoc = "Textures/" + fileName;
    stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        throw std::runtime_error("Failed to load a Texture file: " + fileName + "\n");
    }

    *imageSize = *width * *height * 4;
    return image;
}
