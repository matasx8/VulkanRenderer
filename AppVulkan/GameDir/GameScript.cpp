#pragma once
#include "GameScript.h"
#include<windows.h>
#include<glm/gtx/matrix_decompose.hpp>
#include "Model.h"

namespace GameScript
{
	VulkanRenderer* g_Engine;
	int g_KeyStateTracker = 0;
	constexpr int kRegularShader = 0;
	constexpr int kCelShader = 1;

	RenderPass rp;

	void OnInitialize(RendererInitializationSettings& initSettings)
	{
		initSettings.numThreadsInPool = 0;
	}

	void CreateSelectedMaterial()
	{
		ShaderCreateInfo shader = { "Shaders/selected_vert.spv", "Shaders/selected_frag.spv" };
		constexpr size_t kUniformCount = 1;

		std::vector<uint8_t> Uniforms(kUniformCount);
		Uniforms[0] = kUniformViewProjectionMatrix;

		shader.uniforms = std::move(Uniforms);
		shader.isInstanced = false;

		// TODO: make it so I can have no textures on a material
		std::vector<TextureCreateInfo> textureInfos;
		TextureCreateInfo tci;
		tci.fileName = "plain.png";
		tci.filtering = VK_FILTER_NEAREST;
		tci.wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		textureInfos.push_back(tci);

		Material material(0); // pass any number in constructor, material man will assign
		material.SetShader(shader);
		material.SetTextureDescriptions(textureInfos);

		g_Engine->CreateMaterial(material);
	}

	uint32_t CreateMaterials(const char* filename, int shadernum)
	{
		// get material by id
		// copy material
		// change something about it
		constexpr int kDefaultMaterial = 0;
		Material material = g_Engine->GetMaterial(kDefaultMaterial);

		std::vector<TextureCreateInfo> tcis(1);
		auto& tci = tcis[0];
		tci.fileName = filename;

		material.ChangeTextures(tcis);
		Shader shader = material.GetShader();
		switch (shadernum)
		{
		case 0:
			shader.m_ShaderInfo.vertexShader = "Shaders/shader_vert.spv"; 
			shader.m_ShaderInfo.fragmentShader = "Shaders/shader_frag.spv";
			break;
		case 1:
			shader.m_ShaderInfo.vertexShader = "Shaders/shader2_vert.spv";
			shader.m_ShaderInfo.fragmentShader = "Shaders/shader2_frag.spv";
			break;
		}

		material.SetShader(shader.m_ShaderInfo);

		// upload
		// material manager resuses what it can
		const auto materialID = g_Engine->CreateMaterial(material);
		return materialID;
	}

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		g_Engine = engine;
		constexpr int numDefaultResources = 4;

		CreateMaterials("UVs.jpg", kRegularShader);
		CreateMaterials("default.png", kCelShader);
		CreateSelectedMaterial();
		// duplicate with material
		// but first just duplicate
		auto predicate = [=](int idx)
		{
			return idx < numDefaultResources;
		};

		auto func = [&](ModelManager* const man, Model& model, int idx)
		{
			constexpr int numDuplicates = 10;
			Model copy = model;
			for(int i = 0; i < numDuplicates; i++)
				man->DuplicateWithMaterial(copy, false, i % 4);
		};
		g_Engine->ForEachModelConditional(predicate, func);


		auto update = [=](Model& model, int idx)
		{
			const int nthOther = numDefaultResources;
			const int offset = idx / nthOther; // fix the move func
			model.MoveLocal(glm::vec3(2.0f * offset, 2.f * (idx % nthOther + 1), 0.0f));
		};
		g_Engine->UpdateModels(update);
	}

	void GameScript::OnUpdate()
	{
		auto update = [=](Model& model, int idx)
		{
			model.RotateLocal(1.0f * g_Engine->GetDeltaTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		};
		g_Engine->UpdateModels(update);
	}

	void GameScript::OnEndOfFrame()
	{

	}

	
}