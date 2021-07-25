#include "application.h"

#include "utils/VulkanCheckResult.h"

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
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // Create the window
    window = glfwCreateWindow(800, 600, "Hello Vulkan again!", nullptr, nullptr);
    // glfwSetKeyCallback(window, key_callback);
}

void Application::InitVulkan()
{
    VKInstance::GetInstanceManager()->Init("Hello Vulkan", window);

    VKLogicalDevice::GetDeviceManager()->Init();

    swapchainManager.Init(window);

    vertexShader.CreateShader("./src/shaders/spir-v/trivert.spv", ShaderType::VERTEX_SHADER);
    fragmentShader.CreateShader("./src/shaders/spir-v/trifrag.spv", ShaderType::FRAGMENT_SHADER);

    fixedPipelineFuncs.SetVertexInputSCI();
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
    swapCmdBuffers.RecordBuffers();
    renderPassManager.SetClearColor(0.85, 0.44, 0.48);
    renderPassManager.BeginRenderPass(swapCmdBuffers, framebufferManager.GetFramebuffers(), swapchainManager.GetSwapExtent());
    graphicsPipeline.Bind(swapCmdBuffers);

    for(const auto& buffers : swapCmdBuffers.GetBuffers())
        vkCmdDraw(buffers, 3, 1, 0, 0);

    renderPassManager.EndRenderPass(swapCmdBuffers);
    swapCmdBuffers.EndRecordingBuffers();

    // Create the synchronization stuff
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if(VK_CALL(vkCreateSemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore)))
        throw std::runtime_error("Cannot make semaphore!");
    else VK_LOG("Created imageAvailableSemaphore");
    if(VK_CALL(vkCreateSemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &semaphoreInfo, nullptr, &renderingFinishedSemaphore)))
        throw std::runtime_error("Cannot make semaphore!");
    else VK_LOG("Created renderingFinishedSemaphore");

}


void Application::MainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawFrame();
    }
    vkDeviceWaitIdle(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice());
}

void Application::DrawFrame()
{
    // Acquire the image to render onto
    uint32_t imageIndex;
    if(VK_CALL(vkAcquireNextImageKHR(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), swapchainManager.GetSwapchainKHR(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex)))
        throw std::runtime_error("Cannot acquire next image!");

    VK_LOG("Image index : ", imageIndex);
    // Submit the command buffer queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphore[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(swapCmdBuffers.GetBufferAt(imageIndex));
    VkSemaphore signalSemaphores[] = {renderingFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(VK_CALL(vkQueueSubmit(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE)))
        throw std::runtime_error("Cannot submit command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchainManager.GetSwapchainKHR()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    if(VK_CALL(vkQueuePresentKHR(VKLogicalDevice::GetDeviceManager()->GetPresentQueue(), &presentInfo)))
        throw std::runtime_error("Cannot submit presentation queue!");

}

void Application::CleanUp()
{
    vkDestroySemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), renderingFinishedSemaphore, nullptr);
    cmdPoolManager.Destroy();
    framebufferManager.Destroy();
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
/******************************* GLFW Callbacks *******************************/
