#include "VulkanRenderer.h"

MaterialManager::MaterialManager(VulkanRenderer& gfxEngine)
	: m_GfxEngine(gfxEngine)
{

}

void MaterialManager::InitializeDefaultMaterials()
{
	ShaderCreateInfo defaultShader = { "Shaders/shader_vert.spv", "Shaders/shader_frag.spv" };
	constexpr size_t kUniformCount = 3;
	defaultShader.uniformCount = kUniformCount;

}
