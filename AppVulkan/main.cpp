#pragma once
#define STB_IMAGE_IMPLEMENTATION
//#define DEBUG_FRAME_INFO
#include<iostream>
#include "VulkanRenderer.h"

VulkanRenderer vulkanRenderer;
float angle;

void updateModels(float dt)
{
	std::vector<Model>* Models = vulkanRenderer.getModels();
	
	angle += 10.0f * dt;
	if (angle > 360)
		angle -= 360;//here!@#!@! fix rot
	//auto model = Models[0].getModel();
	auto model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	(*Models)[0].setModel(model);

	//model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 0.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	(*Models)[1].setModel(model);
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

	vulkanRenderer.addModel("Models/12140_Skull_v3_L2.obj");
	//int secondSkull = vulkanRenderer.createMeshModel("Models/12140_Skull_v3_L2.obj");

	while (!glfwWindowShouldClose(vulkanRenderer.window.window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
#ifdef  DEBUG_FRAME_INFO
		Debug::FrameInfo(deltaTime);
#endif //  DEBUG_FRAME_INFO

		updateModels(deltaTime);

		vulkanRenderer.draw(deltaTime);
	}

	vulkanRenderer.cleanup();

	return 0;
}