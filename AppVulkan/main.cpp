#pragma once
#define STB_IMAGE_IMPLEMENTATION
#define DEBUG_FRAME_INFO
#include<iostream>
#include "VulkanRenderer.h"

VulkanRenderer vulkanRenderer;
float angle;

void updateModels(float dt)
{
	std::vector<glm::mat4>* Models = vulkanRenderer.getModelsMatrices();

	float offset = 0.0f;
	for (auto& model : *Models)
	{
		model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f + offset, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		offset += 50.0f;
	}
	
	angle += 10.0f * dt;
}

void addmodel(size_t i)
{
	Material initialMaterial;
	if(i % 5000 == 0)
		initialMaterial = Material("Shaders/shader2_vert.spv", "Shaders/shader2_frag.spv");
	else
		initialMaterial = Material("Shaders/shader_vert.spv", "Shaders/shader_frag.spv");
	vulkanRenderer.addModel("Models/12140_Skull_v3_L2.obj", initialMaterial);
}


int main()
{

	if (vulkanRenderer.init() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	//vulkanRenderer.addModel("Models/12140_Skull_v3_L2.obj");
	//int secondSkull = vulkanRenderer.createMeshModel("Models/12140_Skull_v3_L2.obj");
	size_t counter = 0;

	while (!glfwWindowShouldClose(vulkanRenderer.window.window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
#ifdef  DEBUG_FRAME_INFO
		Debug::FrameInfo(deltaTime);
#endif //  DEBUG_FRAME_INFO
		if (counter++ == 0 || counter % 2500 == 0)
			addmodel(counter);

		updateModels(deltaTime);

		vulkanRenderer.draw(deltaTime);
	}

	vulkanRenderer.cleanup();

	return 0;
}