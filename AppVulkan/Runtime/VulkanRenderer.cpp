#include "VulkanRenderer.h"
#include <iostream>

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(std::string wName, const int width, const int height)
{
    if (window.Initialise(wName, width, height) == -1)
        throw std::runtime_error("Failed to initialize window!\n");

    try
    {
        compileShaders();
        createInstance();
        createSurface();
        setupDebugMessenger();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createColorResources();
        createDepthBufferImage();
        createRenderPass();
        createDescriptorSetLayout();
        createPushConstantRange();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createTextureSampler();
        createCamera();
        createLight();
        //allocateDynamicBufferTransferSpace();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSynchronization();

        uboViewProjection.projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);
        uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 30.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        uboViewProjection.projection[1][1] *= -1;//invert matrix

        createTexture("plain.png");//fallback

    }
    catch (const std::runtime_error& e)
    {
        printf("ERROR: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel)
{
    if (modelId >= modelList.size())
        return;
    modelList[modelId].setModel(newModel);
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

void VulkanRenderer::draw(float dt)
{
    //update camera?
    camera.keyControl(window.getKeys(), dt);
    camera.mouseControl(window.getXChange(), window.getYchange());
#ifdef DEBUG_LOGS
    Debug::Log(camera);
#endif // DEBUG_LOG

   // lights[0].debugInput(window.getKeys(), dt);
    lights[0].debugFollowCam(camera.getCameraPosition(), glm::vec3(0.0f, -90.0f, 0.0f));
    //wait for given fence to signal open from last draw before xontinuing
    vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    //manually reset close fences
    vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

    // get the next available image to draw to and set something to signal when were finished with the image
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    recordCommands(imageIndex);
    //change the VP ubo here

    updateUniformBuffers(imageIndex);

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
}

void VulkanRenderer::cleanup()
{
    //wait until no actions being run on device
    vkDeviceWaitIdle(mainDevice.logicalDevice);

    for (size_t i = 0; i < modelList.size(); i++)
    {
        modelList[i].destroyMeshModel();
    }

    vkDestroyDescriptorPool(mainDevice.logicalDevice, samplerDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, samplerSetLayout, nullptr);

    vkDestroySampler(mainDevice.logicalDevice, textureSampler, nullptr);

    //_aligned_free(modelTransferSpace);
    for (size_t i = 0; i < textureImages.size(); i++)
    {
        vkDestroyImageView(mainDevice.logicalDevice, textureImageViews[i], nullptr);
        vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, textureImageMemory[i], nullptr);
    }

    vkDestroyImageView(mainDevice.logicalDevice, depthBufferImageView, nullptr);
    vkDestroyImage(mainDevice.logicalDevice, depthBufferImage, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, depthBufferImageMemory, nullptr);

    vkDestroyImageView(mainDevice.logicalDevice, colorImageView, nullptr);
    vkDestroyImage(mainDevice.logicalDevice, colorImage, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, colorImageMemory, nullptr);

    vkDestroyDescriptorPool(mainDevice.logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, descriptorSetLayout, nullptr);
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vkDestroyBuffer(mainDevice.logicalDevice, vpUniformBuffer[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, vpUniformBufferMemory[i], nullptr);
        vkDestroyBuffer(mainDevice.logicalDevice, lightsUniformBuffer[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, lightsUniformBufferMemory[i], nullptr);
        vkDestroyBuffer(mainDevice.logicalDevice, cameraUniformBuffer[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, cameraUniformBufferMemory[i], nullptr);
       // vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
       // vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
    }

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
    for(auto& pipe : Pipelines)
        vkDestroyPipeline(mainDevice.logicalDevice, pipe.getPipeline(), nullptr);
    vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
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

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::createInstance()
{
    //validation
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    //info about the application itself
    //most data here doesnt affect the program and is for the developer convenience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App"; //custom name of the app
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//CUSTOM VERSION OF THE APP
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0; //VULKAN VER

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

    //std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    //if (enableValidationLayers) {
    //    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //}

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

stbi_uc* VulkanRenderer::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
    // number of channels image uses
    int channels;

    //load pixel data for image
    std::string fileLoc = "Textures/" + fileName;
    stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        throw std::runtime_error("Failed to load a Texture file: " + fileName + "\n");
    }

    *imageSize = *width * *height * 4;
    return image;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image; // image to create view for
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format; //format of image data
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //allows remapping of rgba components to other rgba vlues
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    //subresources allow the view to view only a part of an image
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags; //which aspect of image to view (e.g. color_bit for viewing color)
    viewCreateInfo.subresourceRange.baseMipLevel = 0; //start mipmap level to view from
    viewCreateInfo.subresourceRange.levelCount = 1; //number of mipmap levels to view
    viewCreateInfo.subresourceRange.baseArrayLayer = 0; //start array level to view from
    viewCreateInfo.subresourceRange.layerCount = 1; //number of array levels to view

    VkImageView imageView;
    VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image view");
    }

    return imageView;
}

void VulkanRenderer::createRenderPass()
{
    // colour attachment of render pass
    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = swapChainImageFormat; // format to use for attachment
    colourAttachment.samples = msaaSamples; // number of samples to write or msaa
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
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

    // need to determine when layout transitions occur using subpass dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies;

    //conversion from vkimagelayoutundefined to vk image layout color attachment..
    //TRANSITION must happen after..
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //but must happen before
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;

    //subpass to presentation
    //TRANSITION must happen after..
    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //but must happen before
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;

    std::array<VkAttachmentDescription, 3> renderPassAttachments = { colourAttachment, depthAttachment, colorAttachmentResolve };

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
    renderPassCreateInfo.pAttachments = renderPassAttachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassCreateInfo.pDependencies = subpassDependencies.data();

    VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a Render Pass");
    }
}

void VulkanRenderer::createDescriptorSetLayout()
{
    //uniform values descripor set layout
    // uboViewProjection binding info
    VkDescriptorSetLayoutBinding vpLayoutBinding = {};
    vpLayoutBinding.binding = 0; // binding point in shader
    vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor (uniform, dynamic uniform)
    vpLayoutBinding.descriptorCount = 1; // number of descriptors for bidning
    vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // shader stage to bind to
    vpLayoutBinding.pImmutableSamplers = nullptr; // for texture- can make the sampler immutable, the imageview it samples from can still be changed

    VkDescriptorSetLayoutBinding lightsLayoutBinding = {};
    lightsLayoutBinding.binding = 1;
    lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightsLayoutBinding.descriptorCount = 1;
    lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    lightsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding cameraLayoutBinding = {};
    cameraLayoutBinding.binding = 2;
    cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    cameraLayoutBinding.pImmutableSamplers = nullptr;
    

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding, lightsLayoutBinding, cameraLayoutBinding };

    // create descriptor set layout with given bidnings
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size()); // number of binding infos
    layoutCreateInfo.pBindings = layoutBindings.data(); //array of binding infos

    //create descriptor set layout
    VkResult result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }

    //texture sampler descriptor set layout
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    //create a descriptor set layout with given bindings for texture
    VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
    textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutCreateInfo.bindingCount = 1;
    textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

    //create desciptor set layout
    result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

void VulkanRenderer::createPushConstantRange()
{
    //define push constant values (no create needed)
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//shader stage push constant will go to
    pushConstantRange.offset = 0; //offset into given data to pass to push constant
    pushConstantRange.size = sizeof(ModelMatrix); //size of daa being passed
}

void VulkanRenderer::createGraphicsPipeline()
{
    shaderMan.WaitForCompile();
    auto vertexShaderCode = readFile("Shaders/shader_vert.spv");
    auto fragmentShaderCode = readFile("Shaders/shader_frag.spv");

    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule; //shader module to be used by stage
    vertexShaderCreateInfo.pName = "main"; //entry point into shader


    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // shader stage name
    fragmentShaderCreateInfo.module = fragmentShaderModule; //shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main"; //entry point into shader

    //put shader stage creation info in to array
    // graphics pipeline creation info requires array of shader stage creates
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

    // how the data for a single vertex (including info such as pos, col..) is as a whole
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0; // can bind multiple streams of data, this defines which one
    bindingDescription.stride = sizeof(Vertex);// potential mistake
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //how to move between data after each vertex

    //how the data for an attribute is defined within a vertex
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

    //position attribute
    attributeDescriptions[0].binding = 0; // which binding the data is at
    attributeDescriptions[0].location = 0; //location in shader where data will be read from
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //format the data will take (also helps define the size)
    attributeDescriptions[0].offset = offsetof(Vertex, pos); // where this attribute is defines in the data for a single vertex
    //normal attribute
    //we don't need colour now so just put in the normals
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, norm);
    //texture attribute
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tex);
 //   vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription; // list of vertex binding descriptions (data spacing/strde)
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();// list of vertex attribute descriptions (data format and where)

    //input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //
    inputAssembly.primitiveRestartEnable = VK_FALSE; // allow overriding of strip topology to start new prims

    //viewport & scissor
    // create a viewport info struct
    VkViewport viewport = {};
    viewport.x = 0.0f;//x start coordinates
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width; // width of viewport
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;//min framebuffer depth
    viewport.maxDepth = 1.0f;

    //create a scissor info struct
    VkRect2D scissor = {};
    scissor.offset = { 0,0 }; //offset to use region from
    scissor.extent = swapChainExtent; // extent to describe region to use, starting at offset

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    //// dynamic states
    //// dynamic states to enable
    //std::vector<VkDynamicState> dynamicStateEnables;
    //dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);//dynamic viewport: can resize in cmbuff
    //dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    ////dynamic state creaion info
    //VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    //dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    //dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE; //change if fragments beyond near/far planes are clipped or clamped to plane
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE; // whether to discard data and skip rasterizer
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // how to handle filling points between vertices, could use wireframe
    rasterizerCreateInfo.lineWidth = 1.0f; // how thick lines shoud be when drawn
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE; //whether to add depth bis to fragments

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE; // enable multisample shading or not
    multisamplingCreateInfo.rasterizationSamples = msaaSamples; // number of samples to use per fragment

    // blending
    //blending decides how to blend a new colour being written to a fragment, with the old value

    //blend attachment state ( how blending is hadled)
    VkPipelineColorBlendAttachmentState colourState = {};
    //colors to apply blending to
    colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourState.blendEnable = VK_TRUE; //enable blending

    //blending uses equation: (srcColorBlendFactor  * new colour) colorBlendOP (dstColorBlendFactor * old colour)
    colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colourState.colorBlendOp = VK_BLEND_OP_ADD;
    colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourState.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
    colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlendingCreateInfo.logicOpEnable = VK_FALSE; // alternative to calculations is to use logical operations
    colourBlendingCreateInfo.attachmentCount = 1;
    colourBlendingCreateInfo.pAttachments = &colourState;

    // pipeline layout
    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout, samplerSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    //create pipeline layout
    VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    //depth stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE; // enable checking depth to determine fragment write
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE; // enable writing to depth buffer (to replace old values)
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // potential for cool effects, coparison op that allows overwrite
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE; // depth bounds test - does the depth value exist between two bounds
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE; // enable stencil test

    Pipelines.push_back(Pipeline());
    Pipelines[0].CreatePipeline(shaderStages, &vertexInputCreateInfo, &inputAssembly, &viewportStateCreateInfo, 
        NULL, &rasterizerCreateInfo, &multisamplingCreateInfo, &colourBlendingCreateInfo, &depthStencilCreateInfo, 
        pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1,
        VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
        vertexShaderModule, fragmentShaderModule, mainDevice.logicalDevice);//shaders get destoyed there, not good, change this later

    //------------Create second pipeline here-------------
    // first get second set of shaders
    auto vertexShaderCode2 = readFile("Shaders/shader2_vert.spv");
    auto fragmentShaderCode2 = readFile("Shaders/shader2_frag.spv");

    vertexShaderModule = createShaderModule(vertexShaderCode2);
    fragmentShaderModule = createShaderModule(fragmentShaderCode2);

    vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule; //shader module to be used by stage
    vertexShaderCreateInfo.pName = "main"; //entry point into shader


    fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // shader stage name
    fragmentShaderCreateInfo.module = fragmentShaderModule; //shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main"; //entry point into shader

    //put shader stage creation info in to array
    // graphics pipeline creation info requires array of shader stage creates
    VkPipelineShaderStageCreateInfo shaderStages2[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

    // create second pipeline
    Pipelines.push_back(Pipeline());
    Pipelines[1].CreatePipeline(shaderStages2, &vertexInputCreateInfo, &inputAssembly, &viewportStateCreateInfo,
        NULL, &rasterizerCreateInfo, &multisamplingCreateInfo, &colourBlendingCreateInfo, &depthStencilCreateInfo,
        pipelineLayout, renderPass, 0, Pipelines[0].getPipeline(), -1,
        VK_PIPELINE_CREATE_DERIVATIVE_BIT,
        vertexShaderModule, fragmentShaderModule, mainDevice.logicalDevice);

    // destroy shader modules
    //vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
   // vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());//pointer to code

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a shader module!");
    }

    return shaderModule;
}

void VulkanRenderer::createDepthBufferImage()
{
    //get supported format for depth buffer
    depthFormat = chooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    //create depth buffer image
    depthBufferImage = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);

    depthBufferImageView = createImageView(depthBufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    colorImage = createImage(swapChainExtent.width, swapChainExtent.height, colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
        msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colorImageMemory);//hmm
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanRenderer::createFramebuffers()
{
    swapchainFramebuffers.resize(swapChainImages.size());

    // create a framebuffer for each swapchain image
    for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
    {
        std::array<VkImageView, 3> attachments =
        {
                            colorImageView,
            depthBufferImageView,
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

void VulkanRenderer::createUniformBuffers()
{
    // ViewProjection buffer size
    VkDeviceSize vpBufferSize = sizeof(UboViewProjection);
    VkDeviceSize lightsBufferSize = Light::getDataSize() * lights.size();
    VkDeviceSize cameraBufferSize = sizeof(glm::vec3);

    // model buffer size
    //VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

    // one uniform buffer for each image (and by extension, comman buffer)
    vpUniformBuffer.resize(swapChainImages.size());
    vpUniformBufferMemory.resize(swapChainImages.size());
    lightsUniformBuffer.resize(swapChainImages.size());
    lightsUniformBufferMemory.resize(swapChainImages.size());
    cameraUniformBuffer.resize(swapChainImages.size());
    cameraUniformBufferMemory.resize(swapChainImages.size());

    //create uniform buffers
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);

        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, lightsBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &lightsUniformBuffer[i], &lightsUniformBufferMemory[i]);
       
        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraUniformBuffer[i], &cameraUniformBufferMemory[i]);
    }
}

void VulkanRenderer::createDescriptorPool()
{
    //type of descriptors
    //view projection pool
    VkDescriptorPoolSize vpPoolSize = {};
    vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size());

    VkDescriptorPoolSize lightsPoolSize = {};
    lightsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightsPoolSize.descriptorCount = static_cast<uint32_t>(lightsUniformBuffer.size());

    VkDescriptorPoolSize cameraPoolSize = {};
    cameraPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraPoolSize.descriptorCount = static_cast<uint32_t>(cameraUniformBuffer.size());

   /* VkDescriptorPoolSize modelPoolSize = {};
    modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());*/

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize, lightsPoolSize, cameraPoolSize };

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(swapChainImages.size()); //max number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()); // amount of pool sizes being passed
    poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

    // create descriptor pool
    VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor pool!");
    }

    //create sampler descriptor pool
    // texture sampler pool
    VkDescriptorPoolSize samplerPoolSize = {};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = MAX_OBJECTS;

    VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
    samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
    samplerPoolCreateInfo.poolSizeCount = 1;
    samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

    result = vkCreateDescriptorPool(mainDevice.logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a descriptor pool");
    }
}

void VulkanRenderer::createDescriptorSets()
{
    //resize descriptor set list so one for every buffer
    descriptorSets.resize(swapChainImages.size());

    std::vector<VkDescriptorSetLayout> setLayouts(swapChainImages.size(), descriptorSetLayout);
    // descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = descriptorPool; //pool to allocate descriptor set from
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size()); // number of sets to allocate
    setAllocInfo.pSetLayouts = setLayouts.data(); // layouts to use to allocate sets (1:1 relationship)

    //allocate descriptor sets(multiple)
    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    //update all of descriptor set buffer bindings
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        // buffer info and data offset info
        VkDescriptorBufferInfo vpBufferInfo = {};
        vpBufferInfo.buffer = vpUniformBuffer[i]; // buffer to get data from
        vpBufferInfo.offset = 0; // position of start of data
        vpBufferInfo.range = sizeof(UboViewProjection); // sizeof data
        
        //data about connection between binding and buffer
        VkWriteDescriptorSet vpSetWrite = {};
        vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        vpSetWrite.dstBinding = 0; // binding to update
        vpSetWrite.dstArrayElement = 0; // index in array to update
        vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        vpSetWrite.descriptorCount = 1; // amount to update
        vpSetWrite.pBufferInfo = &vpBufferInfo; // info about buffer data to bind
//-----
        VkDescriptorBufferInfo lightsBufferInfo = {};
        lightsBufferInfo.buffer = lightsUniformBuffer[i]; // buffer to get data from
        lightsBufferInfo.offset = 0; // position of start of data
        lightsBufferInfo.range = sizeof(Light) * lights.size(); // sizeof data

        //data about connection between binding and buffer
        VkWriteDescriptorSet lightsSetWrite = {};
        lightsSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightsSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        lightsSetWrite.dstBinding = 1; // binding to update
        lightsSetWrite.dstArrayElement = 0; // index in array to update
        lightsSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        lightsSetWrite.descriptorCount = 1; // amount to update
        lightsSetWrite.pBufferInfo = &lightsBufferInfo; // info about buffer data to bind
        //-------------
        VkDescriptorBufferInfo cameraBufferInfo = {};
        cameraBufferInfo.buffer = cameraUniformBuffer[i]; // buffer to get data from
        cameraBufferInfo.offset = 0; // position of start of data
        cameraBufferInfo.range = sizeof(glm::vec3); // sizeof data

        //data about connection between binding and buffer
        VkWriteDescriptorSet cameraSetWrite = {};
        cameraSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        cameraSetWrite.dstSet = descriptorSets[i]; // descriptor set to update
        cameraSetWrite.dstBinding = 2; // binding to update
        cameraSetWrite.dstArrayElement = 0; // index in array to update
        cameraSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of descriptor
        cameraSetWrite.descriptorCount = 1; // amount to update
        cameraSetWrite.pBufferInfo = &cameraBufferInfo; // info about buffer data to bind

        ////model descriptor
        //// model buffer binding info
        //VkDescriptorBufferInfo modelBufferInfo = {};
        //modelBufferInfo.buffer = modelDUniformBuffer[i];
        //modelBufferInfo.offset = 0;
        //modelBufferInfo.range = modelUniformAlignment;

       /* VkWriteDescriptorSet modelSetWrite = {};
        modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        modelSetWrite.dstSet = descriptorSets[i];
        modelSetWrite.dstBinding = 1;
        modelSetWrite.dstArrayElement = 0;
        modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        modelSetWrite.descriptorCount = 1;
        modelSetWrite.pBufferInfo = &modelBufferInfo;*/

        // list of descriptor set writes
        std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite, lightsSetWrite, cameraSetWrite};

        //update the descriptor sets with new buffer/binding info
        vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

int VulkanRenderer::createTextureImage(std::string fileName)
{
    // load image file
    int width, height;
    VkDeviceSize imageSize;
    stbi_uc* imageData = loadTextureFile(fileName, &width, &height, &imageSize);

    // create staging buffer to hold loaded data, ready to copy to device
    VkBuffer imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &imageStagingBuffer, &imageStagingBufferMemory);

    // copy image data to staging buffer
    void* data;
    vkMapMemory(mainDevice.logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(mainDevice.logicalDevice, imageStagingBufferMemory);

    //free original image data
    stbi_image_free(imageData);

    //create image to hold final texture
    VkImage texImage;
    VkDeviceMemory texImageMemory;
    texImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

    //transition image to be dst for copy operation
    transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //copy data to image
    copyImageBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage, width, height);

    transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    //add texture data to vector for reference
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);

    //destroy staging buffers
    vkDestroyBuffer(mainDevice.logicalDevice, imageStagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, imageStagingBufferMemory, nullptr);

    //return index of new texture image
    return textureImages.size() - 1;
}

int VulkanRenderer::createTexture(std::string fileName)
{
    int textureImageLoc = createTextureImage(fileName);

    //create imageview and add to list
    VkImageView imageView = createImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    textureImageViews.push_back(imageView);

    //create texture descriptor
    int descriptorLoc = createTextureDescriptor(imageView);


    return descriptorLoc;
}

void VulkanRenderer::createTextureSampler()
{
    //sampler creation info
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; //whether coords should be normalized between 0 an 1
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; 
    samplerCreateInfo.mipLodBias = 0.0f; // level of details bias for mip level
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;

    VkResult result = vkCreateSampler(mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage)
{
    VkDescriptorSet descriptorSet;

    // descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = samplerDescriptorPool;
    setAllocInfo.pSetLayouts = &samplerSetLayout;
    setAllocInfo.descriptorSetCount = 1;

    // allocate descriptor sets
    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Texture Descriptor Sets");
    }

    //texture image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// image layout when in use
    imageInfo.imageView = textureImage; // image to bind to set
    imageInfo.sampler = textureSampler; // sampler to use for set

    // descriptor write info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    //update new descriptor set
    vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &descriptorWrite, 0, nullptr);

    // add descriptor set to list
    samplerDescriptorSets.push_back(descriptorSet);

    // return descriptor set location
    return samplerDescriptorSets.size() - 1;
    
}

void VulkanRenderer::createCamera()
{
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 20.0f, 0.5f);
}

void VulkanRenderer::createLight()
{
    //create a light. Currently we will only use this one but I should add support for multiple lights later
    lights.push_back(Light(glm::vec3(0.0f, 100.0f, 0.0f)));
}

void VulkanRenderer::compileShaders()
{
    //ShaderMan::CompileShaders();
    //shaderMan = ShaderMan();
    shaderMan.CompileShadersAsync();
}

int VulkanRenderer::createMeshModel(std::string modelFile)
{
    // import model "scene"
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
    if (!scene)
    {
        throw std::runtime_error("Failed to load model! (" + modelFile + ")");
    }

    //get vector of all materials with 1:1 ID placement
    std::vector<std::string> textureNames = Model::LoadMaterials(scene);

    // conversion from the materials list IDs to our Descriptor Arrays IDs
    std::vector<int> matToTex(textureNames.size());

    // loop over textureNames and create textures for them
    for (size_t i = 0; i < textureNames.size(); i++)
    {//if material had no texture, set 0 to indicate no texture, texture 0 will be reserved for a default texture
        if (textureNames[i].empty())
        {
            matToTex[i] = 0;
        }
        else
        {// otherwise, create texture and set value to index of new texture
            matToTex[i] = createTexture(textureNames[i]);
        }
    }

    // load in all our meshes
    std::vector<Mesh> modelMeshes = Model::LoadNode(mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
        scene->mRootNode, scene, matToTex);

    Model meshModel = Model(modelMeshes);
    modelList.push_back(meshModel);

    return modelList.size() - 1;
}

void VulkanRenderer::updateUniformBuffers(uint32_t index)
{
    //do the transformations here or before calling this func
    uboViewProjection.view = camera.calculateViewMatrix();
    // copy vp data
    void* data;
    vkMapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[index], 0, sizeof(UboViewProjection), 0, &data);
    memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
    vkUnmapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[index]);

    data = alloca(Light::getDataSize());
    vkMapMemory(mainDevice.logicalDevice, lightsUniformBufferMemory[index], 0, Light::getDataSize() * lights.size(), 0, &data);
    lights[0].getData(data);
    vkUnmapMemory(mainDevice.logicalDevice, lightsUniformBufferMemory[index]);

    vkMapMemory(mainDevice.logicalDevice, cameraUniformBufferMemory[index], 0, sizeof(glm::vec3), 0, &data);
    memcpy(data, &camera.getCameraPosition(), sizeof(glm::vec3));//hmm
    vkUnmapMemory(mainDevice.logicalDevice, cameraUniformBufferMemory[index]);

    /*
    //copy model data
    //this prevents always reallocating the buffer, because we allocate it once
    //and then update the buffer
    //we can do this because of MAX_OBJECTS
    for (size_t i = 0; i < meshList.size(); i++)
    {
        Model* thisModel = (Model*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment)); // moving the pointer in the buffer
        *thisModel = meshList[i].getModel();
    }

    vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[index], 0, modelUniformAlignment * meshList.size(), 0, &data);
    memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
    vkUnmapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[index]);*/
}

/*void VulkanRenderer::allocateDynamicBufferTransferSpace()
{
    //calculate allignment of model data
  /*  modelUniformAlignment = (sizeof(Model) + minUiformBufferOffset - 1) & ~(minUiformBufferOffset - 1);

    //create space in memory to hold dynamic buffer that is alligned to our required alignemnt and holds max-objects
    modelTransferSpace = (Model*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
}*/

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

    renderPassBeginInfo.framebuffer = swapchainFramebuffers[currentImage];//potential area for optimisations
    // start reording commands to cmb
    VkResult result = vkBeginCommandBuffer(commandBuffer[currentImage], &bufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to start recording a Command Buffer");
    }

    vkCmdBeginRenderPass(commandBuffer[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    //bind pipeline to be used in render pass
    {
        vkCmdBindPipeline(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0].getPipeline());

       // for (size_t j = 0; j < modelList.size(); j++)
       //{
            Model thisModel = modelList[0];//quick hack because we don't want to just render all the models with the same shader
            //push constants to given shader stage directly
            vkCmdPushConstants(commandBuffer[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelMatrix), &thisModel.getModel());

            for (size_t k = 0; k < thisModel.getMeshCount(); k++)
            {
                VkBuffer vertexBuffers[] = { thisModel.getMesh(k)->getVertexBuffer() }; //buffers to bind
                VkDeviceSize offsets[] = { 0 }; //offsets into buffers being bound
                vkCmdBindVertexBuffers(commandBuffer[currentImage], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffer[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

                // dynamic offset amount
                //uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;



                std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[currentImage], samplerDescriptorSets[thisModel.getMesh(k)->getTexId()] };

                // bind descriptor sets
                vkCmdBindDescriptorSets(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                    0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);// will only apply offset to descriptors that are dynamic

                vkCmdDrawIndexed(commandBuffer[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
            }
       // }
    }
    //bind second pipeline
    {
        vkCmdBindPipeline(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[1].getPipeline());

       // for (size_t j = 0; j < modelList.size(); j++)
     //   {
            Model thisModel = modelList[1];
            //push constants to given shader stage directly
            vkCmdPushConstants(commandBuffer[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelMatrix), &thisModel.getModel());

            for (size_t k = 0; k < thisModel.getMeshCount(); k++)
            {
                VkBuffer vertexBuffers[] = { thisModel.getMesh(k)->getVertexBuffer() }; //buffers to bind
                VkDeviceSize offsets[] = { 0 }; //offsets into buffers being bound
                vkCmdBindVertexBuffers(commandBuffer[currentImage], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffer[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

                // dynamic offset amount
                //uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;



                std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[currentImage], samplerDescriptorSets[thisModel.getMesh(k)->getTexId()] };

                // bind descriptor sets
                vkCmdBindDescriptorSets(commandBuffer[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                    0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);// will only apply offset to descriptors that are dynamic

                vkCmdDrawIndexed(commandBuffer[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
            }
       // }
    }
    vkCmdEndRenderPass(commandBuffer[currentImage]);

    //stop recording to command buffer
    result = vkEndCommandBuffer(commandBuffer[currentImage]);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to stop recording a Command Buffer");
    }
    //vkeginCommandBuffer();
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

    //information to create logical device
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); //number of queue create infos
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();//dunno why are we setting these, it should be default

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
        swapchainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        swapChainImages.push_back(swapchainImage);
    }
}

VkImage VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkSampleCountFlagBits numSamples, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory)
{
    //Create Image
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //layout of image data on creation
    imageCreateInfo.usage = useFlags; // bit flags defining what image will be used for
    imageCreateInfo.samples = numSamples;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an image");
    }


    // Create Memory for image

    // get memory requirements for a type of image
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mainDevice.logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice.physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

    result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocInfo, nullptr, imageMemory);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate memory for image!");
    }

    // connect memory to image
    vkBindImageMemory(mainDevice.logicalDevice, image, *imageMemory, 0);

    return image;
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
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_8_BIT; }// I don't want more than x8 MSAA
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
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
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}
