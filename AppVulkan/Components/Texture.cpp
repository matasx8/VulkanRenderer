#include "Texture.h"

Texture::Texture()
    : m_Image(), m_Sampler(), m_TextureDescription()
{
}

void Texture::AddImage(Image& image)
{
    m_Image = image;
}

void Texture::SetSampler(VkSampler sampler)
{
    m_Sampler = sampler;
}

void Texture::SetTextureDescription(TextureCreateInfo& desc)
{
    m_TextureDescription = desc;
}

void Texture::SetTextureDescription(const TextureCreateInfo& desc)
{
    m_TextureDescription = desc;
}

VkSampler Texture::GetSampler() const
{
    return m_Sampler;
}


void Texture::DestroyTexture(VkDevice logicalDevice)
{
    
}

bool Texture::operator==(const Texture& texture) const
{
    return m_TextureDescription == texture.m_TextureDescription;
}

bool Texture::operator==(const TextureCreateInfo& texture) const
{
    return m_TextureDescription == texture;
}

bool TextureCreateInfo::operator==(const TextureCreateInfo& tci) const
{
    if (fileName != tci.fileName)
        return false;
    if (filtering != tci.filtering)
        return false;
    if (wrap != tci.wrap)
        return false;
    return true;
}
