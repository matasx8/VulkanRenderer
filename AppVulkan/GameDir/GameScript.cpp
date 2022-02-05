#pragma once
#include "GameScript.h"
#include<windows.h>
#include<glm/gtx/matrix_decompose.hpp>
#include "Model.h"

namespace GameScript
{
	VulkanRenderer* g_Engine;
	int g_KeyStateTracker = 0;

	RenderPass rp;

	void OnInitialize(RendererInitializationSettings& initSettings)
	{
		initSettings.numThreadsInPool = 0;
	}

	uint32_t CreateMaterials()
	{
		// get material by id
		// copy material
		// change something about it
		constexpr int kDefaultMaterial = 0;
		Material material = g_Engine->GetMaterial(kDefaultMaterial);

		std::vector<TextureCreateInfo> tcis(1);
		auto& tci = tcis[0];
		tci.fileName = "UVs.jpg";

		material.ChangeTextures(tcis);

		// upload
		// material manager resuses what it can
		const auto materialID = g_Engine->CreateMaterial(material);
		return materialID;
	}

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		g_Engine = engine;
		constexpr int numDefaultResources = 4;

		CreateMaterials();
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
				man->DuplicateWithMaterial(copy, false, i % 2);
		};
		g_Engine->ForEachModelConditional(predicate, func);


		auto update = [=](Model& model, int idx)
		{
			const int nthOther = numDefaultResources;
			const int offset = idx / numDefaultResources; // fix the move func
			model.MoveLocal(glm::vec3(2.0f * offset, 2.f * (idx % numDefaultResources + 2), 0.0f));
		};
		g_Engine->UpdateModels(update);
	}

	void GameScript::OnUpdate()
	{

	}

	void GameScript::OnEndOfFrame()
	{

	}

	
}