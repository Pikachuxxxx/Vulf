#include "VulfBase.h"

// STL
#include <sstream>

// Utilities
#include "utils/VulkanCheckResult.h"

// Profiling
#include <Tracy.hpp>

namespace Vulf {

// Public
/*******************************************************************************
 *                              Init Application                               *
 ******************************************************************************/
    VulfBase::VulfBase() {
        InitResources();
        InitWindow();
        InitVulkan();
        InitImGui();
    }

    VulfBase::~VulfBase() {
        _def_CommandPool.Destroy();
        VKLogicalDevice::GetDeviceManager()->Destroy();
        VKInstance::GetInstanceManager()->Destroy();
    }

    void VulfBase::Run() {

    }

// Protected
/*******************************************************************************
 *                  Client Side Customization(default behaviour)               *
 ******************************************************************************/
    void VulfBase::LoadShaders() {

    }

    void VulfBase::BuildCommandPipeline() {

    }

    void VulfBase::BuildRenderPass() {

    }

    void VulfBase::BuildFramebuffer() {

    }

    void VulfBase::Draw() {
        
    }

    void VulfBase::OnUpdate(double dt) {

    }

    void VulfBase::OnRender() {

    }

    void VulfBase::OnImGui() {

    }

// Private
/*******************************************************************************
 *                          Init Application Resouces                          *
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
    }

    void VulfBase::InitImGui() {

    }

    void VulfBase::Present()
    {

    }

    void VulfBase::CreateSynchronizationPrimitives()
    {
        // Create the synchronization stuff
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderingFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(3, VK_NULL_HANDLE); // 3 swapchain images are used

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
