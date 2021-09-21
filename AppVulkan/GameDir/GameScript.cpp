#pragma once
#include "GameScript.h"
#include<windows.h>

namespace GameScript
{
	void UpdateModelsNew(float dt);
	void Input();

	VulkanRenderer* g_Engine;

	float g_Angle = 0.0f;

	std::vector<ModelHandle> g_ModelHandles;

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		if (engine)
			g_Engine = engine;
		else
			throw std::runtime_error("The Engine was passed as nullptr!");

		Scene& currentScene = g_Engine->getActiveScene();

		ShaderCreateInfo shaderInfo = { "Shaders/shader_instanced_vert.spv", "Shaders/shader_instanced_frag.spv" };
		shaderInfo.uniformCount = 3;

		std::vector<UniformData> UniformDatas(3);
		UniformDatas[0].name = "ViewProjection uniform";
		UniformDatas[0].sizes = { sizeof(ViewProjectionMatrix) };
		UniformDatas[0].dataBuffers = { currentScene.getViewProjectionPtr() };

		Light& light = currentScene.getLight(0);
		UniformDatas[1].name = "Light uniform";
		UniformDatas[1].sizes = { sizeof(glm::vec4), sizeof(glm::vec4) };
		UniformDatas[1].dataBuffers = { &light.m_Position, &light.m_Colour };

		Camera& camera = currentScene.getCamera();
		UniformDatas[2].name = "Camera";
		UniformDatas[2].sizes = { sizeof(glm::vec4) };
		UniformDatas[2].dataBuffers = { &camera.getCameraPosition() };

		shaderInfo.uniformData = std::move(UniformDatas);
		shaderInfo.pushConstantSize = 0;
		shaderInfo.shaderFlags = kDefault;
		shaderInfo.isInstanced = true;
		Material material1 = Material(shaderInfo);
		Scene& scene = g_Engine->getActiveScene();

		g_ModelHandles.emplace_back(scene.AddModel("Models/12140_Skull_v3_l2.obj", material1));
		/*g_ModelHandles.push_back(scene.DuplicateModel(1));/*
		g_ModelHandles.push_back(scene.DuplicateModel(1));
		g_ModelHandles.push_back(scene.DuplicateModel(1));
		g_ModelHandles.push_back(scene.DuplicateModel(1));
		g_ModelHandles.push_back(scene.DuplicateModel(1));
		g_ModelHandles.push_back(scene.DuplicateModel(1));*/
		/*g_ModelHandles.emplace_back(scene.AddModel("Models/Old House 2 3D Models.obj", material1));
		g_ModelHandles.emplace_back(scene.AddModel("Models/Old House 2 3D Models.obj", material1));
		g_ModelHandles.emplace_back(scene.AddModel("Models/Old House 2 3D Models.obj", material1));
		g_ModelHandles.emplace_back(scene.AddModel("Models/Old House 2 3D Models.obj", material1));*/
		//g_ModelHandles.emplace_back(scene.AddModel("Models/Group4.obj", material1));
	}

	void GameScript::OnUpdate()
	{
		Input();
		UpdateModelsNew(g_Engine->GetDeltaTime());
		//Sleep(10000);
	}

	void GameScript::OnEndOfFrame()
	{

	}

	void UpdateModelsNew(float dt)
	{
		Scene& scene = g_Engine->getActiveScene();
		float offset = 0.0f;
		{
			Model& model = scene.GetModel(0);
			auto& modelMatrix = model.GetModelMatrix();
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-g_Angle), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		int counter = 1;
		float yOffset = 0.0f;
		for (auto modelHandle : g_ModelHandles)
		{
			if (counter % 21 == 0)
			{
				counter = 0;
				yOffset += 50.0f;
				offset = 0;
			}
			Model& model = scene.GetModel(modelHandle);
			auto& modelMatrix = model.GetModelMatrix();
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f - yOffset, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-g_Angle), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			offset += 50.0f;
			counter++;
		}
		g_Angle += 10.0f * dt;
	}

	void Input()
	{
		Scene& scene = g_Engine->getActiveScene();
		bool* keys = g_Engine->window.getKeys();

		if (g_ModelHandles.size() < 1000 && keys[GLFW_KEY_P])
		{
			g_ModelHandles.push_back(scene.DuplicateModel(1));
		}
	}
}