#pragma once
#include "GameScript.h"

namespace GameScript
{
	void updateModels(float dt);

	VulkanRenderer* g_Engine;

	float angle = 0.0f;

	void GameScript::OnStart(VulkanRenderer* engine)
	{
		if (engine)
			g_Engine = engine;
		else
			throw std::runtime_error("The Engine was passed as nullptr!");

		Scene& currentScene = g_Engine->getActiveScene();

		ShaderCreateInfo shaderInfo = { "Shaders/shader2_vert.spv", "Shaders/shader2_frag.spv" };
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
		Material material1 = Material(shaderInfo);
		g_Engine->addModel("Models/Old House 2 3D Models.obj", material1);
		g_Engine->addModel("Models/Old House 2 3D Models.obj", material1);
		g_Engine->addModel("Models/Old House 2 3D Models.obj", material1);
		g_Engine->addModel("Models/Old House 2 3D Models.obj", material1);
		g_Engine->addModel("Models/Old House 2 3D Models.obj", material1);
		g_Engine->addModel("Models/Group4.obj", material1);
	}

	void GameScript::OnUpdate()
	{
		updateModels(g_Engine->GetDeltaTime());
	}

	void GameScript::OnEndOfFrame()
	{

	}

	void updateModels(float dt)
	{
		std::vector<glm::mat4>* Models = g_Engine->getModelsMatrices();

		float offset = 0.0f;
		auto& model = (*Models)[0];
		model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		auto& model2 = (*Models)[1];
		model2 = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f + offset, 0.0f, 0.0f));

		model2 = glm::rotate(model2, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 50.0f;

		auto& model3 = (*Models)[2];
		model3 = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f + offset, 200.0f, 0.0f));
		////model2 = glm::rotate(model2, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model3 = glm::rotate(model3, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 50.0f;

		auto& model4 = (*Models)[3];
		model4 = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f + offset, 200.0f, 0.0f));
		////model2 = glm::rotate(model2, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model4 = glm::rotate(model4, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 50.0f;

		auto& model5 = (*Models)[4];
		model5 = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f + offset, 200.0f, 0.0f));
		////model2 = glm::rotate(model2, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model5 = glm::rotate(model5, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 50.0f;

		auto& model6 = (*Models)[5];
		model6 = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f + offset, 200.0f, 0.0f));
		////model2 = glm::rotate(model2, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model6 = glm::rotate(model6, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		offset += 50.0f;
		angle += 10.0f * dt;
	}
}