#include "VulfBase.h"

// STL
#include <sstream>

// Utilities
#include "utils/VulkanCheckResult.h"


// TibyObj
//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tinyobj/tiny_obj_loader.h>

namespace Vulf {

// Public
/*******************************************************************************
 *                              Init Application                               *
 ******************************************************************************/

    VulfBase::~VulfBase() { }

    void VulfBase::Run() {
        InitResources();
        InitWindow();
        InitVulkan();
        InitImGui();

        RenderLoop();
#ifdef _WIN32
        OPTICK_SHUTDOWN();
#endif
    }

// Private
/*******************************************************************************
 *                              Init Application                               *
 ******************************************************************************/
    void VulfBase::InitResources() {

    }

    void VulfBase::InitWindow() {
        //
        m_Window = new Window(m_AppName.c_str(), width, height);
    }

    void VulfBase::InitVulkan() {
        // Create the Vulkan Instance
        // TODO: [ICEBOX] Use a proper signature for the application name (same is given for the window as well)
        Instance::Get()->Init(m_AppName.c_str(), m_Window->getGLFWwindow(), true);

        // Create the Device
        Device::Get()->Init();

#ifdef _WIN32
        auto device         = Device::Get()->get_handle();
        auto physicalDevice = Device::Get()->get_gpu();
        auto queuefam       = Device::Get()->get_graphics_queue();
        uint32_t numQueues  = Device::Get()->get_graphics_family_index();
        OPTICK_GPU_INIT_VULKAN(&device, &physicalDevice, &queuefam, &numQueues, 1, nullptr);
#endif
        // Load the shaders
        LoadShaders();

        // Load Assets such as Models and Audio files etc.
        LoadAssets();

        // Initialize the Command Pool and Build
        BuildCommandPool();

        // Build the command pipelines
        BuildCommandPipeline();

        // Create the synchronization primitives
        CreateSynchronizationPrimitives();
    }

    void VulfBase::InitImGui() {

        m_ImGuiOVerlay.init();
        m_ImGuiOVerlay.upload_ui_font("FiraCode-Light.ttf");
        m_ImGuiOVerlay.prepare_pipeline(_def_RenderPass.GetRenderPass());
        ImGui_ImplGlfw_InitForVulkan(m_Window->getGLFWwindow(), true);
    }

    ////////////////////////////////////////////////////////////////////////////
    // PRIVATE
    // Render Loop Beginning
    void VulfBase::RenderLoop() {

        OnStart();

        while (!m_Window->closed()) {
#ifdef _WIN32
            OPTICK_FRAME("MainThread");
            OPTICK_EVENT();
#endif
            // Update the window for input events
            m_Window->Update();

            auto tStart = std::chrono::high_resolution_clock::now();

            m_FrameCounter++;
            auto tEnd = std::chrono::high_resolution_clock::now();
            m_FrameTimer = std::chrono::duration<double, std::milli>(tEnd - tStart); // in Ms

            // Update the camera
            m_Camera.Update(*m_Window, m_FrameTimer.count());

            OnUpdate(m_FrameTimer.count());

            float fpsTimer = std::chrono::duration<double, std::milli>(tEnd - m_LastTimestamp).count();
            if (fpsTimer > 1000.0f) {
                // VK_LOG("FPS : ", m_FrameCounter);
                auto title = m_AppName + " [FPS : " + std::to_string(m_FrameCounter) + "]";
                m_Window->setTitle(title);
                m_FrameCounter = 0;
                m_LastTimestamp = tEnd;
            }

            DrawFrame();
        }
        vkDeviceWaitIdle(VKDEVICE);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(VKDEVICE, m_ImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(VKDEVICE, m_RenderingFinishedSemaphores[i], nullptr);
            vkDestroyFence(VKDEVICE, m_InFlightFences[i], nullptr);
        }
    }
    ////////////////////////////////////////////////////////////////////////////

// Protected
/*******************************************************************************
 *                  Client Side Customization(default behavior)                *
 ******************************************************************************/

    ////////////////////////////////////////////////////////////////////////////
    // PRIVATE
    void VulfBase::BuildCommandPipeline() {
        // Create the swapchain
        BuildSwapchain();

        // Build texture and image resources
        BuildTextureResources();

        // Create Buffer Resources such as Vertex, Index and Uniform buffers
        BuildBufferResource();

        // Build the fixed pipeline stage
        BuildFixedPipeline();

        // Build render passes
        BuildRenderPass();

        // Create the Graphics Pipeline
        BuildGraphicsPipeline();

        // Build the framebuffers
        BuildFramebuffer();

        // Build the command buffers
        BuildCommandBuffers();

        // Build the descriptor sets
        BuildDescriptorSets();
    }
    ////////////////////////////////////////////////////////////////////////////

    void VulfBase::BuildCommandPool() {

        // This is a graphics pool for allocationf command bufers that can only be sunmitted to graphics queues
        _def_CommandPool.Init(Device::Get()->get_graphics_queue_index());
    }

    void VulfBase::BuildTextureResources() {

    }

    void VulfBase::BuildSwapchain() {
        _def_Swapchain.Init(m_Window->getGLFWwindow());
    }

    void VulfBase::BuildFixedPipeline() {

    }

    void VulfBase::BuildRenderPass() {
        _def_RenderPass.Init(_def_Swapchain.get_format());
    }

    void VulfBase::BuildGraphicsPipeline() {

    }

    void VulfBase::BuildFramebuffer() {

    }

    void VulfBase::BuildCommandBuffers() {
        // Build the default (aka submissible) command buffers onto which we can record draw commands
        // _def_SubmissionCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        // One for each frame in flight
    }

    void VulfBase::BuildDescriptorSets() {

    }

//-----------------------------------------------------------------------------//
// Draw Frame + Submission logic + cleanup

    void VulfBase::CleanUpPipeline() {

        // Default swapchain manager
        _def_Swapchain.Destroy();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Draw the Frame
    void VulfBase::DrawFrame() {
        ZoneScopedC(0xff0000)
#ifdef _WIN32
        OPTICK_EVENT();
        OPTICK_GPU_EVENT("Draw Frame");
#endif
        vkWaitForFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame]);

        // Acquire the image to render onto
        VkResult result = vkAcquireNextImageKHR(VKDEVICE, _def_Swapchain.get_handle(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Cannot acquire next image!");
        }
        //
        // if(m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
        //     vkWaitForFences(VKDEVICE, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
        //
        // m_ImagesInFlight[m_ImageIndex] = m_InFlightFences[m_CurrentFrame];
        // Reset and record the command buffer
        // if(VK_CALL(vkResetCommandBuffer(_def_SubmissionCommandBuffers.GetBufferAt(m_CurrentFrame), /*VkCommandBufferResetFlagBits*/ 0)))
        //     throw std::runtime_error("Cannot reset the command buffer");
        // else
        //     VK_LOG_SUCCESS("Command buffer reset successful");


        ImGui_ImplGlfw_NewFrame();
        OnImGui();
        VkCommandBuffer buf;
        OnRender(buf, m_ImageIndex);

        OnUpdateBuffers(m_ImageIndex);

        SubmitFrame();

        FrameMark
    }
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Sunmit the frame for presentation and the command buffers for execution
    void VulfBase::SubmitFrame() {
        ZoneScopedC(0x0000ff);
#ifdef _WIN32
        OPTICK_EVENT();
#endif
        VkResult result;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphore[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;

        // std::vector<VkCommandBuffer> buffers(submissionCommandBuffers.size());
        // buffers.clear();
        // for (int i = 0; i < submissionCommandBuffers.size(); i++) {
        //     buffers.push_back(submissionCommandBuffers[i].GetBufferAt(m_ImageIndex));
        // }

        submitInfo.pCommandBuffers = nullptr;//&(_def_SubmissionCommandBuffers.GetBufferAt(m_CurrentFrame));
        VkSemaphore signalSemaphores[] = {m_RenderingFinishedSemaphores[m_CurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
#ifdef _WIN32
        // OPTICK_GPU_EVENT("Reset In Flight Fences");
#endif
        vkResetFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame]);

#ifdef _WIN32
        OPTICK_GPU_EVENT("Queue Submit");
#endif
        if(VK_CALL(vkQueueSubmit(Device::Get()->get_graphics_queue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]))) {
            throw std::runtime_error("Cannot submit command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {_def_Swapchain.get_handle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_ImageIndex;

#ifdef _WIN32
        OPTICK_GPU_EVENT("Queue Present");
#endif
        result = vkQueuePresentKHR(Device::Get()->get_present_queue(), &presentInfo);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->IsResized()) {
            m_Window->SetResizedFalse();
            RecreateSwapchain();
            return;
        }
        else if(result != VK_SUCCESS) {
            throw std::runtime_error("Cannot submit presentation queue!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
    }
    ////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------//
// App update

    void VulfBase::OnStart() {

    }

    void VulfBase::OnUpdate(double dt) {

    }

    void VulfBase::OnRender(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    }

    // Update the unifomr buffers and descriptor sets
    void VulfBase::OnUpdateBuffers(uint32_t imageIndex) {

    }

    void VulfBase::OnImGui() {

    }

//-----------------------------------------------------------------------------//
// Private methods

    void VulfBase::RecreateSwapchain() {
        ZoneScopedC(0xff00ff);
        VK_LOG_SUCCESS("Recreating Swapchain..........");

        vkDeviceWaitIdle(VKDEVICE);

        CleanUpPipeline();

        BuildCommandPipeline();
    }

    //-----------------------------------------------------------------------------//

    void VulfBase::SyncAppWithGPU(const std::vector<Fence>& fences)
    {
        vkWaitForFences(VK_DEVICE, static_cast<uint32_t>(fences.sizr()), fences.data(), waitOnAllFences, UINT64_MAX);
    }
/******************************************************************************/
}
