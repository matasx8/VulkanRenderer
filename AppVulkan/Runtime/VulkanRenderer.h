#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "STB/stb_image.h"
#include "Utilities.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "Window.h"
#include "Light.h"
#include "ShaderMan.h"
#include "Pipeline.h"
#include "Debug.h"

//#define DEBUG_LOGS;

class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(std::string wName = "Default Window", const int width = 800, const int height = 600);

	void updateModel(int modelId, glm::mat4 newModel);
	int createMeshModel(std::string modelFile);

	void setupDebugMessenger();
	void draw(float dt);
	void cleanup();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	~VulkanRenderer();
	//GLFWwindow* window;//temp
	Window window;

private:

	Camera camera;
	std::vector<Light> lights;

	int currentFrame = 0;

	//scene objects
	std::vector<Model> modelList;

	//scene settings
	struct UboViewProjection {
		glm::mat4 projection;
		glm::mat4 view;
	} uboViewProjection;

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
	struct Device {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffer;

	VkImage depthBufferImage;
	VkDeviceMemory depthBufferImageMemory;
	VkImageView depthBufferImageView;
	VkSampler textureSampler;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	//assets
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;//would be better to have a single buffer, and images reference offsets
	std::vector<VkImageView> textureImageViews;

	//descriptors
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout samplerSetLayout;
	VkPushConstantRange pushConstantRange;
	VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> samplerDescriptorSets;


	std::vector<VkBuffer> vpUniformBuffer;
	std::vector<VkDeviceMemory> vpUniformBufferMemory;
	std::vector<VkBuffer> lightsUniformBuffer;
	std::vector<VkDeviceMemory> lightsUniformBufferMemory;
	std::vector<VkBuffer> cameraUniformBuffer;
	std::vector<VkDeviceMemory> cameraUniformBufferMemory;

	std::vector<VkBuffer> modelDUniformBuffer;
	std::vector<VkDeviceMemory> modelDUniformBufferMemory;

	//VkDeviceSize minUiformBufferOffset;
	//s/ize_t modelUniformAlignment;
	//Model* modelTransferSpace;

	//pipeline
	std::vector<Pipeline> Pipelines;
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
	VkFormat depthFormat;// not sure if this is alright

	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkSampleCountFlagBits numSamples, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createRenderPass();
	void createDescriptorSetLayout();
	void createPushConstantRange();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createDepthBufferImage();
	void createColorResources();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronization();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	int createTextureImage(std::string fileName);
	int createTexture(std::string fileName);
	void createTextureSampler();
	int createTextureDescriptor(VkImageView textureImage);
	void createCamera();
	void createLight();
	
	void compileShaders();

	void updateUniformBuffers(uint32_t index);

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

	// loader functions
	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
};
