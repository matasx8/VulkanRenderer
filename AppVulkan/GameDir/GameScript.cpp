#pragma once
#include "GameScript.h"
#include<windows.h>

// Example of instanced and non-instanced drawing

namespace GameScript
{
	void UpdateModelsNew(float dt);
	void Input();

	VulkanRenderer* g_Engine;

	float g_Angle = 0.0f;
	//auto g_ExampleModel = "Models/Old House 2 3D Models.obj";
	auto g_ExampleModel = "Models/duck2.obj";
	//auto g_ExampleModel = "Models/Group4.obj";
	int g_InstanceCount = 1000 - 1;
	int g_KeyStateTracker = 0;
	ModelHandle g_InstancedModel;

	std::vector<ModelHandle> g_ModelHandles;

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		if (engine)
			g_Engine = engine;
		else
			throw std::runtime_error("The Engine was passed as nullptr!");

		Scene& currentScene = g_Engine->getActiveScene();
		// instantiate 300 instanced models
		{
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
			shaderInfo.shaderFlags = 0;
			// mark that this will be instanced 
			shaderInfo.isInstanced = true;

			Material material1 = Material(shaderInfo);

			g_InstancedModel = currentScene.AddModel(g_ExampleModel, material1);
			g_ModelHandles.emplace_back(g_InstancedModel);
			// add a number of instances
			currentScene.GetModel(g_InstancedModel).AddInstances(g_InstanceCount);
		}

		{
			ShaderCreateInfo shaderInfo = { "Shaders/shader_vert.spv", "Shaders/shader_frag.spv" };
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
			shaderInfo.shaderFlags = kUseModelMatrixForPushConstant;
			shaderInfo.isInstanced = false;

			Material material1 = Material(shaderInfo);

			auto handle = currentScene.AddModel(g_ExampleModel, material1);
			g_ModelHandles.emplace_back(handle);

			for (int i = 0; i < g_InstanceCount; i++)
				// duplicate without marking instanced
				g_ModelHandles.push_back(currentScene.DuplicateModel(handle, false));
		}

	}

	void GameScript::OnUpdate()
	{
		Input();
		UpdateModelsNew(g_Engine->GetDeltaTime());
	}

	void GameScript::OnEndOfFrame()
	{

	}

	void TransformFunction(InstanceData& element, float& offset, float& yoffset, int& counter, float& angle)
	{
		if (counter % 50 == 0)
		{
			yoffset += 5.0f;
			offset = 0;
		}

		auto matrix = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f - yoffset, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 5.0f;
		counter++;

		element = matrix;
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
			if (counter % 50 == 0)
			{
				yOffset += 5.0f;
				offset = 0;
			}
			Model& model = scene.GetModel(modelHandle);
			if (model.IsInstanced())
				continue;
			if (model.IsHidden())
				break;

			auto& modelMatrix = model.GetModelMatrix();
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f - yOffset, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-g_Angle), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			offset += 5.0f;
			counter++;
		}

		Model& model = scene.GetModel(g_InstancedModel);
		if(!model.IsHidden())
			model.ApplyFunc(TransformFunction, offset, yOffset, counter, g_Angle);
		g_Angle += 10.0f * dt;
	}

	void Input()
	{
		Scene& scene = g_Engine->getActiveScene();
		bool* keys = g_Engine->window.getKeys();

		auto& models = scene.getModels();
		int instanceCount = scene.getModels().size();

		if (keys[GLFW_KEY_1] && g_KeyStateTracker ^ GLFW_KEY_1) // show all
		{
			scene.GetModel(0).SetIsHidden(true);
			for (int i = 1; i < instanceCount; i++)
			{
				models[i].SetIsHidden(false);
			}
		}

		if (keys[GLFW_KEY_2] && g_KeyStateTracker ^ GLFW_KEY_2) // non instanced
		{
			auto& Models = scene.getModels();
			scene.GetModel(g_InstancedModel).SetIsHidden(true);
		}
		if (keys[GLFW_KEY_3] && g_KeyStateTracker ^ GLFW_KEY_3) // instanced
		{
			auto& Models = scene.getModels();
			scene.GetModel(0).SetIsHidden(true);
			for (int i = 0; i < instanceCount; i++)
			{
				if (models[i].IsInstanced())
					continue;
				Models[i].SetIsHidden(true);
			}
		}
	}
}