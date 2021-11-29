#include "VulkanRenderer.h"
#include <iostream>

VulkanRenderer::VulkanRenderer() 
{
    m_DeltaTime = 0;
    m_LastTime = 0;
    m_InstancingBuffers;
} // TODO: initialize variables

int VulkanRenderer::init(const RendererInitializationSettings& initSettings)
{
    if (window.Initialise(initSettings.windowName, initSettings.windowWidth, initSettings.windowHeight) == -1)
        throw std::runtime_error("Failed to initialize window!\n");

    try
    {
        EnableCrashDumps();
        compileShaders();
        createInstance();
        CreateThreadPool(initSettings.numThreadsInPool);
        createSurface();
        setupDebugMessenger();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createColorResources();
        createDepthBufferImage();
        createRenderPass();
        createCommandPool();
        createFramebuffers();
        createCommandBuffers();
        // default model, pipeline, components
        CreateDescriptorPool();
        createInitialScene();
        createSynchronization();

    }
    catch (const std::runtime_error& e)
    {
        printf("ERROR: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanRenderer::draw()
{
    UpdateDeltaTime();

    currentScene.cameraKeyControl(window.getKeys(), m_DeltaTime);
    currentScene.cameraMouseControl(window.getXChange(), window.getYchange());

    //wait for given fence to signal open from last draw before xontinuing
    vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    //manually reset close fences
    vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

    // get the next available image to draw to and set something to signal when were finished with the image
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    recordCommands(imageIndex);
    //change the VP ubo here

    currentScene.updateScene(imageIndex);

    // submit command bufferto queue for execution, making sure it waits for the image to be signalled as available before drawing
    //and signals when it has finished rendering
    //queue submission info
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailable[currentFrame]; // list of semaphores to wait one
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages; // stages to check semaphores at
    submitInfo.commandBufferCount = 1; //number of command buffers to submit
    submitInfo.pCommandBuffers = &commandBuffer[imageIndex];
    submitInfo.signalSemaphoreCount = 1; // number of semaphores to signal
    submitInfo.pSignalSemaphores = &renderFinished[currentFrame]; //semaphores to signal when command buffer finishes -- finished rendering

    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit Command Buffer to Queue!");
    }
    // present image to screen when it has signlled finished rendering
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;  
    presentInfo.pWaitSemaphores = &renderFinished[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain; // swapchain to present images to
    presentInfo.pImageIndices = &imageIndex; //index of images in swapchain to present

    result = vkQueuePresentKHR(presentationQueue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present image");
    }

    //get next frame
    currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
    currentScene.onFrameEnded();
}

void VulkanRenderer::cleanup()
{
    //wait until no actions being run on device
    vkDeviceWaitIdle(mainDevice.logicalDevice);

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
        vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
    }
    vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);
    for (auto& framebuffer : swapchainFramebuffers)
    {
        vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
    }

    m_DescriptorPool.DestroyDescriptorPool();
    
    for (auto& buffer : m_InstancingBuffers)
    {
        buffer.Destroy();
    }

    currentScene.CleanUp(mainDevice.logicalDevice);
    depthBufferImage.destroyImage(mainDevice.logicalDevice);
    colorImage.destroyImage(mainDevice.logicalDevice);
    for (auto& pipe : Pipelines)//!HERE
    {
        pipe.CleanUp(mainDevice.logicalDevice);
    }

    vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);
    for (auto& image : swapChainImages)
    {
        vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
    }

    vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);

    delete m_ThreadPool;
}

VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanRenderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanRenderer::createInstance()
{
    //validation
#ifndef NDEBUG
    enableValidationLayers = enableValidationLayers ? checkValidationLayerSupport() : false;
#endif

    //info about the application itself
    //most data here doesnt affect the program and is for the developer convenience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App"; //custom name of the app
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//CUSTOM VERSION OF THE APP
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2; //VULKAN VER

    //creation information for a vkinstane
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    //createInfo.pNext point to extended information
    createInfo.pApplicationInfo = &appInfo;

    // create list to hold instance extensions
    std::vector<const char*> instanceExtensions = std::vector<const char*>();

    //set up extension instance will use
    uint32_t glfwExtensionCount = 0; //glfw may require multiple extensions
    const char** glfwExtensions;

    //get glfw extensions
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //add glfw extensions to list of extension
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
        instanceExtensions.push_back(glfwExtensions[i]);
    }
    if (enableValidationLayers) 
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }


    // check instance extensions supported
    if (!checkInstanceExtensionSupport(&instanceExtensions))
    {
        throw std::runtime_error("vkinstnce does not support required extensions");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    //create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Vulkan instance");
    }
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

//format: vk_format_G8G8B8A8_UNORM
//colorspace: VK_COLOR_SPACE_SRGB8_NONLINEAR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return { VK_FORMAT_R8G8B8A8_UNORM,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& format : formats)
    {
        if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
    for (const auto& presentationMode : presentationModes)
    {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentationMode;
        }
    }

    //must be present so just in case
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
    //if current extent is at numeric limits, then extent can vary. otherwise its the ize of the window
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return surfaceCapabilities.currentExtent;
    }
    else
    {//if value can vary, need to set manually
        int width, height;//get window size
        height = (int)window.getBufferHeigt();
        width = (int)window.getBufferWidth();
        //glfwGetFramebufferSize(window, &width, &height);

        //create new extent using window size
        VkExtent2D newExtent = {};
        newExtent.width = static_cast<uint32_t>(width);
        newExtent.height = static_cast<uint32_t>(height);

        // surface also defines max and min, so make sure within boundries by clamping value
        newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
        newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
        
        return newExtent;
    }
}

VkFormat VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
    // loop through the options and find compatible one
    for (VkFormat format : formats)
    {
        // get properties for given format on this device
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format, &properties);

        //depending on tiling choice, need to check for proper bit flag
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find a matching format!");
}


void VulkanRenderer::createRenderPass()
{
    // colour attachment of render pass
    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = swapChainImageFormat; // format to use for attachment
    colourAttachment.samples = msaaSamples; // number of samples to write or msaa
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //framebuffer data will be stored as an image, but images can be given different data layouts
    //to give optimal use for certain operations
    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // image data layout before render pass starts
    // with msaa can't be presented directly. We first need to resolve them to a regular image
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // image data layout after render pass (to change to

    // depth attachment of render pass
    VkAttachmentDescription depthAttachment = { };
    depthAttachment.format = depthFormat; //make sure createDepth.. is called before createRenderpass
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Resolve attachment
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT; // no msaa because it's for presenting
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //attachment reference uses an attachment index that refers to index in the attachment list passed to render passcreateinfo
    VkAttachmentReference colourAttachmentReference = {};
    colourAttachmentReference.attachment = 0;
    colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveReference = {};
    colorAttachmentResolveReference.attachment = 2;
    colorAttachmentResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //info about a particular subpass the render pass is using
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //pipeline type subpass is to be bound to
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colourAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    subpass.pResolveAttachments = &colorAttachmentResolveReference; 

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> renderPassAttachments = { colourAttachment, depthAttachment, colorAttachmentResolve };

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
    renderPassCreateInfo.pAttachments = renderPassAttachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Render Pass");
    }
}

void VulkanRenderer::createDepthBufferImage()
{
    //get supported format for depth buffer
    depthFormat = chooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    //create depth buffer image


    depthBufferImage.createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mainDevice.physicalDevice, mainDevice.logicalDevice);

    depthBufferImage.createImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, mainDevice.logicalDevice);
}

void VulkanRenderer::createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;


    colorImage.createImage(swapChainExtent.width, swapChainExtent.height, colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
        msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mainDevice.physicalDevice, mainDevice.logicalDevice);//hmm
    colorImage.createImageView(colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, mainDevice.logicalDevice);
}

void VulkanRenderer::createFramebuffers()
{
    swapchainFramebuffers.resize(swapChainImages.size());

    // create a framebuffer for each swapchain image
    for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
    {
        std::array<VkImageView, 3> attachments =
        {
                            colorImage.getImageView(),
            depthBufferImage.getImageView(),
                        swapChainImages[i].imageView
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass; //render pass layout the framebuffer will be used with
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data(); //list of attachments (1:1 with render pass)
        framebufferCreateInfo.width = swapChainExtent.width; //framebuffer width
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create a Framebuffer");
        }
    }
}

void VulkanRenderer::createCommandPool()
{
    //get indices of queue families from device
    QueueFamilyIndices queueFamilyIndices = getQueueFamilies(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // queuefamily type that buffers from this command pool will use

    //create a graphics queue family command pool
    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &graphicsCommandPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a command pool");
    }
}

void VulkanRenderer::createCommandBuffers()
{
    //resize command buffer count to have one for each fb
    commandBuffer.resize(swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo cbAllocInfo = {};
    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = graphicsCommandPool; // buffer secondary cant be called directly, can be called from other buffers
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // buffer you submint directly to queue, cant be called by other buffers
    cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());
    //allocate command buffers and place handles in array of buffers
    VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &cbAllocInfo, commandBuffer.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Command Buffers");
    }
}

void VulkanRenderer::createSynchronization()
{
    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);

    //semaphore creation information
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //fence creation info
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinished[i]) ||
            vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]))
        {
            throw std::runtime_error("Failed to create semaphore and/or fence");
        }
    }
}


void VulkanRenderer::createLight()
{
    //create a light. Currently we will only use this one but I should add support for multiple lights later
    currentScene.addLight();
}

void VulkanRenderer::createInitialScene()
{
    shaderMan.WaitForCompile();

    currentScene = Scene(graphicsQueue, graphicsCommandPool, mainDevice.physicalDevice, mainDevice.logicalDevice,swapChainImages.size(), swapChainExtent, msaaSamples, &m_DescriptorPool);

    currentScene.addLight();

    //initial model
    ShaderCreateInfo shaderInfo = { "Shaders/shader_vert.spv", "Shaders/shader_frag.spv" };
    shaderInfo.uniformCount = 3;

    std::vector<UniformData> UniformDatas(3);
        UniformDatas[0].name = "ViewProjection uniform";
        UniformDatas[0].sizes = { sizeof(ViewProjectionMatrix) };
        UniformDatas[0].dataBuffers = { currentScene.getViewProjectionPtr() };

        Light& light = currentScene.getLight(0);
        UniformDatas[1].name = "Light uniform";
        UniformDatas[1].sizes = { sizeof(glm::vec4), sizeof(glm::vec4) };
        UniformDatas[1].dataBuffers = { &light.m_Position, &light.m_Colour };

        Camera& camera = currentScene.getCamera();
        UniformDatas[2].name = "Camera";
        UniformDatas[2].sizes = { sizeof(glm::vec4) };
        UniformDatas[2].dataBuffers = { &camera.getCameraPosition() };

    shaderInfo.uniformData = std::move(UniformDatas);
    shaderInfo.pushConstantSize = 0;
    shaderInfo.shaderFlags = kUseModelMatrixForPushConstant;
    shaderInfo.isInstanced = false;
    Material initialMaterial = Material(shaderInfo);
    currentScene.AddModel("Models/12140_Skull_v3_L2.obj", initialMaterial, renderPass);
}

void VulkanRenderer::CreateDescriptorPool()
{
    m_DescriptorPool.CreateDescriptorPool(mainDevice.logicalDevice);
}

void VulkanRenderer::CreateThreadPool(uint32_t numThreads)
{
    m_ThreadPool = new thread_pool(numThreads);
}

void VulkanRenderer::EnableCrashDumps()
{
    tracker.Initialize();
}


void VulkanRenderer::compileShaders()
{
    shaderMan.CompileShadersAsync();
}

void VulkanRenderer::UpdateDeltaTime()
{
    float now = glfwGetTime();
    m_DeltaTime = now - m_LastTime;
    m_LastTime = now;
}

void VulkanRenderer::AddModel(std::string fileName, Material material)
{
    currentScene.AddModel(fileName, material, renderPass);
}

void VulkanRenderer::DrawInstanced(int index, uint32_t currentImage)
{
    auto& Models = currentScene.getModels();
    int currentPipelineIndex = static_cast<int>(Models[index].getPipelineIndex());
    Pipeline& gfxPipeline = currentScene.getPipeline(currentPipelineIndex);
    int meshCount = Models[index].getMeshCount();

    if (gfxPipeline.hasPushConstant())
    {
        uint32_t size = gfxPipeline.getPushConstantSize();
        const void* pushDataBuffer = gfxPipeline.getPushConstantDataBuffer();
        vkCmdPushConstants(commandBuffer[currentImage], gfxPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, size, pushDataBuffer);    
    }

    VkDeviceSize offsets[] = { 0 }; //offsets into buffers being bound
    VkBuffer instanceBuffers[] = { m_InstancingBuffers[currentImage].GetInstanceData() };
    vkCmdBindVertexBuffers(commandBuffer[currentImage], 1, 1, instanceBuffers, offsets);
    // try to bind instance buffer once
    for (size_t i = 0; i < Models[index].getMeshCount(); i++)
    {
        VkBuffer vertexBuffers[] = { Models[index].getMesh(i)->getVertexBuffer() };
        
        vkCmdBindVertexBuffers(commandBuffer[currentImage], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer[currentImage], Models[index].getMesh(i)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        auto descriptorSet = gfxPipeline.getDescriptorSet(currentImage);
        int texId = Models[index].getMesh(i)->getTexId();
        auto textureDescriptorSet = currentScene.getTextureDescriptorSet(texId);
        std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSet, textureDescriptorSet };

        // bind descriptor sets
        vkCmdBindDescriptorSets(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.getPipelineLayout(),
            0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);// will only apply offset to descriptors that are dynamic
        vkCmdDrawIndexed(commandBuffer[currentImage], Models[index].getMesh(i)->getIndexCount(), m_InstancingBuffers[currentImage].GetElementCount(), 0, 0, 0);
    }
    m_InstancingBuffers[currentImage].Reset();
}

void VulkanRenderer::recordCommands(uint32_t currentImage)
{
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //information about how to begin a render pass (only needed for graphical applicatos)
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = { 0, 0 }; //start point of render pass in pixels
    renderPassBeginInfo.renderArea.extent = swapChainExtent; // size of region t run render pass
    
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.5f, 0.65f, 0.4f, 1.0f };
    clearValues[1].depthStencil.depth = 1.0f;
    renderPassBeginInfo.pClearValues = clearValues.data(); // list of clear values
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

    renderPassBeginInfo.framebuffer = swapchainFramebuffers[currentImage];

    VkResult result = vkBeginCommandBuffer(commandBuffer[currentImage], &bufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to start recording a Command Buffer");
    }

    vkCmdBeginRenderPass(commandBuffer[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    std::vector<Model>& Models = currentScene.getModels();
    int lastPipelineIndex = -1;

    for(size_t i = 0; i < Models.size(); i++)
    {
        Model& model = Models[i];
        if (model.IsHidden())
            continue;


        int currentPipelineIndex = model.getPipelineIndex();
        // pipeline indices should be sorted
        if (currentPipelineIndex > lastPipelineIndex)
        {
            VkPipeline newPipeline = currentScene.getPipeline(currentPipelineIndex).getPipeline();
            vkCmdBindPipeline(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, newPipeline);
            lastPipelineIndex = currentPipelineIndex;
        }
#ifdef DEBUG
        else if (currentPipelineIndex < lastPipelineIndex)
            throw std::runtime_error("Current pipeline index was lesser than old one. This indicates Model vector was not sorted.");
#endif
            recordingDefaultPath(currentPipelineIndex, model, currentImage);
    }

    vkCmdEndRenderPass(commandBuffer[currentImage]);

    //stop recording to command buffer
    result = vkEndCommandBuffer(commandBuffer[currentImage]);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to stop recording a Command Buffer");
    }
}

void VulkanRenderer::recordingDefaultPath(int currentPipelineIndex, Model& model, int currentImage)
{
        Pipeline& gfxPipeline = currentScene.getPipeline(currentPipelineIndex);
        VkPipelineLayout pipelineLayout = gfxPipeline.getPipelineLayout();

        const auto& modelMatrix = model.GetModelMatrix();

        // TODO: was used last frame?? if yes maybe we don't need to bind.. maybe we don't need to bind a lot of things also?
        if (gfxPipeline.hasPushConstant())
        {
            if (gfxPipeline.useModelMatrixForPushConstant())
            {
                uint32_t size = sizeof(modelMatrix);
                const void* pushDataBuffer = &modelMatrix;
                vkCmdPushConstants(commandBuffer[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, pushDataBuffer);
            }
            else
            {// shouldn't be able to have a size that exceeds gfx caps
                uint32_t size = gfxPipeline.getPushConstantSize();
                const void* pushDataBuffer = gfxPipeline.getPushConstantDataBuffer();
                vkCmdPushConstants(commandBuffer[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, pushDataBuffer);
            }
        }

        // If model is instanced, bind instacing data
        if (model.IsInstanced())
        {
            VkDeviceSize offsets[] = { 0 };
            VkBuffer instanceBuffers[] = { model.GetInstanceData() };
            vkCmdBindVertexBuffers(commandBuffer[currentImage], 1, 1, instanceBuffers, offsets);
        }

        for (size_t k = 0; k < model.getMeshCount(); k++)
        {
            VkBuffer vertexBuffers[] = { model.getMesh(k)->getVertexBuffer() }; //buffers to bind
            VkDeviceSize offsets[] = { 0 }; //offsets into buffers being bound
            vkCmdBindVertexBuffers(commandBuffer[currentImage], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer[currentImage], model.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);


            // TODO: VVVVVVVVVVVVVVVVVVVVVVVVVVVVV bad
            auto descriptorSet = gfxPipeline.getDescriptorSet(currentImage);
            int texId = model.getMesh(k)->getTexId();
            auto textureDescriptorSet = currentScene.getTextureDescriptorSet(texId);
            std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSet, textureDescriptorSet };

            // bind descriptor sets
            vkCmdBindDescriptorSets(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);// will only apply offset to descriptors that are dynamic

            vkCmdDrawIndexed(commandBuffer[currentImage], model.getMesh(k)->getIndexCount(), model.GetInstanceCount(), 0, 0, 0);
        }
}

void VulkanRenderer::createLogicalDevice()
{
    //get the queue family indices for the chosen physcial device
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    //vector for queue creation information and set for family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

    //queue the logical device needs to create and info to do so
    for (int queueFamilyIndex : queueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex; //index of the graphics family to create a queue from
        queueCreateInfo.queueCount = 1;
        float priority = 1.0f;
        queueCreateInfo.pQueuePriorities = &priority; //vulkan needs to know multiple queues, so dice priority ( 1 = highest
   
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceDiagnosticsConfigCreateInfoNV dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV;
    dci.flags = VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |      // Enable tracking of resources.
        VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV |  // Capture call stacks for all draw calls, compute dispatches, and resource copies.
        VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV;

    //information to create logical device
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); //number of queue create infos
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &dci;

    VkPhysicalDeviceFeatures devicefeatures = {};
    devicefeatures.samplerAnisotropy = VK_TRUE;//just because its set, doesnt mean we support it - have to check

    deviceCreateInfo.pEnabledFeatures = &devicefeatures; //physical device features logical device will use

    VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a logical device!");
    }

    //queues are created at the same time as the device
    //so we want to handle to queues
    //from given logical device of given queu fam, of given queue index, place refference in given vkqueue
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);

    // TODO: make the device accessible globally and actually safe
    s_DevicePtr = mainDevice;
}

void VulkanRenderer::createSurface()
{
    //creating a surface create info struct specific to OS
    VkResult result = glfwCreateWindowSurface(instance, window.window, nullptr, &surface);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a surface!");
    }
}

void VulkanRenderer::createSwapChain()
{
    //get swapchain details so we can pick the best settings
    SwapChainDetails swapChainDetails = getSwapChainDetails(mainDevice.physicalDevice);

    // 1choose best surface format
    VkSurfaceFormatKHR surfaceFormats = chooseBestSurfaceFormat(swapChainDetails.formats);
    // 2 chose best pres. mode
    VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
    // 3 choose swap chain image resolution
    VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);
    
    // how many images are in the swap chain? get 1 more than the min to allow triple buffering
    uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

    //if 0 then limitless
    if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
    {
        imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
    }

    //creatinion info for swapchai
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.imageFormat = surfaceFormats.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormats.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageArrayLayers = 1; // number of layers for each image in chain
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform; //transform to perform on swapchain images
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // how to handle blending images
    swapChainCreateInfo.clipped = VK_TRUE;

    //get queue family indices
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    //if graphics and presentation families are different, then swapchain must let images be shared between families
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        uint32_t queueFamilyIndices[] = {
            (uint32_t)indices.graphicsFamily,
            (uint32_t)indices.presentationFamily
        };

        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //image share handling
        swapChainCreateInfo.queueFamilyIndexCount = 2; //number of queues to share between
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; //array of queues to share between
    }
    else
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    //if old swap chain being destroyed and this one replaces it
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a swapchain");
    }

    swapChainImageFormat = surfaceFormats.format;
    swapChainExtent = extent;

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, nullptr);
    std::vector<VkImage> images(swapChainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, images.data());

    for (VkImage image : images)
    {
        //store image handle
        SwapChainImage swapchainImage = {};
        
        swapchainImage.image = image;
        swapchainImage.imageView = Image::createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mainDevice.logicalDevice);

        swapChainImages.push_back(swapchainImage);
    }
}


void VulkanRenderer::getPhysicalDevice()
{
    //enumerate physical devices the vkinstance can acess
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    //if no devices available then none support vulkan
    if (deviceCount == 0)
    {
        throw std::runtime_error("Can't find GPUs that support Vulkan instance");
    }

    //get list of physical devices
    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

    //just pick physical device
    mainDevice.physicalDevice = deviceList[0];

    for (const auto& device : deviceList)
    {
        if (checkDeviceSuitable(device))
        {
            mainDevice.physicalDevice = device;
            msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    // get properties of our new device
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);

    //minUiformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkSampleCountFlagBits VulkanRenderer::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
    //need to get number of extensions for querying extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    //check if given extensions are in list of available extensions
    //note find out if this is a right way to loop over
    for (const auto& checkExtension : *checkExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(checkExtension, extension.extensionName))
            {
                hasExtension = true;
                break;
            }
        }

        if (!hasExtension)
        {
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    if (extensionCount == 0)
    {
        return false;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    for (const auto& deviceExtension : deviceExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(deviceExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }

        if (!hasExtension)
        {
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device)
{
    
    //info about device itself
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
    //info about what the device can do
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    QueueFamilyIndices indices = getQueueFamilies(device);

    bool extensSupported = checkDeviceExtensionSupport(device);

    bool swapChainValid = false;

    if (extensSupported)
    {
        SwapChainDetails swapChainDetails = getSwapChainDetails(device);
        swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
    }

    return indices.isValid() && extensSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            Debug::LogMsg("Warning: No available validaiton layers found!\n");
            return false;
        }
    }

    return true;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

    int i = 0;
    //go through each queue family and check if it has at least one of the require types of queue
    for (const auto& queueFamily : queueFamilyList)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        //check if queue family supports presentatin
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
        if (queueFamily.queueCount > 0 && presentationSupport)
        {
            indices.presentationFamily = i;
        }

        if (indices.isValid())
        {
            break;
        }
        i++;
    }

    return indices;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
    SwapChainDetails swapChainDetails;

    //capabilities, get the surface cpaabilities for the given surface on the given phsyical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    //if formats returned, get list of formats
    if (formatCount != 0)
    {
        swapChainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
    }

    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

    if (presentationCount != 0)
    {
        swapChainDetails.presentationModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
    }

    return swapChainDetails;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    if (messageSeverity >= 256) {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}
