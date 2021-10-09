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

        /*
        m_LastTime = glfwGetTime();
        m_SecondTimer = glfwGetTime();
        while (!m_Window->closed()) {
            // Update the window for input events
            m_Window->Update();

            // Calculate the delta time taken to render the fame
            double currentTIme = glfwGetTime();
            m_DeltaTime = currentTIme - m_LastTime;

            // OnUpdate
            OnUpdate(m_DeltaTime);

            // Render the scene
            OnRender();
            m_NbFrames++;

            m_LastTime = currentTIme;
            if (glfwGetTime() - m_SecondTimer > 1.0) {
                m_SecondTimer += 1.0;
                VK_LOG("FPS : ", m_NbFrames);
                m_NbFrames = 0;

            }
                    
        }
        */

        while (!m_Window->closed()) {                                       
            
            m_Window->Update();

            auto tStart = std::chrono::high_resolution_clock::now();

            OnRender();

            m_FrameCounter++;
            auto tEnd = std::chrono::high_resolution_clock::now();
            m_FrameTimer = std::chrono::duration<double, std::milli>(tEnd - tStart); // in Ms

            OnUpdate(m_FrameTimer.count());

            float fpsTimer = std::chrono::duration<double, std::milli>(tEnd - m_LastTimestamp).count();
            if (fpsTimer > 1000.0f) {
                VK_LOG("FPS : ", m_FrameCounter);
                m_FrameCounter = 0;
                m_LastTimestamp = tEnd;
            }
        }
    }

// Protected
/*******************************************************************************
 *                  Client Side Customization(default behavior)                *
 ******************************************************************************/

    void VulfBase::BuildCommandPool() {

    }

    void VulfBase::BuildSwapchain() {

    }

    void VulfBase::BuildPipeline() {

    }

    void VulfBase::BuildGraphicsPipeline() {

    }

    void VulfBase::BuildRenderPass() {

    }

    void VulfBase::BuildFramebuffer() {

    }

    void VulfBase::BuildCommandBuffers() {

    }

    void VulfBase::BuildDescriptorSets() {

    }

//-----------------------------------------------------------------------------//

    void VulfBase::CreateTextureResources() {

    }

    void VulfBase::UpdateBuffers() {

    }
 //-----------------------------------------------------------------------------//

    void VulfBase::Draw() {
        
    }

    void VulfBase::RenderFrame() {

    }

    void VulfBase::PresentFrame() {

    }

//-----------------------------------------------------------------------------//

    void VulfBase::OnUpdate(double dt) {
        // Update the camera
        m_Camera.Update(*m_Window, dt);

        //VK_LOG("deltaTime : ", std::setprecision(15) , dt);

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
        m_Window = new Window("Vulf Renderer", width, height);
    }

    void VulfBase::InitVulkan() {
        // Create the Vulkan Instance
        // TODO: Use a proper signature for the application name (same is given for the window as well)
        VKInstance::GetInstanceManager()->Init("Vulf", m_Window->getGLFWwindow(), enableValidationLayers);

        // Create the Device
        VKLogicalDevice::GetDeviceManager()->Init();

        // Initialize the Command Pool
        _def_CommandPool.Init();

        // Build the command pipeline

    }

    void VulfBase::InitImGui() {

    }

    void VulfBase::Present() {

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

/******************************************************************************/
}
