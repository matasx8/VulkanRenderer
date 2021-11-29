#pragma once
#include "GameScript.h"
#include<windows.h>
#include<glm/gtx/matrix_decompose.hpp>

// Example of instanced and non-instanced drawing

namespace GameScript
{
	void UpdateModelsNew(float dt);
	void Input();

	VulkanRenderer* g_Engine;
	int g_KeyStateTracker = 0;

	void OnInitialize(RendererInitializationSettings& initSettings)
	{
		initSettings.numThreadsInPool = 0;
	}

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		if (engine)
			g_Engine = engine;
		else
			throw std::runtime_error("The Engine was passed as nullptr!");

		Scene& currentScene = g_Engine->getActiveScene();

		ShaderCreateInfo shaderInfo = { "Shaders/fractal_vert.spv", "Shaders/fractal_frag.spv" };
		shaderInfo.uniformCount = 0;
		shaderInfo.pushConstantSize = 0;
		shaderInfo.shaderFlags = 0; 
		shaderInfo.isInstanced = false;

		Material quadMaterial = Material(shaderInfo);

		// AddCustomRenderPass
	}

	void GameScript::OnUpdate()
	{

	}

	void GameScript::OnEndOfFrame()
	{

	}

	
}