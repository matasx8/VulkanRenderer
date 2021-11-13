//TODO: rename member variables to m_VariableName
//TODO: make physical and logical device accessible everywhere
//TODO: must create a data structure for Models!! currently this is a huge bottleneck
//		- make it so its fast to retrieve a model
//		- make it data oriented

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include<iostream>
#include "VulkanRenderer.h"
#include "GameDir/Gamescript.h"

VulkanRenderer vulkanRenderer;

int main()
{

	if (vulkanRenderer.init() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	glfwSwapInterval(1);

	GameScript::OnStart(&vulkanRenderer);

	while (!glfwWindowShouldClose(vulkanRenderer.window.window))
	{
		glfwPollEvents();

#ifdef  DEBUG_FRAME_INFO
		Debug::FrameInfo(vulkanRenderer.GetDeltaTime());
#endif 

		GameScript::OnUpdate();

		vulkanRenderer.draw();

		GameScript::OnEndOfFrame();
	}

	vulkanRenderer.cleanup();

	return 0;
}