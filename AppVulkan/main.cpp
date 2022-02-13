//TODO: rename member variables to m_VariableName
// TODO: make stuff non copyable

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include<iostream>
#include "VulkanRenderer.h"
#include "GameDir/Gamescript.h"

VulkanRenderer vulkanRenderer;

int main()
{
	RendererInitializationSettings initSettings = {};
	GameScript::OnInitialize(initSettings);

	if (vulkanRenderer.init(initSettings) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	glfwSwapInterval(1);

	GameScript::OnStart(&vulkanRenderer);

	while (!glfwWindowShouldClose(vulkanRenderer.window.m_WindowPtr))
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