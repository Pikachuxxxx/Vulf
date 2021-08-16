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
    glfwSetWindowUserPointer(window->getGLFWwindow(), this);
    glfwSetFramebufferSizeCallback(window->getGLFWwindow(), resize_callback);
}

void Application::InitVulkan()
{
    VKInstance::GetInstanceManager()->Init("Hello Vulkan", window->getGLFWwindow());

    VKLogicalDevice::GetDeviceManager()->Init();

    swapchainManager.Init(window->getGLFWwindow());

    vertexShader.CreateShader("./src/shaders/spir-v/vert.spv", ShaderType::VERTEX_SHADER);
    fragmentShader.CreateShader("./src/shaders/spir-v/frag.spv", ShaderType::FRAGMENT_SHADER);

    // fixedPipelineFuncs.SetVertexInputSCI();
    fixedPipelineFuncs.SetInputAssemblyStageInfo(Topology::TRIANGLES);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI();
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();
    fixedPipelineFuncs.SetPipelineLayout();

    renderPassManager.Init(swapchainManager.GetSwapFormat());
    std::vector<VkPipelineShaderStageCreateInfo>  shaderInfo = {vertexShader.GetShaderStageInfo(), fragmentShader.GetShaderStageInfo()};

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), swapchainManager.GetSwapExtent());

    cmdPoolManager.Init();

    swapCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());

    // Create the triangle vertex buffer
    VkDeviceSize vertexDataSize = sizeof(rainbowTriangleVertices[0]) * rainbowTriangleVertices.size();
    triangleStagingBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    triangleStagingBuffer.MapVertexBufferData(rainbowTriangleVertices);
    triangleVertexBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    triangleStagingBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), triangleVertexBuffer.GetBuffer(), vertexDataSize);
    triangleStagingBuffer.DestroyBuffer();
    // Index buffer for the quad
    VkDeviceSize indexDataSize = sizeof(rainbowTriangleIndices[0]) * rainbowTriangleIndices.size();
    triangleStagingIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    triangleStagingIndexBuffer.MapIndexBufferData(rainbowTriangleIndices);
    triangleIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    triangleStagingIndexBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), triangleIndexBuffer.GetBuffer(), indexDataSize);
    triangleStagingIndexBuffer.DestroyBuffer();

    // Create the white quad vertex buffer
    vertexDataSize = sizeof(whiteQuadVertices[0]) * whiteQuadVertices.size();
    whiteQuadStagingBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    whiteQuadStagingBuffer.MapVertexBufferData(whiteQuadVertices);
    whiteQuadVertexBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    whiteQuadStagingBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), whiteQuadVertexBuffer.GetBuffer(), vertexDataSize);
    whiteQuadStagingBuffer.DestroyBuffer();
    // Index buffer for the white quad
    indexDataSize = sizeof(whiteQuadIndices[0]) * whiteQuadIndices.size();
    whiteQuadStagingIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    whiteQuadStagingIndexBuffer.MapIndexBufferData(whiteQuadIndices);
    whiteQuadIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    whiteQuadStagingIndexBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), whiteQuadIndexBuffer.GetBuffer(), indexDataSize);
    whiteQuadStagingIndexBuffer.DestroyBuffer();

    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    for (int i = 0; i < cmdBuffers.size(); i++)
    {
        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
        graphicsPipeline.Bind(cmdBuffers[i]);
        // Bind buffer to the comands
        triangleVertexBuffer.BindVertexBuffer(cmdBuffers[i]);
        triangleIndexBuffer.BindIndexBuffer(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        // Drawing another white quad
        whiteQuadVertexBuffer.BindVertexBuffer(cmdBuffers[i]);
        whiteQuadIndexBuffer.BindIndexBuffer(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        renderPassManager.EndRenderPass(cmdBuffers[i]);
		swapCmdBuffers.EndRecordingBuffer(cmdBuffers[i]);
    }

    // Create the synchronization stuff
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderingFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(cmdBuffers.size(), VK_NULL_HANDLE);
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

void Application::MainLoop()
{
    lastTime = glfwGetTime();;
    while (!window->closed()) {
        window->Update();
        camera.Update(*window);

        double currentTime = glfwGetTime();
        double delta = currentTime - lastTime;
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

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
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
    cmdPoolManager.Destroy();
    framebufferManager.Destroy();
    triangleVertexBuffer.DestroyBuffer();
    triangleIndexBuffer.DestroyBuffer();
    whiteQuadVertexBuffer.DestroyBuffer();
    whiteQuadIndexBuffer.DestroyBuffer();
    graphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    vertexShader.DestroyModule();
    fragmentShader.DestroyModule();
    swapchainManager.Destroy();
    VKLogicalDevice::GetDeviceManager()->Destroy();
    VKInstance::GetInstanceManager()->Destroy();
}
/******************************************************************************/
void Application::RecreateSwapchain()
{
    VK_LOG_SUCCESS("Recreating Swapchain..........");
    vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());

    framebufferManager.Destroy();
    triangleVertexBuffer.DestroyBuffer();
    triangleIndexBuffer.DestroyBuffer();
    whiteQuadVertexBuffer.DestroyBuffer();
    whiteQuadIndexBuffer.DestroyBuffer();
    graphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    swapchainManager.Destroy();

    swapchainManager.Init(window->getGLFWwindow());
    fixedPipelineFuncs.SetInputAssemblyStageInfo(Topology::TRIANGLES);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI();
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();
    fixedPipelineFuncs.SetPipelineLayout();

    renderPassManager.Init(swapchainManager.GetSwapFormat());
    std::vector<VkPipelineShaderStageCreateInfo>  shaderInfo = {vertexShader.GetShaderStageInfo(), fragmentShader.GetShaderStageInfo()};

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), swapchainManager.GetSwapExtent());

    // Create the triangle vertex buffer
    VkDeviceSize vertexDataSize = sizeof(rainbowTriangleVertices[0]) * rainbowTriangleVertices.size();
    triangleStagingBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    triangleStagingBuffer.MapVertexBufferData(rainbowTriangleVertices);
    triangleVertexBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    triangleStagingBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), triangleVertexBuffer.GetBuffer(), vertexDataSize);
    triangleStagingBuffer.DestroyBuffer();
    // Index buffer for the quad
    VkDeviceSize indexDataSize = sizeof(rainbowTriangleIndices[0]) * rainbowTriangleIndices.size();
    triangleStagingIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    triangleStagingIndexBuffer.MapIndexBufferData(rainbowTriangleIndices);
    triangleIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    triangleStagingIndexBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), triangleIndexBuffer.GetBuffer(), indexDataSize);
    triangleStagingIndexBuffer.DestroyBuffer();

    // Create the white quad vertex buffer
    vertexDataSize = sizeof(whiteQuadVertices[0]) * whiteQuadVertices.size();
    whiteQuadStagingBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    whiteQuadStagingBuffer.MapVertexBufferData(whiteQuadVertices);
    whiteQuadVertexBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    whiteQuadStagingBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), whiteQuadVertexBuffer.GetBuffer(), vertexDataSize);
    whiteQuadStagingBuffer.DestroyBuffer();
    // Index buffer for the white quad
    indexDataSize = sizeof(whiteQuadIndices[0]) * whiteQuadIndices.size();
    whiteQuadStagingIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    whiteQuadStagingIndexBuffer.MapIndexBufferData(whiteQuadIndices);
    whiteQuadIndexBuffer.CreateBuffer(indexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    whiteQuadStagingIndexBuffer.CopyBufferToDevice(cmdPoolManager.GetPool(), whiteQuadIndexBuffer.GetBuffer(), indexDataSize);
    whiteQuadStagingIndexBuffer.DestroyBuffer();

    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    for (int i = 0; i < cmdBuffers.size(); i++)
    {
        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.BeginRenderPass(cmdBuffers[i], framebuffers[i], swapchainManager.GetSwapExtent());
        graphicsPipeline.Bind(cmdBuffers[i]);
        // Bind buffer to the comands
        triangleVertexBuffer.BindVertexBuffer(cmdBuffers[i]);
        triangleIndexBuffer.BindIndexBuffer(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        // Drawing another white quad
        whiteQuadVertexBuffer.BindVertexBuffer(cmdBuffers[i]);
        whiteQuadIndexBuffer.BindIndexBuffer(cmdBuffers[i]);
        vkCmdDrawIndexed(cmdBuffers[i], 6, 1, 0, 0, 0);
        renderPassManager.EndRenderPass(cmdBuffers[i]);
		swapCmdBuffers.EndRecordingBuffer(cmdBuffers[i]);
    }
}
/******************************* GLFW Callbacks *******************************/
void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Application::resize_callback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}
