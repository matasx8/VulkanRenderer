#include "VulkanRenderer.h"

enum UniformType : uint8_t
{
	kUniformSun,
	kUniformCameraPosition,
	kUniformViewProjectionMatrix
};

MaterialManager::MaterialManager(VulkanRenderer& gfxEngine)
	: m_GfxEngine(gfxEngine), m_AllTimeMaterialCount(0)
{

}

void MaterialManager::InitializeDefaultMaterials()
{
	ShaderCreateInfo defaultShader = { "Shaders/shader_vert.spv", "Shaders/shader_frag.spv" };
	constexpr size_t kUniformCount = 3;

	std::vector<uint8_t> Uniforms(kUniformCount);
	Uniforms[0] = kUniformViewProjectionMatrix;
	Uniforms[1] = kUniformSun;
	Uniforms[2] = kUniformCameraPosition;

	defaultShader.uniforms = std::move(Uniforms);
	// TODO: change this to behave like uniforms
	defaultShader.pushConstantSize = 0;
	defaultShader.shaderFlags = kUseModelMatrixForPushConstant;
	defaultShader.isInstanced = false;

	// default material should be 0
	assert(m_AllTimeMaterialCount == 0);
	Material defaultMaterial(m_AllTimeMaterialCount, defaultShader);

	m_AllTimeMaterialCount++;
}
