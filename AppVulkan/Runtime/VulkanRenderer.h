// TODO: #define VK_check_result
#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Utilities.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "Window.h"
#include "Light.h"
#include "ShaderMan.h"
#include "Pipeline.h"
#include "Debug.h"
#include "Scene.h"
#include "Image.h"

//#define DEBUG_LOGS;
#define DEBUG

class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(std::string wName = "Default Window", const int width = 800, const int height = 600);

	void addModel(std::string fileName, Material material);
	std::vector<glm::mat4>* getModelsMatrices();
	//TODO: void LoadScene();

	void setupDebugMessenger();
	void draw(float dt);
	void cleanup();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	//GLFWwindow* window;//temp
	Window window;

private:

	int currentFrame = 0;

	//-- SCENE ---
	Scene currentScene;

	const std::vector<const char*> validationLayers = {//TODO: add more? - yes
"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkInstance instance;
	Device mainDevice;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffer;

	Image depthBufferImage;
	Image colorImage;

	ShaderMan shaderMan;


	//VkDeviceSize minUiformBufferOffset;
	//s/ize_t modelUniformAlignment;
	//Model* modelTransferSpace;

	//pipeline
	std::vector<Pipeline> Pipelines;
	///VkPipeline graphicsPipeline;
	//VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	//pools
	VkCommandPool graphicsCommandPool;

	//synch
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkFormat depthFormat;// not sure if this is alright

	void createInstance();
	void createLogicalDevice();
	void createSurface();
	// a queue of images that are waiting to be presented to the screen
	void createSwapChain();
	void createRenderPass();
	void createDepthBufferImage();
	void createColorResources();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronization();
	void createLight();
	void createScene();
	
	void compileShaders();

	//void allocateDynamicBufferTransferSpace();

	// record funcs
	void recordCommands(uint32_t currentImage);

	//getters
	void getPhysicalDevice();
	std::vector<const char*> getRequiredExtensions();
	VkSampleCountFlagBits getMaxUsableSampleCount();

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
	VkFormat chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags);
};

