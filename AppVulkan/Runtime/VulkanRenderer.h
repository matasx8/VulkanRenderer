// TODO: #define VK_check_result
// TODO: improve aftermath (dump in seperate folder that is untracked by repo)
// TODO: improve compiling by moving as many headers to cpp files
// TODO: implement or find a better job system impl
// TODO: if we don't find validation layers, don't use them
#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>
#include <thread>
#include <mutex>
#include <stdio.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <thread-pool/thread_pool.hpp>

#include "NsightAftermathGpuCrashTracker.h"

#include "RenderPass.h"
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
#include "DescriptorPool.h"
#include "InstanceDataBuffer.h"
#include "Containers/ModelManager.h"
#include "Containers/MaterialManager.h"
#include "OSUtilities.h"

//#define DEBUG_LOGS;
#define DEBUG

class VulkanRenderer // : NonCopyable
{
public:

	VulkanRenderer();

	int init(const RendererInitializationSettings& initSettings);

	Scene& getActiveScene() { return currentScene; }
	float GetDeltaTime() const { return m_DeltaTime; }
	thread_pool* const GetThreadPool() { return m_ThreadPool;}

	void setupDebugMessenger();
	void draw();
	void cleanup();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	Window window;

private:

	friend class ModelManager;
	friend class MaterialManager;

	int currentFrame = 0;
	float m_DeltaTime;
	float m_LastTime;

	//-- SCENE ---
	Scene currentScene;

	const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	bool enableValidationLayers = false;
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
	  
	RenderPassManager m_RenderPassManager;
	ModelManager m_ModelManager;

	Image depthBufferImage;
	Image colorImage;

	ShaderMan shaderMan;

	//pipeline
	std::vector<Pipeline> Pipelines;

	//pools
	VkCommandPool graphicsCommandPool;
	DescriptorPool m_DescriptorPool;
	std::vector<InstanceDataBuffer> m_InstancingBuffers;
	thread_pool* m_ThreadPool;

	GpuCrashTracker tracker;

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
	
	//void createRenderPass();
	void createDepthBufferImage();
	void createColorResources();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronization();
	void createLight();
	void CreateDescriptorPool();
	void CreateThreadPool(uint32_t numThreads);
	Image UploadImage(int width, int height, VkDeviceSize imageSize, stbi_uc* imageData);
	VkDescriptorSet CreateTextureDescriptorSet(const Texture& texture);
	VkDescriptorSetLayout CreateTextureDescriptorSetLayout();
	VkSampler CreateTextureSampler(const TextureCreateInfo& createInfo);

	void EnableCrashDumps();

	void TemporarySetup();

	void compileShaders();

	void UpdateDeltaTime();

	void LoadNode(std::vector<Mesh>& meshList, aiNode* node, const aiScene* scene);

	// record funcs
	void recordCommands(uint32_t currentImage);
	void recordingDefaultPath(int currentPipelineIndex, Model& model, int currentImage);

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

