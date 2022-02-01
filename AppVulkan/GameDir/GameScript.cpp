#pragma once
#include "GameScript.h"
#include<windows.h>
#include<glm/gtx/matrix_decompose.hpp>
#include "Model.h"

// Example of instanced and non-instanced drawing

namespace GameScript
{
	void UpdateModelsNew(float dt);
	void Input();

	VulkanRenderer* g_Engine;
	int g_KeyStateTracker = 0;

	RenderPass rp;

	void OnInitialize(RendererInitializationSettings& initSettings)
	{
		initSettings.numThreadsInPool = 0;
	}

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		g_Engine = engine;
		constexpr int numDefaultResources = 4;

		// duplicate with material
		// but first just duplicate
		auto predicate = [=](int idx)
		{
			return idx < numDefaultResources;
		};

		auto func = [&](ModelManager* const man, Model& model, int idx)
		{
			constexpr int numDuplicates = 10;
			for(int i = 0; i < numDuplicates; i++)
				man->Duplicate(model, false);
		};
		g_Engine->ForEachModelConditional(predicate, func);


		auto update = [=](Model& model, int idx)
		{
			const int nthOther = numDefaultResources;
			const int offset = idx / numDefaultResources; // fix the move func
			model.MoveLocal(glm::vec3(2.0f * offset, 2.f * (idx % numDefaultResources ), 0.0f));
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