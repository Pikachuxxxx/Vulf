#include "application.h"

#include "utils/VulkanCheckResult.h"

#include <sstream>

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

        ImGui_ImplVulkan_SetMinImageCount(2);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
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

    fixedPipelineFuncs.SetInputAssemblyStageInfo(Topology::TRIANGLES);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI();
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();

    // Create the uniform buffer
    mvpUniformBuffer.CreateUniformBuffer(swapchainManager.GetSwapImageCount());
    auto layout = mvpUniformBuffer.GetDescriptorSetLayout();
    fixedPipelineFuncs.SetPipelineLayout(layout);

    renderPassManager.Init(swapchainManager.GetSwapFormat());
    std::vector<VkPipelineShaderStageCreateInfo>  shaderInfo = {vertexShader.GetShaderStageInfo(), fragmentShader.GetShaderStageInfo()};

    graphicsPipeline.Create(shaderInfo, fixedPipelineFuncs, renderPassManager.GetRenderPass());

    framebufferManager.Create(renderPassManager.GetRenderPass(), swapchainManager.GetSwapImageViews(), swapchainManager.GetSwapExtent());

    swapCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());
    imguiCmdBuffers.AllocateBuffers(cmdPoolManager.GetPool());

    // Create the triangle vertex buffer
    triVBO.Create(rainbowTriangleVertices, cmdPoolManager.GetPool());
    triIBO.Create(rainbowTriangleIndices, cmdPoolManager.GetPool());

    quadVBO.Create(whiteQuadVertices, cmdPoolManager.GetPool());
    quadIBO.Create(whiteQuadIndices, cmdPoolManager.GetPool());

    // Record the commands
    // RecordCommands();
}

void Application::RecordCommands()
{
    auto cmdBuffers = swapCmdBuffers.GetBuffers();
    auto framebuffers = framebufferManager.GetFramebuffers();
    auto descriptorSets = mvpUniformBuffer.GetSets();
    for (int i = 0; i < cmdBuffers.size(); i++) {
        swapCmdBuffers.RecordBuffer(cmdBuffers[i]);
        // renderPassManager.SetClearColor(0.85, 0.44, 0.48);
        renderPassManager.SetClearColor(abs(cos(glfwGetTime())), 0.44, 0.48);
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
        RPinfo.clearValueCount = 1;
        VkClearValue clearColor = { {{0.0, 1.0, 1.0, 1.0f}} };
        RPinfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(imguiCmdBuffers.GetBufferAt(i), &RPinfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCmdBuffers.GetBufferAt(i));

        vkCmdEndRenderPass(imguiCmdBuffers.GetBufferAt(i));
        vkEndCommandBuffer(imguiCmdBuffers.GetBufferAt(i));
    }
}

void Application::CleanUpCommandListResources()
{
    framebufferManager.Destroy();
    triVBO.Destroy();
    triIBO.Destroy();
    quadVBO.Destroy();
    quadIBO.Destroy();
    mvpUniformBuffer.Destroy();
    graphicsPipeline.Destroy();
    renderPassManager.Destroy();
    fixedPipelineFuncs.DestroyPipelineLayout();
    swapchainManager.Destroy();
}

void Application::UpdateMVPUBO(uint32_t currentImageIndex)
{
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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
    ImGui::ShowDemoWindow();
    static char* buf = new char[256];
    static float f;
    ImGui::Begin("Yeah Bitch!");
    {
        ImGui::Text("Hello, world %d", 123);
        if (ImGui::Button("Save"))
        {
            ImGui::Text("Hello, world %d", 123);
        }
        ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        static ImVec4 color;
        ImGui::ColorEdit4("Mycolor#2", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    }
    ImGui::End();
}
