#include "VulfBase.h"

// STL
#include <sstream>

// Utilities
#include "utils/VulkanCheckResult.h"


// TibyObj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

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
#ifdef OPTICK_ENABLE
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

#ifdef OPTICK_ENABLE
        auto device         = Device::Get()->get_handle();
        auto physicalDevice = Device::Get()->get_gpu();
        auto queuefam       = Device::Get()->get_graphics_queue();
        uint32_t numQueues  = Device::Get()->get_graphics_queue_index();
        OPTICK_GPU_INIT_VULKAN(&device, &physicalDevice, &queuefam, &numQueues, 1);
#endif
        // Load the shaders
        LoadShaders();

        // Load Assets such as Models and Audio files etc.
        LoadAssets();

        // Initialize the Command Pool and Build
        BuildCommandPool();

        // Create the swapchain
        BuildSwapchain();

        // Build the command pipelines
        BuildCommandPipeline();

        // Create the synchronization primitives
        InitSyncPrimitives();
    }

    void VulfBase::InitImGui() {

        m_ImGuiOVerlay.init();
        m_ImGuiOVerlay.upload_ui_font("FiraCode-Light.ttf");
        m_ImGuiOVerlay.prepare_pipeline(baseRenderPass.get_handle());
        // This is not necessary if one creates their own hooks for Input system and ImGui (GLFW does it for us)
        ImGui_ImplGlfw_InitForVulkan(m_Window->getGLFWwindow(), true);
    }

    ////////////////////////////////////////////////////////////////////////////
    // PRIVATE
    // Render Loop Beginning
    void VulfBase::RenderLoop() {

        OnStart();

        while (!m_Window->closed()) {
#ifdef OPTICK_ENABLE
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
                m_FPS = m_FrameCounter;

                // Update the moving avg FPS
                ++avgQnt;
                avgFPS += ((get_fps() - avgFPS)/avgQnt);

                // Update the min and max fps
                if(get_fps() > maxFPS)
                    maxFPS = get_fps();

                if(get_fps() < minFPS)
                    minFPS = get_fps();

                auto title = m_AppName + " [FPS : " + std::to_string(m_FrameCounter) + "]";
                m_Window->setTitle(title);
                m_FrameCounter = 0;
                m_LastTimestamp = tEnd;
            }

            DrawFrame();
        }
        vkDeviceWaitIdle(VKDEVICE);

        CleanUpPipeline();

        QuitApp();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_ImageAvailableSemaphores[i].Destroy();
            m_RenderFinishedSemaphores[i].Destroy();
            m_InFlightFences[i].Destroy();
        }
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        graphicsCommandPool.Destroy(); // --> Automatically frees cmdBuffers out of existence
    }
    ////////////////////////////////////////////////////////////////////////////

// Protected
/*******************************************************************************
 *                  Client Side Customization(default behavior)                *
 ******************************************************************************/

    ////////////////////////////////////////////////////////////////////////////
    // PRIVATE
    void VulfBase::BuildCommandPipeline() {

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

        // This is a graphics pool for allocating command buffers that can only be submitted to graphics queues
        graphicsCommandPool.Init(Device::Get()->get_graphics_queue_index());

        // This command pool is for allocating the compute command buffers
        computeCommandPool.Init(Device::Get()->get_compute_queue_index());

    }

    void VulfBase::BuildSwapchain() {
        baseSwapchain.Init(m_Window->getGLFWwindow());
    }

    void VulfBase::BuildTextureResources() {

    }

    void VulfBase::BuildBufferResource() {

    }

    void VulfBase::BuildFixedPipeline() {

    }

    void VulfBase::BuildRenderPass() {
        baseRenderPass.Init(baseSwapchain.get_format());
    }

    void VulfBase::BuildGraphicsPipeline() {

    }

    void VulfBase::BuildFramebuffer() {

    }

    void VulfBase::BuildCommandBuffers() {
        // Build the default (aka submissble) command buffers onto which we can record draw commands
        // _def_SubmissionCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        // One for each frame in flight
        m_DrawCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_ComputeCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_DrawCmdBuffers[i].Init(graphicsCommandPool.get_handle());
            m_ComputeCmdBuffers[i].Init(computeCommandPool.get_handle());
        }
    }

    void VulfBase::BuildDescriptorSets() {

    }

//-----------------------------------------------------------------------------//
// cleanup --> Used by applicaion to destroy it's resources
    void VulfBase::CleanUpPipeline() {

    }
//-----------------------------------------------------------------------------//
// Draw Frame + Submission logic
    ////////////////////////////////////////////////////////////////////////////
    // Draw the Frame
    void VulfBase::DrawFrame() {
        //ZoneScopedC(0xff0000)
#ifdef OPTICK_ENABLE
        OPTICK_EVENT();
        OPTICK_GPU_EVENT("Draw Frame");
#endif
        vkWaitForFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame].get_handle(), VK_TRUE, UINT64_MAX);
        //vkResetFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame]);

        // Acquire the image to render onto
        VkResult result = vkAcquireNextImageKHR(VKDEVICE, baseSwapchain.get_handle(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame].get_handle(), VK_NULL_HANDLE, &m_ImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Cannot acquire next image!");
        }

        vkResetFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentFrame].get_handle());

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

        // Reset the command buffer first --> then record onto it
        m_DrawCmdBuffers[m_CurrentFrame].reset();
        m_ComputeCmdBuffers[m_CurrentFrame].reset();
        OnRender(m_DrawCmdBuffers[m_CurrentFrame], m_ComputeCmdBuffers[m_CurrentFrame]);

        OnUpdateBuffers(m_CurrentFrame);

        SubmitFrame();

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        FrameMark
    }
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Submit the frame for presentation and the command buffers for execution
    void VulfBase::SubmitFrame() {
        ZoneScopedC(0x0000ff);
#ifdef OPTICK_ENABLE
        OPTICK_EVENT();
#endif
        VkResult result;

        // wait and signal stages/semaphore as Vulf needs and provide an API in future to make this easier for the user and not modify source code
        VkSubmitInfo computeSubmitInfo{};
        computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
        VkSemaphore waitSemaphore[] = { m_GraphicsSemaphores[m_CurrentFrame].get_handle() };
        computeSubmitInfo.waitSemaphoreCount = 1;
        computeSubmitInfo.pWaitSemaphores = waitSemaphore;
        computeSubmitInfo.pWaitDstStageMask = waitStages;
        computeSubmitInfo.signalSemaphoreCount = 1;
        computeSubmitInfo.pSignalSemaphores = &m_ComputeSemaphores[m_CurrentFrame].get_handle();
        computeSubmitInfo.commandBufferCount = 1;
        computeSubmitInfo.pCommandBuffers = &m_ComputeCmdBuffers[m_CurrentFrame].get_handle();

        if (VK_CALL(vkQueueSubmit(Device::Get()->get_compute_queue(), 1, &computeSubmitInfo, VK_NULL_HANDLE))) {
            throw std::runtime_error("Cannot submit compute command buffer!");
        }

        VkSubmitInfo graphicsSubmitInfo{};
        graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        waitStages[0] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        graphicsSubmitInfo.waitSemaphoreCount = 2;
        VkSemaphore grpahiscWaitSemaphore[] = { m_ComputeSemaphores[m_CurrentFrame].get_handle() ,m_ImageAvailableSemaphores[m_CurrentFrame].get_handle() };
        graphicsSubmitInfo.pWaitSemaphores = grpahiscWaitSemaphore;
        graphicsSubmitInfo.pWaitDstStageMask = waitStages;

        graphicsSubmitInfo.commandBufferCount = 1;
        graphicsSubmitInfo.pCommandBuffers = &m_DrawCmdBuffers[m_CurrentFrame].get_handle();

        VkSemaphore signalSemaphores[] = { m_GraphicsSemaphores[m_CurrentFrame].get_handle(), m_RenderFinishedSemaphores[m_CurrentFrame].get_handle() };
        graphicsSubmitInfo.signalSemaphoreCount = 2;
        graphicsSubmitInfo.pSignalSemaphores = signalSemaphores;
#ifdef OPTICK_ENABLE
        // OPTICK_GPU_EVENT("Reset In Flight Fences");
#endif

#ifdef OPTICK_ENABLE
        OPTICK_GPU_EVENT("Queue Submit");
#endif
        if (VK_CALL(vkQueueSubmit(Device::Get()->get_graphics_queue(), 1, &graphicsSubmitInfo, m_InFlightFences[m_CurrentFrame].get_handle()))) {
            throw std::runtime_error("Cannot submit graphics command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame].get_handle();
        VkSwapchainKHR swapChains[] = { baseSwapchain.get_handle() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_ImageIndex;

#ifdef OPTICK_ENABLE
        OPTICK_GPU_EVENT("Queue Present");
#endif
        result = vkQueuePresentKHR(Device::Get()->get_present_queue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->IsResized()) {
            m_Window->SetResizedFalse();
            RecreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Cannot submit presentation queue!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;;
    }
    ////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------//
// App update

    void VulfBase::OnStart() {

    }

    void VulfBase::OnUpdate(double dt) {

    }

    void VulfBase::OnRender(CmdBuffer dcb, CmdBuffer ccb) {
    }

    // Update the uniform buffers and descriptor sets
    void VulfBase::OnUpdateBuffers(uint32_t frameIdx) {

    }

    void VulfBase::OnImGui() {

    }

//-----------------------------------------------------------------------------//
// Private methods

    void VulfBase::InitSyncPrimitives() {

        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_ComputeSemaphores.clear();
        m_GraphicsSemaphores.clear();
        m_InFlightFences.clear();
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_ComputeSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_GraphicsSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_ImageAvailableSemaphores[i].Init("m_ImageAvailableSemaphores");
            m_RenderFinishedSemaphores[i].Init("m_RenderFinishedSemaphores");
            m_GraphicsSemaphores[i].Init("m_GraphicsSemaphores");
            m_ComputeSemaphores[i].Init("m_ComputeSemaphores");
            m_InFlightFences[i].Init();
        }
    }

    void VulfBase::RecreateSwapchain() {
        //ZoneScopedC(0xff00ff);
        VK_LOG_SUCCESS("Recreating Swapchain..........");

        vkDeviceWaitIdle(VKDEVICE);

        // Destroy
        CleanUpPipeline();

        QuitApp(); // Destroy sync primitives; cmd pool --> dcbs; swapChain & rp properly; (base stuff handled by Vulf)

        BuildSwapchain();

        BuildCommandPipeline(); // Recreate sc;sync prims; cmdpool; rp; (base stuff again + client - app stuff)
    }

    void VulfBase::QuitApp() {
        // Destroy sync primitives; cmd pool --> dcbs; swapChain & rp properly; (base stuff handled by Vulf)
        baseRenderPass.Destroy();
        baseSwapchain.Destroy();
    }
/******************************************************************************/
}
