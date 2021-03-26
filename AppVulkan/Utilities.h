#pragma once
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
//indices (locations) of queue families if they exist
struct QueueFamilyIndices
{
	int graphicsFamily = -1; //location of graphics q family
	int presentationFamily = -1;

	//check if queue families are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities; //surface properties
	std::vector<VkSurfaceFormatKHR> formats;      //surface image formats, eg rgba and size of ea color
	std::vector<VkPresentModeKHR> presentationModes;
};