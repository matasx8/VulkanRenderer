#pragma once
#define STB_IMAGE_IMPLEMENTATION
#define DEBUG_FRAME_INFO
#include<iostream>
#include "VulkanRenderer.h"

VulkanRenderer vulkanRenderer;


int main()
{

	if (vulkanRenderer.init() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	int ind = vulkanRenderer.createMeshModel("Models/12140_Skull_v3_L2.obj");
	int secondSkull = vulkanRenderer.createMeshModel("Models/12140_Skull_v3_L2.obj");

	while (!glfwWindowShouldClose(vulkanRenderer.window.window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
#ifdef  DEBUG_FRAME_INFO
		Debug::FrameInfo(deltaTime);
#endif //  DEBUG_FRAME_INFO

		angle += 10.f * deltaTime;
		if (angle > 360.0f) { angle -= 360.0f; }

		glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		testMat = glm::rotate(testMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		vulkanRenderer.updateModel(ind, testMat);

		glm::mat4 testMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 0.0f));
		testMat2 = glm::rotate(testMat2, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
		testMat2 = glm::rotate(testMat2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		vulkanRenderer.updateModel(secondSkull, testMat2);

		vulkanRenderer.draw(deltaTime);
	}

	vulkanRenderer.cleanup();

	return 0;
}