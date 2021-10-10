#include "application.h"

// STL
#include <array>
#include <assert.h>
#include <sstream>

// Utilities
#include "utils/VulkanCheckResult.h"
#include "utils/readShader.h"
#include <common/output_stream.h>


// Profiling
#include <Tracy.hpp>

// TibyObj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

static int SpirvReflectExample(const void* spirv_code, size_t spirv_nbytes)
{
  // Generate reflection data for a shader
  SpvReflectShaderModule module;
  SpvReflectResult result = spvReflectCreateShaderModule(spirv_nbytes, spirv_code, &module);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  // Enumerate and extract shader's input variables
  uint32_t var_count = 0;
  result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  VK_LOG("Input Variables Count : ", var_count);
  SpvReflectInterfaceVariable** input_vars =
    (SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
  result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  for (size_t i = 0; i < var_count; i++) {
    SpvReflectInterfaceVariable* inputVar = input_vars[i];
    std::cout << "SPIRV ID              : " << inputVar->spirv_id << std::endl;
    std::cout << "Input Variable Name   : " << inputVar->name << std::endl;
    std::cout << "location id           : " << inputVar->location << std::endl;
    std::cout << "Storage class         : " << ToStringSpvStorageClass(inputVar->storage_class) << std::endl;
    std::cout << "Decoration            : " << ToStringDecorationFlags(inputVar->storage_class) << std::endl;
    std::cout << "Member Count          : " << inputVar->member_count << std::endl;
    std::cout << "Format                : " << ToStringFormat(inputVar->format) << " (" << inputVar->format << ")" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
  }

  // Output variables, descriptor bindings, descriptor sets, and push constants
  // can be enumerated and extracted using a similar mechanism.

  // Destroy the reflection data when no longer required.
  spvReflectDestroyShaderModule(&module);
  return 0;
}

/**************************** Application Flow ********************************/
void Application::Run()
{
    InitResources();
    InitWindow();
    InitVulkan();
    InitImGui();
    MainLoop();
    CleanUp();
}

void Application::InitResources()
{
    GenerateSphereSmooth(1, 32, 32);
    GenerateQuadSphere(1, 32, 32);
    for (size_t i = 0; i < sphereVertices.size(); i++) {
        Vertex vertex{};
        vertex.position = sphereVertices[i];
        vertex.color = sphereNormals[i];
        vertex.texCoord = sphereUVs[i];
        sphereVertexData.push_back(vertex);
    }
}

void Application::InitWindow()
{
/******************************************************************/
/**/ window = new Window("Hello Vulkan again!", width, height); /**/
/******************************************************************/
}

void Application::InitVulkan()
{
    VKInstance::GetInstanceManager()->Init("Hello Vulkan", window->getGLFWwindow(), enableValidationLayers);

    VKLogicalDevice::GetDeviceManager()->Init();

    vertexShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultVert.spv"), ShaderType::VERTEX_SHADER);
    fragmentShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultFrag.spv"), ShaderType::FRAGMENT_SHADER);
    outlineFragmentShader.CreateShader((SHADER_BINARY_DIR) + std::string("/outline.spv"), ShaderType::FRAGMENT_SHADER);

////////////////////////////////////////////////////////////////////////////////
    // SPIRV Reflect
    auto defaultByteCode = readFile((SHADER_BINARY_DIR) + std::string("/defaultVert.spv"));
    SpirvReflectExample(defaultByteCode.data(), defaultByteCode.size());

////////////////////////////////////////////////////////////////////////////////

    cmdPoolManager.Init();
    RecreateCommandPipeline();

    // Create the synchronization stuff
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderingFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(3, VK_NULL_HANDLE);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i])) ||
        VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &renderingFinishedSemaphores[i])) ||
        VK_CALL(vkCreateFence(VKDEVICE, &fenceInfo, nullptr, &inFlightFences[i])))
        {
            throw std::runtime_error("Cannot make semaphore!");
        }
    }
}

void Application::InitImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // ImGui Specific Command pool
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if(VK_CALL(vkCreateDescriptorPool(VKDEVICE, &pool_info, nullptr, &imguiDescriptorPool)))
        throw std::runtime_error("Cannot create ImGui command pool!");
    else VK_LOG_SUCCESS("Imgui Command Pool succesfully created!");

    // Create a render pass (last one in the application) for ImGUi
    // ImGui Attachment Description
    VkAttachmentDescription imguiAttachmentDesc = {};
    imguiAttachmentDesc.format = swapchainManager.GetSwapFormat();
    imguiAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    imguiAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    imguiAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    imguiAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    imguiAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    imguiAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imguiAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Since UI is the last render pass, now this will be used for presentation

    // ImGui color attachment reference to be used by the attachment and this is described by the attachment Description
    VkAttachmentReference imguiColorAttachmentRef = {};
    imguiColorAttachmentRef.attachment = 0;
    imguiColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Create a subpass using the attachment reference
    VkSubpassDescription imguiSubpassDesc{};
    imguiSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    imguiSubpassDesc.colorAttachmentCount = 1;
    imguiSubpassDesc.pColorAttachments = &imguiColorAttachmentRef;

    // Create the sub pass dependency to communicate between different subpasses, we describe the dependencies between them
    VkSubpassDependency imguiDependency{};
    imguiDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    imguiDependency.dstSubpass = 0;
    imguiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    imguiDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    imguiDependency.srcAccessMask = 0;
    imguiDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Now create the imgui renderPass
    VkRenderPassCreateInfo imguiRPInfo{};
    imguiRPInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    imguiRPInfo.attachmentCount = 1;
    imguiRPInfo.pAttachments = &imguiAttachmentDesc;
    imguiRPInfo.subpassCount = 1;
    imguiRPInfo.pSubpasses = &imguiSubpassDesc;
    imguiRPInfo.dependencyCount = 1;
    imguiRPInfo.pDependencies = &imguiDependency;
    if(VK_CALL(vkCreateRenderPass(VKDEVICE, &imguiRPInfo, nullptr, &imguiRenderPass)))
        throw std::runtime_error("Cannot create imgui render pass");
    else VK_LOG_SUCCESS("ImGUi Renderpass succesfully created !");

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window->getGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VKInstance::GetInstanceManager()->GetInstance();
    init_info.PhysicalDevice = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU();
    init_info.Device = VKDEVICE;
    init_info.QueueFamily = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGraphicsFamilyIndex();
    init_info.Queue = VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2; // IDK why?
    init_info.ImageCount = swapchainManager.GetSwapImageCount();
    init_info.CheckVkResultFn = ImGuiError;
    ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

    // Get the command buffers for imgui
    imguiCmdBuffer = cmdPoolManager.AllocateBuffer();

    if(VK_CALL(vkResetCommandPool(VKDEVICE, cmdPoolManager.GetPool(), 0)))
        throw std::runtime_error("Canot reset imgui command pool!");
    else VK_LOG_SUCCESS("succesfully reset ImGui Command Pool");

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(imguiCmdBuffer, &begin_info);

    ImGui_ImplVulkan_CreateFontsTexture(imguiCmdBuffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &imguiCmdBuffer;
    vkEndCommandBuffer(imguiCmdBuffer);

    if(VK_CALL(vkQueueSubmit(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE)))
        throw std::runtime_error("Cannot submit imgui command buffer!");
    else VK_LOG_SUCCESS("succesfully submitted ImGui command buffer!");

    vkDeviceWaitIdle(VKDEVICE);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    vkFreeCommandBuffers(VKDEVICE, cmdPoolManager.GetPool(), 1, &imguiCmdBuffer);
}

void Application::MainLoop()
{
    lastTime = glfwGetTime();;
    while (!window->closed()) {
        window->Update();
        camera.Update(*window, delta);

        double currentTime = glfwGetTime();
        delta = currentTime - lastTime;
        nbFrames++;
        if ( delta >= 1.0 ){ // If last cout was more than 1 sec ago

            double fps = double(nbFrames) / delta;

            std::stringstream ss;
            ss << " Hello Vulkan again! " << " [" << (unsigned int)fps << " FPS]";
            glfwSetWindowTitle(window->getGLFWwindow(), ss.str().c_str());

            nbFrames = 0;
            lastTime = currentTime;
        }

        OnUpdate(delta);

        ImGui_ImplVulkan_SetMinImageCount(MAX_FRAMES_IN_FLIGHT);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGuizmo Initialization
        ImGuizmo::BeginFrame();
        ImGuiIO& io = ImGui::GetIO();
        // ImGuizmo::SetOrthographic(true);
        // ImGuizmo::SetRect(0, 0, swapchainManager.GetSwapExtent().width, swapchainManager.GetSwapExtent().height);
        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        // VK_LOG("X : ", ImGui::GetWindowPos().x, ", Y : ", ImGui::GetWindowPos().y);
        // ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
        ImGuizmo::SetRect(ImGui::GetWindowPos().x - (windowWidth / 2), ImGui::GetWindowPos().y - (windowHeight / 2), swapchainManager.GetSwapExtent().width, swapchainManager.GetSwapExtent().height);
        OnImGui();
        ImGui::Render();
        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        //vkDeviceWaitIdle(VKDEVICE);
        RecordCommands();
        DrawFrame();
        FrameMark
    }
    vkDeviceWaitIdle(VKDEVICE);
}

void Application::DrawFrame()
{
    ZoneScopedC(0xff0000)
    vkWaitForFences(VKDEVICE, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    // Acquire the image to render onto
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(VKDEVICE, swapchainManager.GetSwapchainKHR(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        // framebufferResized = false;
        RecreateSwapchain();
        return;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Cannot acquire next image!");
    }

    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(VKDEVICE, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    // Update the uniform buffer before submitting the commands
    UpdateMVPUBO(imageIndex);

    //VK_LOG("Image index : ", imageIndex);
    // Submit the command buffer queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphore[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 2;
    VkCommandBuffer buffers[] = {swapCmdBuffers.GetBufferAt(imageIndex), imguiCmdBuffers.GetBufferAt(imageIndex)};
    submitInfo.pCommandBuffers = buffers;
    VkSemaphore signalSemaphores[] = {renderingFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(VKDEVICE, 1, &inFlightFences[currentFrame]);
    if(VK_CALL(vkQueueSubmit(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]))) {
        throw std::runtime_error("Cannot submit command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchainManager.GetSwapchainKHR()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(VKLogicalDevice::GetDeviceManager()->GetPresentQueue(), &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->IsResized()) {
        window->SetResizedFalse();
        RecreateSwapchain();
        return;
    }
    else if(result != VK_SUCCESS) {
        throw std::runtime_error("Cannot submit presentation queue!");
    }

    currentFrame = (currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}

void Application::CleanUp()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(VKDEVICE, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(VKDEVICE, renderingFinishedSemaphores[i], nullptr);
        vkDestroyFence(VKDEVICE, inFlightFences[i], nullptr);
    }
    delete window;
    window = nullptr;

    // Clearn Up ImGui
    CleanUpImGuiResources();

    cmdPoolManager.Destroy();
    CleanUpCommandListResources();
    vertexShader.DestroyModule();
    fragmentShader.DestroyModule();
    outlineFragmentShader.DestroyModule();
    VKLogicalDevice::GetDeviceManager()->Destroy();
    VKInstance::GetInstanceManager()->Destroy();
}
/******************************************************************************/

void Application::RecreateCommandPipeline()
{
    swapchainManager.Init(window->getGLFWwindow());

    fixedPipelineFuncs.SetInputAssemblyStageInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI(false);
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();

    wireframeFixedPipelineFuncs.SetInputAssemblyStageInfo(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    wireframeFixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    wireframeFixedPipelineFuncs.SetRasterizerSCI(true);
    wireframeFixedPipelineFuncs.SetMultiSampleSCI();
    wireframeFixedPipelineFuncs.SetDepthStencilSCI();
    wireframeFixedPipelineFuncs.SetColorBlendSCI();
    wireframeFixedPipelineFuncs.SetDynamicSCI();


    // Create the texture
    gridTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/TestGrid_1024.png"), cmdPoolManager);
    earthTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/earthmap.jpg"), cmdPoolManager);

    // Create the push contants
    VkPushConstantRange modelPushConstant;
    modelPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    modelPushConstant.offset = 0;
    modelPushConstant.size = sizeof(DefaultPushConstantData);

    // Create the uniform buffer
    mvpUniformBuffer.CreateUniformBuffer(swapchainManager.GetSwapImageCount(), gridTexture);
    auto layout = mvpUniformBuffer.GetDescriptorSetLayout();

    fixedPipelineFuncs.SetPipelineLayout(layout, modelPushConstant);
    wireframeFixedPipelineFuncs.SetPipelineLayout(layout, modelPushConstant);

    renderPassManager.Init(swapchainManager.GetSwapFormat());
    std::vector<VkPipelineShaderStageCreateInfo>  shaderInfo = {vertexShader.GetShaderStageInfo(), fragmentShader.GetShaderStageInfo()};
    std::vector<VkPipelineShaderStageCreateInfo>  outlineShaderInfo = {vertexShader.GetShaderStageInfo(), outlineFragmentShader.GetShaderStageInfo()};

    fixedTopologyPipelines.resize(5);
    wireframeFixedTopologyPipelineFuncs.resize(5);
    graphicsPipelines.resize(5);
    wireframeGraphicsPipelines.resize(5);

    for (size_t i = 0; i <= 4; i++) {
        fixedTopologyPipelines[i].SetInputAssemblyStageInfo((VkPrimitiveTopology)i);
        fixedTopologyPipelines[i].SetViewportSCI(swapchainManager.GetSwapExtent());
        fixedTopologyPipelines[i].SetRasterizerSCI(false);
        fixedTopologyPipelines[i].SetMultiSampleSCI();
        fixedTopologyPipelines[i].SetDepthStencilSCI();
        fixedTopologyPipelines[i].SetColorBlendSCI();
        fixedTopologyPipelines[i].SetDynamicSCI();

        // Wireframe mode fixed functions
        wireframeFixedTopologyPipelineFuncs[i].SetInputAssemblyStageInfo((VkPrimitiveTopology)i);
        wireframeFixedTopologyPipelineFuncs[i].SetViewportSCI(swapchainManager.GetSwapExtent());
        wireframeFixedTopologyPipelineFuncs[i].SetRasterizerSCI(true);
        wireframeFixedTopologyPipelineFuncs[i].SetMultiSampleSCI();
        wireframeFixedTopologyPipelineFuncs[i].SetDepthStencilSCI();
        wireframeFixedTopologyPipelineFuncs[i].SetColorBlendSCI();
        wireframeFixedTopologyPipelineFuncs[i].SetDynamicSCI();

        fixedTopologyPipelines[i].SetPipelineLayout(layout, modelPushConstant);
        wireframeFixedTopologyPipelineFuncs[i].SetPipelineLayout(layout, modelPushConstant);

        // Create the corresponding graphics pipelines (wireframe and non-wireframe mode)
        graphicsPipelines[i].Create(shaderInfo, fixedTopologyPipelines[i], renderPassManager.GetRenderPass());
        // Wireframe uses the outline shader now
        wireframeGraphicsPipelines[i].Create(outlineShaderInfo, wireframeFixedTopologyPipelineFuncs[i], renderPassManager.GetRenderPass());
    }

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());
    wireframeGraphicsPipeline.Create(shaderInfo, wireframeFixedPipelineFuncs, renderPassManager.GetRenderPass());
    // wireframeOutlineGraphicsPipeline.Create(outlineShaderInfo, wireframeFixedPipelineFuncs, renderPassManager.GetRenderPass());

    // Create the depth image
    depthImage.CreateDepthImage(swapchainManager.GetSwapExtent().width, swapchainManager.GetSwapExtent().height, cmdPoolManager);

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), depthImage.GetDepthImageView(), swapchainManager.GetSwapExtent());

    swapCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());
    imguiCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());

    // Create the triangle vertex buffer
    triVBO.Create(rainbowTriangleVertices, cmdPoolManager);
    triIBO.Create(rainbowTriangleIndices, cmdPoolManager);

    quadVBO.Create(whiteQuadVertices, cmdPoolManager);
    quadIBO.Create(whiteQuadIndices, cmdPoolManager);

    // Budda vbo and  ibo
    LoadModel((SRC_DIR) + std::string("/data/models/lowpolyTriSphere.obj"), buddaVertices, buddaIndices, buddaQuadIndices, true);
    buddaVBO.Create(buddaVertices, cmdPoolManager);
    buddaIBO.Create(buddaIndices, cmdPoolManager);
    LoadModel((SRC_DIR) + std::string("/data/models/lowpolyQuadSphere.obj"), buddaVertices, buddaIndices, buddaQuadIndices, false);
    buddaQuadIBO.Create(buddaQuadIndices, cmdPoolManager);

    cubeVBO.Create(cubeVertices, cmdPoolManager);

    sphereVBO.Create(sphereVertexData, cmdPoolManager);
    sphereIBO.Create(sphereIndices, cmdPoolManager);
    sphereQuadIBO.Create(sphereQuadIndices, cmdPoolManager);
}

void Application::RecordCommands()
{
    ZoneScoped

    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    auto descriptorSets = mvpUniformBuffer.GetSets();
    for (int i = 0; i <  cmdBuffers.size(); i++) {

        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        // renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
        // if(!enableWireframe)
        //     graphicsPipeline.Bind(cmdBuffers[i]);
        // else
        //     wireframeGraphicsPipeline.Bind(cmdBuffers[i]);

        // Select the wireframe or non-wireframe graphics pipeline based on the primite mode
        if(!enableWireframe) {
            graphicsPipelines[topologyPipelineID].Bind(cmdBuffers[i]);
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedTopologyPipelines[topologyPipelineID].GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
        }
        else {
            wireframeGraphicsPipelines[wireframeTopologyPipelineID].Bind(cmdBuffers[i]);
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframeFixedTopologyPipelineFuncs[wireframeTopologyPipelineID].GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
        }

        // Bind buffer to the comands
        // triIBO.Bind(cmdBuffers[i]);

        // Bind the push contants
        modelPCData.model = glm::rotate(glm::mat4(1.0f), (float)glm::radians(90.0f * 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);

        // Bind the descriptor sets
        // if(!enableWireframe)
        //     vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedPipelineFuncs.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
        // else
        //     vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframeFixedPipelineFuncs.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

        // Issue the draw command for the super big quad
        triVBO.Bind(cmdBuffers[i]);
        vkCmdDraw(cmdBuffers[i], 3, 1, 0, 0);
        // vkCmdDrawIndexed(cmdBuffers[i], 3, 1, 0, 0, 0);
        // Drawing another white quad
        // quadVBO.Bind(cmdBuffers[i]);
        // quadIBO.Bind(cmdBuffers[i]);
        // vkCmdDrawIndexed(cmdBuffers[i], 5, 1, 0, 0, 0);

        // Draw the Cube
        // modelPCData.model = g lm::rotate(glm::mat4(1.0f), (float)glm::radians(90.0f * sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
        cubeVBO.Bind(cmdBuffers[i]);
        // vkCmdDraw(cmdBuffers[i], 36, 1, 0, 0);


        // Draw the budda model
        buddaVBO.Bind(cmdBuffers[i]);
        buddaIBO.Bind(cmdBuffers[i]);
        modelPCData.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f));
        modelPCData.model *= glm::rotate(glm::mat4(1.0f), (float)glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPCData.model *= glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
        vkCmdDrawIndexed(cmdBuffers[i], buddaIndices.size(), 1, 0, 0, 0);

        // Quad mesh budda
        buddaQuadIBO.Bind(cmdBuffers[i]);
        modelPCData.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, -2.5f));
        modelPCData.model *= glm::rotate(glm::mat4(1.0f), (float)glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPCData.model *= glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
        vkCmdDrawIndexed(cmdBuffers[i], buddaQuadIndices.size(), 1, 0, 0, 0);

        // Draw the Sphere
        sphereVBO.Bind(cmdBuffers[i]);
        // sphereIBO.Bind(cmdBuffers[i]);
        // sphereQuadIBO.Bind(cmdBuffers[i]);

        // modelPCData.model = glm::translate(glm::mat4(1.0f), glm::vec3((float)sin(glfwGetTime()), 1.0f, 1.0f));
        // vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
        // vkCmdDrawIndexed(cmdBuffers[i], sphereIndices.size(), 1, 0, 0, 0);

        for (float x = -6.0f; x < 11.0f; x+= 4.0f) {
            for (float y = -6.0f; y < 11.0f; y+= 4.0f) {
                sphereIBO.Bind(cmdBuffers[i]);
                graphicsPipelines[topologyPipelineID].Bind(cmdBuffers[i]);
                vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedTopologyPipelines[topologyPipelineID].GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
                modelPCData.model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.5f));
                modelPCData.model *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                vkCmdPushConstants(cmdBuffers[i], fixedTopologyPipelines[topologyPipelineID].GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
                vkCmdDrawIndexed(cmdBuffers[i], sphereIndices.size(), 1, 0, 0, 0);

                // Scale and draw the outline again
                // Bind the outline pipeline
                if(enableWireframe) {
                    sphereQuadIBO.Bind(cmdBuffers[i]);
                    wireframeGraphicsPipelines[wireframeTopologyPipelineID].Bind(cmdBuffers[i]);
                    vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframeFixedTopologyPipelineFuncs[wireframeTopologyPipelineID].GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
                    modelPCData.model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.5f));
                    modelPCData.model *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    // Scale and upload the model matrix
                    modelPCData.model *= glm::scale(glm::mat4(1.0f), glm::vec3(1.01f));
                    vkCmdPushConstants(cmdBuffers[i], wireframeFixedTopologyPipelineFuncs[wireframeTopologyPipelineID].GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);
                    // Draw again
                    vkCmdDrawIndexed(cmdBuffers[i], sphereQuadIndices.size(), 1, 0, 0, 0);
                }
            }
        }

        renderPassManager.EndRenderPass(cmdBuffers[i]);
		swapCmdBuffers.EndRecordingBuffer(cmdBuffers[i]);

        // ImGui Commands recording
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(imguiCmdBuffers.GetBufferAt(i), &info);

        VkRenderPassBeginInfo RPinfo = {};
        RPinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        RPinfo.renderPass = imguiRenderPass;
        RPinfo.framebuffer = framebuffers[i];
        RPinfo.renderArea.extent.width = swapchainManager.GetSwapExtent().width;
        RPinfo.renderArea.extent.height = swapchainManager.GetSwapExtent().height;
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {
            {clearColor[0], clearColor[1], clearColor[2], clearColor[3]}
        };
        clearValues[1].depthStencil = {1.0f, 0};

        RPinfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        RPinfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(imguiCmdBuffers.GetBufferAt(i), &RPinfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCmdBuffers.GetBufferAt(i));

        vkCmdEndRenderPass(imguiCmdBuffers.GetBufferAt(i));
        vkEndCommandBuffer(imguiCmdBuffers.GetBufferAt(i));
    }
}

void Application::RecreateSwapchain()
{
    VK_LOG_SUCCESS("Recreating Swapchain..........");
    vkDeviceWaitIdle(VKDEVICE);

    CleanUpCommandListResources();

    RecreateCommandPipeline();
    RecordCommands();
}

void Application::UpdateMVPUBO(uint32_t currentImageIndex)
{
    ZoneScoped

    UniformBufferObject ubo{};
    // ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.view = camera.GetViewMatrix();
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)swapchainManager.GetSwapExtent().width / swapchainManager.GetSwapExtent().height, 0.01f, 100.0f);
    ubo.proj[1][1] *= -1;

    LightUniformBufferObject lightUBO;
    lightUBO.objectColor = objectColor;
    lightUBO.lightColor = lightColor;
    lightUBO.lightPos = lightPos;

    mvpUniformBuffer.UpdateBuffer(ubo, currentImageIndex);
    mvpUniformBuffer.UpdateLightBuffer(lightUBO, currentImageIndex);
}

void Application::CleanUpCommandListResources()
{
    framebufferManager.Destroy();
    depthImage.Destroy();
    gridTexture.Destroy();
    earthTexture.Destroy();
    sphereVBO.Destroy();
    sphereIBO.Destroy();
    sphereQuadIBO.Destroy();
    cubeVBO.Destroy();
    triVBO.Destroy();
    triIBO.Destroy();
    quadVBO.Destroy();
    quadIBO.Destroy();
    buddaVBO.Destroy();
    buddaIBO.Destroy();
    buddaQuadIBO.Destroy();
    mvpUniformBuffer.Destroy();
    graphicsPipeline.Destroy();
    wireframeGraphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    wireframeFixedPipelineFuncs.DestroyPipelineLayout();

    for (size_t i = 0; i <= 4; i++) {
        std::cout << "\033[4;33;49m Deleting Pipeline layouts \033[0m" << std::endl;

        graphicsPipelines[i].Destroy();
        wireframeGraphicsPipelines[i].Destroy();
        fixedTopologyPipelines[i].DestroyPipelineLayout();
        wireframeFixedTopologyPipelineFuncs[i].DestroyPipelineLayout();
    }
    graphicsPipelines.clear();
    wireframeGraphicsPipelines.clear();
    fixedTopologyPipelines.clear();
    wireframeFixedTopologyPipelineFuncs.clear();

    swapchainManager.Destroy();
}

void Application::CleanUpImGuiResources()
{
    // ImGUi render pass
    vkDestroyRenderPass(VKDEVICE, imguiRenderPass, nullptr);

    // Resources to destroy when the program ends
    ImGui_ImplVulkan_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Imgui Descriptor pool
    vkDestroyDescriptorPool(VKDEVICE, imguiDescriptorPool, nullptr);

}
/******************************* ImGui Callbacks *******************************/
void Application::ImGuiError(VkResult err)
{
    if(VK_CALL(err))
        throw std::runtime_error("ImGui Error!");
}

void Application::OnImGui()
{
    ZoneScoped

    auto proj = glm::perspective(glm::radians(45.0f), (float)swapchainManager.GetSwapExtent().width  / swapchainManager.GetSwapExtent().height, 0.01f, 100.0f);
    // proj[1][1] *= -1;
    modelTransform = modelTransform.AttachGuizmo(globalOperation, camera.GetViewMatrix(), proj);
    lightPos = glm::vec3(modelTransform.position.x, modelTransform.position.y, modelTransform.position.z);

    ImGui::ShowDemoWindow();
    ImGui::Begin("Yeah Bitch!");
    {
        static ImVec4 color = {0.24, 0.24, 0.24, 1.0f};
        ImGui::Text("Background Color"); ImGui::SameLine(); ImGui::ColorEdit4("Mycolor#2", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        clearColor[0] = color.x;
        clearColor[1] = color.y;
        clearColor[2] = color.z;
        clearColor[3] = color.w;
        renderPassManager.SetClearColor(clearColor);
        ImGui::Checkbox("Wireframe Mode ", &enableWireframe);
        static const char* items[] = { "Translate", "Rotate", "Scale"};
        static const char* current_item = "Translate";

        if (ImGui::BeginCombo("Guizmo Mode##combo", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(items[n], is_selected))
                    current_item = items[n];
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
        if(!strcmp(current_item, "Translate"))
            globalOperation = ImGuizmo::TRANSLATE;
        else if(!strcmp(current_item, "Rotate"))
            globalOperation = ImGuizmo::ROTATE;
        else if(!strcmp(current_item, "Scale"))
            globalOperation = ImGuizmo::SCALE;

        static const char* topologies[] = { "Point List", "Line List", "Line Strip", "Triangle List", "Traingle Strip"};
        static const char* currentTopology = "Triangle List";
        static const char* currentWireframeTopology = "Line Strip";
        if (ImGui::BeginCombo("Topology", currentTopology))
        {
            for (int n = 0; n < IM_ARRAYSIZE(topologies); n++)
            {
                bool is_topology_selected = (currentTopology == topologies[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(topologies[n], is_topology_selected)) {
                    currentTopology = topologies[n];
                    topologyPipelineID = n;
                }
                if (is_topology_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Wireframe Topology", currentWireframeTopology))
        {
            for (int n = 0; n < IM_ARRAYSIZE(topologies); n++)
            {
                bool is_wireframe_topology_selected = (currentWireframeTopology == topologies[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(topologies[n], is_wireframe_topology_selected)) {
                    currentWireframeTopology = topologies[n];
                    wireframeTopologyPipelineID = n;
                }
                if (is_wireframe_topology_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }

    }
    ImGui::End();

    ImGui::Begin("Image Demo");
    {
        // imguiGridTexture = ImGui_ImplVulkan_AddTexture(gridTexture.GetTextureImageSampler(), gridTexture.GetTextureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        // imguiEarthTexture = ImGui_ImplVulkan_AddTexture(earthTexture.GetTextureImageSampler(), earthTexture.GetTextureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        // ImGui::Image((ImTextureID)ImGui_ImplVulkan_AddTexture(depthImage.GetImage().GetImageSampler(), depthImage.GetDepthImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL), ImVec2(200, 200));
        // ImGui::Image((ImTextureID)imguiEarthTexture, ImVec2(200, 200));
    }
    ImGui::End();

    ImGui::Begin("Light Settings");
    {
        ImGui::ColorEdit3("Object Color", &objectColor.r);
        ImGui::ColorEdit3("Light Color", &lightColor.r);
        ImGui::DragFloat3("Light Position", &lightPos.x, 0.1f);
    }
    ImGui::End();
}

void Application::OnUpdate(double dt) { }

void Application::LoadModel(std::string path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, std::vector<uint16_t>& quadIndices, bool triangulate)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr, triangulate)) {
        throw std::runtime_error(warn + err);
    }
    std::cout << "Beginning to load model at path : " << path << std::endl;
    std::unordered_map<Vertex, uint16_t> uniqueVertices{};

    // PrintInfo(attrib, shapes, materials);

    auto faces = shapes[0].mesh.num_face_vertices;
    // std::cout << "\033[4;33;49m Num faces in shape 0 : " << faces[2] << " \033[0m" << std::endl;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                0.0f, 0.0f
                //attrib.texcoords[2 * index.texcoord_index + 0],
                //attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint16_t>(vertices.size());
                vertices.push_back(vertex);
            }

            // indices.push_back(uniqueVertices[vertex]);
        }

        // std::vector<uint16_t> quadIndices;

        size_t index_offset = 0;
        // For each face
        for (size_t f = 0; f < 22 /*shape.mesh.num_face_vertices.size()*/; f++)
        {
          size_t fnum = shape.mesh.num_face_vertices[f];

          // printf("  face[%ld].fnum = %ld\n", static_cast<long>(f), static_cast<unsigned long>(fnum));

          // For each vertex in the face
          for (size_t v = 0; v < fnum; v++) {
            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            // printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f), static_cast<long>(v), idx.vertex_index, idx.normal_index, idx.texcoord_index);
            quadIndices.push_back(idx.vertex_index);
            indices.push_back(idx.texcoord_index);
          }
          // tinyobj::index_t idx = shape.mesh.indices[index_offset];
          // quadIndices.push_back(idx.vertex_index);

          index_offset += fnum;
        }

        // std::cout << "Indices count : " << indices.size() << std::endl;
    }
}
