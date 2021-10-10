#include "VulfBase.h"

// STL
#include <sstream>

// Utilities
#include "utils/VulkanCheckResult.h"

// Profiling
#include <Tracy.hpp>

// TibyObj
//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tinyobj/tiny_obj_loader.h>

namespace Vulf {

// Public
/*******************************************************************************
 *                              Init Application                               *
 ******************************************************************************/

    VulfBase::~VulfBase() {
        _def_CommandPool.Destroy();
        VKLogicalDevice::GetDeviceManager()->Destroy();
        VKInstance::GetInstanceManager()->Destroy();
    }

    void VulfBase::Run() {
        InitResources();
        InitWindow();
        InitVulkan();
        InitImGui();

        RenderLoop();
    }

//-----------------------------------------------------------------------------//
// Render Loop
    void VulfBase::RenderLoop() {
        while (!m_Window->closed()) {

            // Update the window for input events
            m_Window->Update();

            auto tStart = std::chrono::high_resolution_clock::now();

            m_FrameCounter++;
            auto tEnd = std::chrono::high_resolution_clock::now();
            m_FrameTimer = std::chrono::duration<double, std::milli>(tEnd - tStart); // in Ms

            // Update the camera
            m_Camera.Update(*m_Window, m_FrameTimer.count());

            OnUpdate(m_FrameTimer.count());

            OnRender();

            DrawFrame();

            float fpsTimer = std::chrono::duration<double, std::milli>(tEnd - m_LastTimestamp).count();
            if (fpsTimer > 1000.0f) {
                VK_LOG("FPS : ", m_FrameCounter);
                m_FrameCounter = 0;
                m_LastTimestamp = tEnd;
            }
        }
        vkDeviceWaitIdle(VKDEVICE);
    }

// Protected
/*******************************************************************************
 *                  Client Side Customization(default behavior)                *
 ******************************************************************************/

    ////////////////////////////////////////////////////////////////////////////
    void VulfBase::BuildCommandPipeline() {
        // Create Buffer Resources such as Vertex, Index and Uniform buffers
        BuildBufferResource();

        // Build texture and image resources
        BuildTextureResources();

        // Create the swapchain
        BuildSwapchain();

        // Build the fixed pipeline stage
        BuildFixedPipeline();

        // Build renderpasses
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
        _def_CommandPool.Init();
    }

    void VulfBase::BuildTextureResources() {

    }

    void VulfBase::BuildSwapchain() {
        _def_Swapchain.Init(m_Window->getGLFWwindow());
    }

    void VulfBase::BuildFixedPipeline() {

    }

    void VulfBase::BuildRenderPass() {

    }

    void VulfBase::BuildGraphicsPipeline() {

    }

    void VulfBase::BuildFramebuffer() {

    }

    void VulfBase::BuildCommandBuffers() {

    }

    void VulfBase::BuildDescriptorSets() {

    }

//-----------------------------------------------------------------------------//

    void VulfBase::CleanUpPipeline() {

    }

 //-----------------------------------------------------------------------------//

    void VulfBase::DrawFrame() {
        vkWaitForFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);

        // Acquire the image to render onto
        VkResult result = vkAcquireNextImageKHR(VKDEVICE, _def_Swapchain.GetSwapchainKHR(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentImageIndex], VK_NULL_HANDLE, &m_NextImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Cannot acquire next image!");
        }

        if(m_ImagesInFlight[m_NextImageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(VKDEVICE, 1, &m_ImagesInFlight[m_NextImageIndex], VK_TRUE, UINT64_MAX);
        m_ImagesInFlight[m_NextImageIndex] = m_InFlightFences[m_CurrentImageIndex];

        UpdateBuffers();

        SubmitFrame();
    }

    void VulfBase::UpdateBuffers() {

    }

    void VulfBase::SubmitFrame() {

        VkResult result;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphore[] = { m_ImageAvailableSemaphores[m_CurrentImageIndex] };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = submissionCommandBuffers.size();
        submitInfo.pCommandBuffers = submissionCommandBuffers.data();
        VkSemaphore signalSemaphores[] = {m_RenderingFinishedSemaphores[m_CurrentImageIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(VKDEVICE, 1, &m_InFlightFences[m_CurrentImageIndex]);
        if(VK_CALL(vkQueueSubmit(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentImageIndex]))) {
            throw std::runtime_error("Cannot submit command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {_def_Swapchain.GetSwapchainKHR()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_NextImageIndex;

        result = vkQueuePresentKHR(VKLogicalDevice::GetDeviceManager()->GetPresentQueue(), &presentInfo);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->IsResized()) {
            m_Window->SetResizedFalse();
            RecreateSwapchain();
            return;
        }
        else if(result != VK_SUCCESS) {
            throw std::runtime_error("Cannot submit presentation queue!");
        }

        m_CurrentImageIndex = (m_CurrentImageIndex + 1 ) % MAX_FRAMES_IN_FLIGHT;
    }

//-----------------------------------------------------------------------------//

    void VulfBase::OnUpdate(double dt) {

    }

    void VulfBase::OnRender() {

    }

    void VulfBase::OnImGui() {

    }

// Private
/*******************************************************************************
 *                          Init Application Resources                         *
 ******************************************************************************/
    void VulfBase::InitResources() {

    }

    void VulfBase::InitWindow() {
        //
        m_Window = new Window(m_AppName.c_str(), width, height);
    }

    void VulfBase::InitVulkan() {
        // Create the Vulkan Instance
        // TODO: Use a proper signature for the application name (same is given for the window as well)
        VKInstance::GetInstanceManager()->Init(m_AppName.c_str(), m_Window->getGLFWwindow(), enableValidationLayers);

        // Create the Device
        VKLogicalDevice::GetDeviceManager()->Init();

        // Load the shaders
        LoadShaders();

        // Initialize the Command Pool and Build
        BuildCommandPool();

        // Build the command pipelines
        BuildCommandPipeline();

        // Create the synchironization primitives
        CreateSynchronizationPrimitives();
    }

    void VulfBase::InitImGui() {

    }

    void VulfBase::CreateSynchronizationPrimitives() {

        // Create the synchronization stuff
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderingFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(SWAP_IMAGES_COUNT, VK_NULL_HANDLE); // 3 swapchain images are used

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if( VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i])) ||
                VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &m_RenderingFinishedSemaphores[i])) ||
                VK_CALL(vkCreateFence(VKDEVICE, &fenceInfo, nullptr, &m_InFlightFences[i]))) {
                throw std::runtime_error("Cannot create Synchronization primitives!");
            }
        }
    }

    void VulfBase::RecreateSwapchain() {
        VK_LOG_SUCCESS("Recreating Swapchain..........");
        vkDeviceWaitIdle(VKDEVICE);

        CleanUpPipeline();

        BuildCommandPipeline();
        
        OnRender();
    }

/******************************************************************************/
}
