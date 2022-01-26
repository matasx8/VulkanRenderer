#include "Texture.h"

Texture::Texture()
    : m_Image()
{
}

void Texture::AddImage(Image& image)
{
    m_Image = image;
}

void Texture::SetDescriptorSet(VkDescriptorSet descriptorSet)
{
    m_DescriptorSet = descriptorSet;
}

void Texture::SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
    m_DescriptorSetLayout = descriptorSetLayout;
}

void Texture::SetSampler(VkSampler sampler)
{
    m_Sampler = sampler;
}

VkDescriptorSet Texture::GetDescriptorSet() const
{
    return m_DescriptorSet;
}

VkDescriptorSetLayout Texture::GetDescriptorSetLayout() const
{
    return m_DescriptorSetLayout;
}

VkSampler Texture::GetSampler() const
{
    return m_Sampler;
}


void Texture::DestroyTexture(VkDevice logicalDevice)
{
    
}
