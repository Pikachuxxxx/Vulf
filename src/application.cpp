#include "application.h"

#include "utils/VulkanCheckResult.h"

#include <sstream>

/**************************** Application Flow ********************************/
void Application::Run()
{
    InitWindow();
    InitVulkan();
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
    mvpUBODSLayout.CreateDescriptorSetLayout();
    RecordCommandLists();

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

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window->getGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VKInstance::GetInstanceManager()->GetInstance();
    init_info.PhysicalDevice = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU();
    init_info.Device = VKLogicalDevice::GetDeviceManager()->GetLogicalDevice();
    init_info.QueueFamily = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGraphicsFamilyIndex();
    init_info.Queue = VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool.GetDescriptorPool();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = swapchainManager.GetSwapImageCount();
    init_info.ImageCount = swapchainManager.GetSwapImageCount();
    init_info.CheckVkResultFn = ImGuiError;
    // ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

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
        /*
        // Changing the clear color every frame, whilst also waiting for the commands to finish before re-recording them
        auto cmdBuffers = swapCmdBuffers.GetBuffers();
        auto framebuffers = framebufferManager.GetFramebuffers();
        for (int i = 0; i < cmdBuffers.size(); i++)
        {
            vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());
            swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
            renderPassManager.SetClearColor(abs(cos(glfwGetTime())), 0.44, 0.48);
            renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
            graphicsPipeline.Bind(cmdBuffers[i]);
            vkCmdDraw(cmdBuffers[i], 3, 1, 0, 0);
            renderPassManager.EndRenderPass(cmdBuffers[i]);
    		swapCmdBuffers.EndRecordingBuffer(cmdBuffers[i]);
        }
        */
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
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(swapCmdBuffers.GetBufferAt(imageIndex));
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

    cmdPoolManager.Destroy();
    mvpUBODSLayout.Destroy();
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

    RecordCommandLists();
}

void Application::RecordCommandLists()
{
    swapchainManager.Init(window->getGLFWwindow());

    fixedPipelineFuncs.SetInputAssemblyStageInfo(Topology::TRIANGLES);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI();
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();
    fixedPipelineFuncs.SetPipelineLayout(mvpUBODSLayout.GetLayout());

    renderPassManager.Init(swapchainManager.GetSwapFormat());
    std::vector<VkPipelineShaderStageCreateInfo>  shaderInfo = {vertexShader.GetShaderStageInfo(), fragmentShader.GetShaderStageInfo()};

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), swapchainManager.GetSwapExtent());

    swapCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());

    // Create the triangle vertex buffer
    triVBO.Create(rainbowTriangleVertices, cmdPoolManager.GetPool());
    triIBO.Create(rainbowTriangleIndices, cmdPoolManager.GetPool());

    quadVBO.Create(whiteQuadVertices, cmdPoolManager.GetPool());
    quadIBO.Create(whiteQuadIndices, cmdPoolManager.GetPool());

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    mvpUBOs.resize(swapchainManager.GetSwapImageCount());
    for (size_t i = 0; i < swapchainManager.GetSwapImageCount(); i++) {
        mvpUBOs[i].CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    descriptorPool.CreatePool(swapchainManager.GetSwapImageCount());

    set.CreateSets(swapchainManager.GetSwapImageCount(), descriptorPool.GetDescriptorPool(), mvpUBODSLayout.GetLayout(), mvpUBOs, bufferSize);

    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    auto descriptorSets = set.GetSets();
    for (int i = 0; i < cmdBuffers.size(); i++) {
        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
        graphicsPipeline.Bind(cmdBuffers[i]);
        // Bind buffer to the comands
        triVBO.Bind(cmdBuffers[i]);
        triIBO.Bind(cmdBuffers[i]);
        // Bind the descriptor sets
        vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedPipelineFuncs.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
        // Issue the draw command
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        // Drawing another white quad
        quadVBO.Bind(cmdBuffers[i]);
        quadIBO.Bind(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        renderPassManager.EndRenderPass(cmdBuffers[i]);
		swapCmdBuffers.EndRecordingBuffer(cmdBuffers[i]);
    }
}

void Application::CleanUpCommandListResources()
{
    framebufferManager.Destroy();
    triVBO.Destroy();
    triIBO.Destroy();
    quadVBO.Destroy();
    quadIBO.Destroy();
    for (size_t i = 0; i < swapchainManager.GetSwapImageCount(); i++) {
        mvpUBOs[i].DestroyBuffer();
    }
    descriptorPool.Destroy();
    graphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    swapchainManager.Destroy();
}

void Application::UpdateMVPUBO(uint32_t currentImageIndex)
{
    UniformBufferObject ubo{};
    ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, delta * 10.0f + 0.2f, 0.0f));
    ubo.view = camera.GetViewMatrix();
    ubo.proj = glm::mat4(1.0f);//glm::perspective(glm::radians(45.0f), 800.f / 600.0f, 0.01f, 100.0f);
    ubo.proj = glm::mat4(1.0f);//glm::ortho(-200, 200, -150, 150, 0, 1);
    // ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), mvpUBOs[currentImageIndex].GetBufferMemory(), 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), mvpUBOs[currentImageIndex].GetBufferMemory());
}
/******************************* ImGui Callbacks *******************************/
void Application::ImGuiError(VkResult err)
{
    if(VK_CALL(err))
        throw std::runtime_error("ImGui Error!");
}
