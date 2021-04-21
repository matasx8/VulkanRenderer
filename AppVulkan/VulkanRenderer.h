#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Utilities.h"
#include "Mesh.h"

class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(GLFWwindow* newWindow);
	void setupDebugMessenger();
	void draw();
	void cleanup();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	
	~VulkanRenderer();

private:
	GLFWwindow* window;

	int currentFrame = 0;

	//scene objects
	std::vector<Mesh> meshList;

	const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffer;

	//pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	//pools
	VkCommandPool graphicsCommandPool;

	//synch
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;


	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createRenderPass();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronization();

	// record funcs
	void recordCommands();

	//getters
	void getPhysicalDevice();
	std::vector<const char*> getRequiredExtensions();

	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkDeviceSuitable(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

};

