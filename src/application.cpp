#include "application.h"

#include "utils/VulkanCheckResult.h"

#include <sstream>

// TibyObj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
/**************************** Application Flow ********************************/
void Application::Run()
{
    InitWindow();
    InitVulkan();
    InitImGui();
    MainLoop();
    CleanUp();
}

void Application::InitWindow()
{
    // Create the window
    window = new Window("Hello Vulkan again!", 800, 600);
}

void Application::InitVulkan()
{
    VKInstance::GetInstanceManager()->Init("Hello Vulkan", window->getGLFWwindow());

    VKLogicalDevice::GetDeviceManager()->Init();

    vertexShader.CreateShader("./src/shaders/spir-v/vert.spv", ShaderType::VERTEX_SHADER);
    fragmentShader.CreateShader("./src/shaders/spir-v/frag.spv", ShaderType::FRAGMENT_SHADER);

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
        if(VK_CALL(vkCreateSemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i])) ||
        VK_CALL(vkCreateSemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &semaphoreInfo, nullptr, &renderingFinishedSemaphores[i])) ||
        VK_CALL(vkCreateFence(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i])))
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
    if(VK_CALL(vkCreateDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &pool_info, nullptr, &imguiDescriptorPool)))
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
    if(VK_CALL(vkCreateRenderPass(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &imguiRPInfo, nullptr, &imguiRenderPass)))
        throw std::runtime_error("Cannot create imgui render pass");
    else VK_LOG_SUCCESS("ImGUi Renderpass succesfully created !");

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window->getGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VKInstance::GetInstanceManager()->GetInstance();
    init_info.PhysicalDevice = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU();
    init_info.Device = VKLogicalDevice::GetDeviceManager()->GetLogicalDevice();
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

    if(VK_CALL(vkResetCommandPool(DEVICE, cmdPoolManager.GetPool(), 0)))
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

    vkDeviceWaitIdle(DEVICE);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    vkFreeCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), cmdPoolManager.GetPool(), 1, &imguiCmdBuffer);

    // RecordCommands();
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
        // ImGuizmo Initializaiton
        ImGuizmo::BeginFrame();
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, swapchainManager.GetSwapExtent().width, swapchainManager.GetSwapExtent().height);
        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        // ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
        // ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        OnImGui();
        ImGui::Render();
        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());
        RecordCommands();
        DrawFrame();
    }
    vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());
}

void Application::DrawFrame()
{
    vkWaitForFences(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    // Acquire the image to render onto
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), swapchainManager.GetSwapchainKHR(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        // framebufferResized = false;
        RecreateSwapchain();
        return;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Cannot acquire next image!");
    }

    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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

    vkResetFences(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), 1, &inFlightFences[currentFrame]);
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
        vkDestroySemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), renderingFinishedSemaphores[i], nullptr);
        vkDestroyFence(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), inFlightFences[i], nullptr);
    }
    delete window;
    window = nullptr;

    // Clearn Up ImGui
    CleanUpImGuiResources();

    cmdPoolManager.Destroy();
    CleanUpCommandListResources();
    vertexShader.DestroyModule();
    fragmentShader.DestroyModule();
    VKLogicalDevice::GetDeviceManager()->Destroy();
    VKInstance::GetInstanceManager()->Destroy();
}
/******************************************************************************/
void Application::RecreateSwapchain()
{
    VK_LOG_SUCCESS("Recreating Swapchain..........");
    vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());

    CleanUpCommandListResources();

    RecreateCommandPipeline();
    RecordCommands();
}

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

    wireframeFixedPipelineFuncs.SetInputAssemblyStageInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    wireframeFixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    wireframeFixedPipelineFuncs.SetRasterizerSCI(true);
    wireframeFixedPipelineFuncs.SetMultiSampleSCI();
    wireframeFixedPipelineFuncs.SetDepthStencilSCI();
    wireframeFixedPipelineFuncs.SetColorBlendSCI();
    wireframeFixedPipelineFuncs.SetDynamicSCI();

    // Create the texture
    gridTexture.CreateTexture("./data/textures/TestGrid_1024.png", cmdPoolManager);

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

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());
    wireframeGraphicsPipeline.Create(shaderInfo, wireframeFixedPipelineFuncs, renderPassManager.GetRenderPass());

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), swapchainManager.GetSwapExtent());

    swapCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());
    imguiCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());

    // Create the triangle vertex buffer
    triVBO.Create(rainbowTriangleVertices, cmdPoolManager);
    triIBO.Create(rainbowTriangleIndices, cmdPoolManager);

    quadVBO.Create(whiteQuadVertices, cmdPoolManager);
    quadIBO.Create(whiteQuadIndices, cmdPoolManager);

    // Budda vbo and  ibo
    LoadModel("./data/models/quad.obj", buddaVertices, buddaIndices);
    std::cout << "Verts " << buddaVertices.size() << std::endl;
    buddaVBO.Create(buddaVertices, cmdPoolManager);
    buddaIBO.Create(buddaIndices, cmdPoolManager);

    cubeVBO.Create(cubeVertices, cmdPoolManager);

    // Record the commands
    // RecordCommands();
}

void Application::RecordCommands()
{
    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    auto descriptorSets = mvpUniformBuffer.GetSets();
    for (int i = 0; i <  cmdBuffers.size(); i++) {
        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        // renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
        if(!enableWireframe)
            graphicsPipeline.Bind(cmdBuffers[i]);
        else
            wireframeGraphicsPipeline.Bind(cmdBuffers[i]);
        // Bind buffer to the comands
        triVBO.Bind(cmdBuffers[i]);
        triIBO.Bind(cmdBuffers[i]);

        // Bind the push contants
        modelPCData.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);


        // Bind the descriptor sets
        if(!enableWireframe)
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedPipelineFuncs.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
        else
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframeFixedPipelineFuncs.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

        // Issue the draw command
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        // Drawing another white quad
        quadVBO.Bind(cmdBuffers[i]);
        quadIBO.Bind(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);

        cubeVBO.Bind(cmdBuffers[i]);
        vkCmdDraw(cmdBuffers[i], 36, 1, 0, 0);

        modelPCData.model = modelTransform.transformMatrix;//glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedPipelineFuncs.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstantData), &modelPCData);

        // Draw the budda model
        buddaVBO.Bind(cmdBuffers[i]);
        buddaIBO.Bind(cmdBuffers[i]);
        // vkCmdDrawIndexed(cmdBuffers[i], buddaIndices.size(), 1, 0, 0, 0);
        vkCmdDraw(cmdBuffers[i], buddaVertices.size(), 1, 0, 0);

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
        RPinfo.clearValueCount = 0;
        VkClearValue clearColor = { {{0.0, 1.0, 1.0, 1.0f}} };
        RPinfo.pClearValues = nullptr;
        vkCmdBeginRenderPass(imguiCmdBuffers.GetBufferAt(i), &RPinfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCmdBuffers.GetBufferAt(i));

        vkCmdEndRenderPass(imguiCmdBuffers.GetBufferAt(i));
        vkEndCommandBuffer(imguiCmdBuffers.GetBufferAt(i));
    }
}

void Application::CleanUpCommandListResources()
{
    framebufferManager.Destroy();
    gridTexture.Destroy();
    cubeVBO.Destroy();
    triVBO.Destroy();
    triIBO.Destroy();
    quadVBO.Destroy();
    quadIBO.Destroy();
    buddaVBO.Destroy();
    buddaIBO.Destroy();
    mvpUniformBuffer.Destroy();
    graphicsPipeline.Destroy();
    wireframeGraphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    wireframeFixedPipelineFuncs.DestroyPipelineLayout();
    swapchainManager.Destroy();
}

void Application::UpdateMVPUBO(uint32_t currentImageIndex)
{
    UniformBufferObject ubo{};
    // ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.view = camera.GetViewMatrix();
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)swapchainManager.GetSwapExtent().width / swapchainManager.GetSwapExtent().height, 0.01f, 100.0f);
    ubo.proj[1][1] *= -1;

    mvpUniformBuffer.UpdateBuffer(ubo, currentImageIndex);
}

void Application::CleanUpImGuiResources()
{
    // ImGUi render pass
    vkDestroyRenderPass(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), imguiRenderPass, nullptr);

    // Resources to destroy when the program ends
    ImGui_ImplVulkan_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Imgui Descriptor pool
    vkDestroyDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), imguiDescriptorPool, nullptr);

}
/******************************* ImGui Callbacks *******************************/
void Application::ImGuiError(VkResult err)
{
    if(VK_CALL(err))
        throw std::runtime_error("ImGui Error!");
}

void Application::OnImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(swapchainManager.GetSwapExtent().width / 2, 0, swapchainManager.GetSwapExtent().width, swapchainManager.GetSwapExtent().height);
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    modelTransform = modelTransform.AttachGuizmo(globalOperation, camera.GetViewRHMatrix(), glm::perspectiveRH(glm::radians(45.0f), (float)swapchainManager.GetSwapExtent().width / swapchainManager.GetSwapExtent().height, 0.01f, 100.0f));

    ImGui::ShowDemoWindow();
    ImGui::Begin("Yeah Bitch!");
    {
        ImGui::Text("Hello, world %d", 123);
        static ImVec4 color = {0.84, 0.44, 0.48, 1.0f};;
        ImGui::Text("Background Color"); ImGui::SameLine(); ImGui::ColorEdit4("Mycolor#2", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        clearColor[0] = color.x;
        clearColor[1] = color.y;
        clearColor[2] = color.z;
        clearColor[3] = color.w;
        renderPassManager.SetClearColor(clearColor);
        ImGui::Checkbox("Wireframe Mode ", &enableWireframe);
        const char* items[] = { "Translate", "Rotate", "Scale"};
        static const char* current_item = NULL;

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
            // if(!strcmp(current_item, "Translate"))
            //     globalOperation = ImGuizmo::TRANSLATE;
            // else if(!strcmp(current_item, "Rotate"))
            //     globalOperation = ImGuizmo::ROTATE;
            // else if(!strcmp(current_item, "Scale"))
            //     globalOperation = ImGuizmo::SCALE;
    }
    ImGui::End();
}

void Application::OnUpdate(double dt) { }

void Application::LoadModel(std::string path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }
    std::cout << "Beginning to load model at path : " << path << std::endl;
    std::unordered_map<Vertex, uint16_t> uniqueVertices{};

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
            };
            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint16_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}
