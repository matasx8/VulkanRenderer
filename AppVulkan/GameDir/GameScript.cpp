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

	RenderPass rp;

	void OnInitialize(RendererInitializationSettings& initSettings)
	{
		initSettings.numThreadsInPool = 0;
	}

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		//if (engine)
		//	g_Engine = engine;
		//else
		//	throw std::runtime_error("The Engine was passed as nullptr!");

		//Scene& currentScene = g_Engine->getActiveScene();

		//ShaderCreateInfo shaderInfo = { "Shaders/fractal_vert.spv", "Shaders/fractal_frag.spv" };
		//shaderInfo.uniformCount = 0;
		//shaderInfo.pushConstantSize = 0;
		//shaderInfo.shaderFlags = 0; 
		//shaderInfo.isInstanced = false;

		//Material quadMaterial = Material(shaderInfo);

		//RenderPassDesc DrawOpaques =
		//{
		//	4,	// msaaCount
		//	kRenderPassPlace_Opaques, // place
		//	kTargetSwapchain,
		//	1,
		//	VK_FORMAT_R8G8B8A8_UNORM,
		//	kLoadOpDontCare,
		//	kStoreOpStore,					// what happens if its dont care on swapchain?
		//	VK_FORMAT_D32_SFLOAT_S8_UINT,
		//	kLoadOpDontCare,
		//	kStoreOpStore
		//};

		//// create renderpass and add a command
		//rp.CreateRenderPass(DrawOpaques);
		//rp.DrawQuad(quadMaterial);

		//auto& rpm = currentScene.GetRenderPassManager();
		//rpm.AddRenderPass(DrawOpaques, rp);

		//// alright, plan to quickly do something so it works failed
		//// proceed to making everything from scratch again
		//// implement data structures (probably hash tables) to get pipelines, framebuffers...
		//// renderpasses will be recorded each frame
		//// !!!!!
		//// figure out how to deal with meshes
	}

	void GameScript::OnUpdate()
	{
	/*	Scene& currentScene = g_Engine->getActiveScene();
		auto& rpm = currentScene.GetRenderPassManager();
		rpm.AddRenderPass(rp);*/
	}

	void GameScript::OnEndOfFrame()
	{

	}

	
}