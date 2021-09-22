#pragma once
#include "VulkanRenderer.h"

// Get debug information printed to console
#define DEBUG_FRAME_INFO

namespace GameScript
{

	extern VulkanRenderer* g_Engine;

	// This function is called before the first frame
	void OnStart(VulkanRenderer* engine);

	// This function is called before every frame
	void OnUpdate();

	// This function is called after every frame
	void OnEndOfFrame();
}