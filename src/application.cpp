#include "application.h"
#include "utils/readShader.h"

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
}

void Application::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer(rainbowTriangleVertexBuffer);
    createCommandBuffers();
    createSynchronozationObjects();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // Render the farme
        drawFrame();
    }
    vkDeviceWaitIdle(device);
}

void Application::cleanup()
{
    cleanUpSwapchain();

    vkDestroyBuffer(device, rainbowTriangleVertexBuffer, nullptr);

    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAcquiredSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
/****************************** Vulkan Instance *******************************/
void Application::createInstance()
{
    // Fill the application info struct
    VkApplicationInfo applicationInfo{};
    applicationInfo.pApplicationName        = "Hello Vulkan";
    applicationInfo.applicationVersion      = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.sType                   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pEngineName             = "Vulkan Engine";
    applicationInfo.engineVersion           = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion              = VK_API_VERSION_1_0;

    // Create the Vulkan instance with the required information including intance extensions and layers
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo           = &applicationInfo;

    auto extensions = getRequiredExtensions();
    instanceInfo.enabledExtensionCount      = extensions.size();
    instanceInfo.ppEnabledExtensionNames    = extensions.data();
    if(enableValidationLayers && checkValidationLayersSupport())
    {
        instanceInfo.enabledLayerCount      = validationLayers.size();
        instanceInfo.ppEnabledLayerNames    = validationLayers.data();
    }
    else
        instanceInfo.enabledLayerCount      = 0;

    if(VK_CALL(vkCreateInstance(&instanceInfo, nullptr, &instance)))
        throw std::runtime_error("Failed to create Instance!");
}
/****************************** Debug messenger *******************************/
void Application::setupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
    debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugUtilsCreateInfo.pfnUserCallback = debugCallback;
    debugUtilsCreateInfo.pUserData = nullptr;

    if(VK_CALL(CreateDebugUtilsMessengerEXT(instance, &debugUtilsCreateInfo, nullptr, &debugMessenger)))
        throw std::runtime_error("Failed to create debug utils messenger handle!");
}
/****************************** Surface Creation*******************************/
void Application::createSurface()
{
    if(VK_CALL(glfwCreateWindowSurface(instance, window, nullptr, &surface)))
        throw std::runtime_error("Could not create a surface!");
}
/******************************* Physical Device ******************************/
void Application::pickPhysicalDevice()
{
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> availableGPUs(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, availableGPUs.data());

    if(gpu_count == 0)
        throw std::runtime_error("No suitable GPUs were found!");

    for(const auto& pdevice : availableGPUs)
    {
        if(isDeviceSuitable(pdevice))
        {
            gpu = pdevice;
            break;
        }
    }
}
/******************************* Logical Device *******************************/
void Application::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(gpu);
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if(enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        deviceCreateInfo.enabledLayerCount = 0;

    if(VK_CALL(vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device)))
        throw std::runtime_error("Could not create logical device!");

    // Retrieve the graphics queue
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}
/********************************* Swap Chain *********************************/
void Application::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainDetails(gpu);
    // Isolate the swapchain details into their respective blocks
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    // Set the images count for the swapchain images
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // restrict the max number of swapchain images that can be used
    if(imageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(gpu);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily)
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if(VK_CALL(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain)))
        throw std::runtime_error("Failed to create swapchain!");

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data());
    // cache the swapchain format and extent
    swapchainFormat = surfaceFormat.format;
    swapchainExtent = extent;
}
/********************************* Image Views ********************************/
void Application::createImageViews()
{
    swapchainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo imageviewCreateInfo{};
        imageviewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageviewCreateInfo.image = swapChainImages[i];
        imageviewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageviewCreateInfo.format = swapchainFormat;
        imageviewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageviewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageviewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageviewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed
        imageviewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageviewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageviewCreateInfo.subresourceRange.levelCount = 1;
        imageviewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageviewCreateInfo.subresourceRange.layerCount = 1;

        if(VK_CALL(vkCreateImageView(device, &imageviewCreateInfo, nullptr, &swapchainImageViews[i])))
            throw std::runtime_error("Failed to create image views!");
    }
}
/******************************** Render Pass *********************************/
void Application::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //  IDK why tf we used this????
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderpassCreateInfo{};
    renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassCreateInfo.attachmentCount = 1;
    renderpassCreateInfo.pAttachments = &colorAttachment;
    renderpassCreateInfo.subpassCount = 1;
    renderpassCreateInfo.pSubpasses = &subpass;
    renderpassCreateInfo.dependencyCount = 1;
    renderpassCreateInfo.pDependencies = &dependency;

    if(VK_CALL(vkCreateRenderPass(device, &renderpassCreateInfo, nullptr, &renderpass)))
        throw std::runtime_error("Failed to create render pass!");
}
/***************************** Graphics Pipeline ******************************/
void Application::createGraphicsPipeline()
{
    std::vector<char> vertexShaderCode = readFile("./src/shaders/spir-v/vert.spv");
    std::vector<char> fragmentShaderCode = readFile("./src/shaders/spir-v/frag.spv");

    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    // Load the attibutes and binding description from the vertex struct
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescription();
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDescription;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInput.pVertexAttributeDescriptions = attributeDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if(VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)))
        throw std::runtime_error("Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageCreateInfos;

    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;//&dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if(VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)))
        throw std::runtime_error("Failed to create graphics pipeline");

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
}
/******************************* Frame Buffers ********************************/
void Application::createFrameBuffers()
{
    swapChainFramebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &swapchainImageViews[i];
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if(VK_CALL(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i])))
            throw std::runtime_error("Failed to create framebuffers!");
    }
}
/******************************* Command Pool *********************************/
void Application::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(gpu);
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolInfo.flags = 0;

    if(VK_CALL(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool)))
        throw std::runtime_error("Failed to create command pool!");
}
/***************************** Vertex Buffers ********************************/
void Application::createVertexBuffer(VkBuffer& buffer)
{
    VkBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = sizeof(rainbowTriangleVertices[0]) * rainbowTriangleVertices.size();
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(VK_CALL(vkCreateBuffer(device, &vertexBufferInfo, nullptr, &buffer)))
        throw std::runtime_error("Failed to create vertex buffer");

    // Get the memory requirements of the vertex buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, rainbowTriangleVertexBuffer, &memRequirements);

    // Allocate the memory for the vertex buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if(VK_CALL(vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory)))
        throw std::runtime_error("Failed to allocate Vetex Buffer Memory!");

    // Bind the memory to the buffer after allocation
    vkBindBufferMemory(device, rainbowTriangleVertexBuffer, vertexBufferMemory, 0);

    // Filling the vertex buffer with vertex data
    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, vertexBufferInfo.size, 0, &data);
    memcpy(data, rainbowTriangleVertices.data(), (size_t) vertexBufferInfo.size);
    vkUnmapMemory(device, vertexBufferMemory);
}
/***************************** Command Buffers ********************************/
void Application::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocInfo{};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if(VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data())))
        throw std::runtime_error("Failed to create command buffers!");

    // Begin recording the command buffers
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferInfo.flags = 0;
        commandBufferInfo.pInheritanceInfo = nullptr;

        if(VK_CALL(vkBeginCommandBuffer(commandBuffers[i], &commandBufferInfo)))
            throw std::runtime_error("Failed to create command buffer!");

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderpass;
        renderPassBeginInfo.framebuffer = swapChainFramebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapchainExtent;
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = {rainbowTriangleVertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(rainbowTriangleVertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if(VK_CALL(vkEndCommandBuffer(commandBuffers[i])))
            throw std::runtime_error("Failed to end recording command buffers!");

    }
}
/******************************** Semphores ***********************************/
void Application::createSynchronozationObjects()
{
    imageAcquiredSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if( VK_CALL(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAcquiredSemaphores[i])) ||
            VK_CALL(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i])) ||
            VK_CALL(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])))
            throw std::runtime_error("Failed to create semaphores and fences!");
    }
}
/******************************** Draw Frame **********************************/
void Application::drawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAcquiredSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    // Mark the image as now being in use by this frame
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAcquiredSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if(VK_CALL(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])))
        throw std::runtime_error("Failed to submit to graphics queue!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapchain();
    }
     else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");


    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
/**************************** Recreate Swap Chain *****************************/
void Application::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanUpSwapchain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandBuffers();
}
/**************************** CleanUp Swap Chain ******************************/
void Application::cleanUpSwapchain()
{
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderpass, nullptr);

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}
/******************************** Vulkan Helper *******************************/
bool Application::checkValidationLayersSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layer : validationLayers)
    {
        bool layerFound = false;
        for (const auto& availLayer : availableLayers)
        {
            if(strcmp(layer, availLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if(!layerFound)
            return false;
    }

    // Now print the layers only for debug purposes
    #ifndef NDEBUG
    PrettyTable<std::string, std::string, std::string> vt({"Available Layers", "Requested Layers", "Found"});
    for (uint32_t i = 0; i < layerCount; i++)
    {
        for(uint32_t j = 0; j < validationLayers.size(); j++)
        {
            if(strcmp(availableLayers[i].layerName, validationLayers[j]) == 0)
            {
                vt.addRow(availableLayers[i].layerName, validationLayers[j], "Yes");
            }
            else
            {
                vt.addRow(availableLayers[i].layerName, "", "");
                break;
            }
        }
    }
    #endif
    vt.print(std::cout);

    return true;
}

// TODO: Print the extensions loaded and required as comparision
std::vector<const char*> Application::getRequiredExtensions()
{
    // First get the extensions needed by GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    // Add any user defined extensions
    if (enableValidationLayers) {
       extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    extensions.insert(extensions.end(), instanceExtensions.begin(), instanceExtensions.end());
   return extensions;
}

bool Application::isDeviceSuitable(VkPhysicalDevice gpu)
{
    // TODO: Fill with more requirements
    QueueFamilyIndices indices = findQueueFamilies(gpu);
    bool areDeviceExtensionsSupported = checkDeviceExtensionSupport(gpu);

    return indices.isComplete() && areDeviceExtensionsSupported;
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice gpu)
{
    QueueFamilyIndices indices;
    // Get the queue families present in the GPU
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queue : queueFamilies)
    {
        if(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);

        if(presentSupport)
            indices.presentFamily = i;

        i++;
    }
    return indices;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice gpu)
{
    uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

   std::vector<VkExtensionProperties> availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

   std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

   #ifndef NDEBUG
   PrettyTable<std::string> table({"Available Device Extensions"});
   for(int i = 0; i < extensionCount; i++)
   {
       // for (size_t j = 0; j < deviceExtensions.size(); j++)
       // {
       //     std::cout << deviceExtensions[j] << std::endl;
       //     if(strcmp(availableExtensions[i].extensionName, deviceExtensions[j]) == 0)
       //     {
       //         table.addRow(availableExtensions[i].extensionName, deviceExtensions[i], "Yes");
       //         break;
       //     }
       //     else
       //     table.addRow(availableExtensions[i].extensionName, "", "");
       // }
       table.addRow(availableExtensions[i].extensionName);
   }
   table.print(std::cout);
   #endif

   return requiredExtensions.empty();
}

SwapChainSupportDetails Application::querySwapChainDetails(VkPhysicalDevice gpu)
{
    SwapChainSupportDetails details;
    // Get the capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.capabilities);
    // Get the formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
    if(formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.formats.data());
    }
    // Get the present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &formatCount, nullptr);
    if(presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &formatCount, details.presentModes.data());
    }
    return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats)
{
    for(const auto& format : surfaceFormats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return surfaceFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if(capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
        return extent;
    }
}

VkShaderModule Application::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if(VK_CALL(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule)))
        throw std::runtime_error("Failed to create shader module!");
    return shaderModule;
}

uint32_t Application::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

    // Find a memory type that is suitable for the vertex buffer
    // and check if the memory type property if it's host visible or host coherent
    // for CPU-GPU visiblity of the vertex memory
    // TODO: Optimise the memory to exist only on the Host and enable writing and reading to it from the CPU
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

/******************************** Function pointers ****************************/
VkResult Application::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Application::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}
/*************************** Event callback Functions *************************/
// TODO: Format messaging!
VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void Application::framebuffer_resize_callback(GLFWwindow* window, int height, int width)
{
    auto wind = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    wind->framebufferResized = true;
}
