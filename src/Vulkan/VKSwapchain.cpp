#include "VKSwapchain.h"

#include "VKInstance.h"
#include "VKDevice.h"

#include "../utils/VulkanCheckResult.h"

#include <cstdint>
#include <algorithm>
#include <sstream>

void VKSwapchain::Init(GLFWwindow* window)
{
    m_Window = window;
    // Query the swapchain surface properties
    querySwapchainProperties();
    // Get the coloe space iamge format for the swapchain
    m_SurfaceFormat = chooseSurfaceFormats();
    // Get the swpahcain extents
    m_SwapchainExtent = getSwapchainExtent();
    // Choose the swapchain presentation mode
    m_PresentMode = choosePresentMode();

    // Now craete the Swapchain
    VkSwapchainCreateInfoKHR swcCI{};
    swcCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swcCI.surface = VKInstance::GetInstanceManager()->GetSurface();
    m_SwapchainImageCount = m_SurfaceProperties.capabilities.minImageCount + 1; // For triple buffering
    // Bound checking the swapchain imagec count only for triple buffer aka 2 frames in flight
    if (m_SurfaceProperties.capabilities.maxImageCount > 0 && m_SwapchainImageCount > m_SurfaceProperties.capabilities.maxImageCount)
        m_SwapchainImageCount = m_SurfaceProperties.capabilities.maxImageCount;
    VK_LOG("Swapchain image count : ", m_SwapchainImageCount);
    swcCI.minImageCount = m_SwapchainImageCount;
    swcCI.imageFormat = m_SurfaceFormat.format;
    swcCI.imageColorSpace = m_SurfaceFormat.colorSpace;
    swcCI.imageExtent = m_SwapchainExtent;
    swcCI.imageArrayLayers = 1;
    swcCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        swcCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swcCI.queueFamilyIndexCount = 2;
        swcCI.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swcCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swcCI.queueFamilyIndexCount = 0; // Optional
        swcCI.pQueueFamilyIndices = nullptr; // Optional
    }

    swcCI.preTransform = m_SurfaceProperties.capabilities.currentTransform;
    swcCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swcCI.clipped = VK_TRUE;
    swcCI.oldSwapchain = VK_NULL_HANDLE;

    // Now actually create the swapchainManager
    if(VK_CALL(vkCreateSwapchainKHR(VKDEVICE, &swcCI, nullptr, &m_SwapchainKHR)))
        throw std::runtime_error("Cannot create the Swapchain!");
    else
        VK_LOG_SUCCESS("Swapchain created successfully!");

    // Retrieve the swapchain imaes for rendering usage
    RetrieveSwapchainImages();
    // Create the image view for the swapchain images
    CreateSwapchainImageViews();
}

void VKSwapchain::Destroy()
{
    // Destroy the swpachain image views first
    for (const auto & imageView : m_SwapchainImageViews)
        vkDestroyImageView(VKDEVICE, imageView, nullptr);
    vkDestroySwapchainKHR(VKDEVICE, m_SwapchainKHR, nullptr);
}

void VKSwapchain::querySwapchainProperties()
{
    // Get the surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), VKInstance::GetInstanceManager()->GetSurface(), &m_SurfaceProperties.capabilities);
    // Get the surface formats supported
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), VKInstance::GetInstanceManager()->GetSurface(), &formatsCount, nullptr);
    m_SurfaceProperties.formats.resize(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), VKInstance::GetInstanceManager()->GetSurface(), &formatsCount, m_SurfaceProperties.formats.data());
    // Get the surface available presentModes
    uint32_t presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), VKInstance::GetInstanceManager()->GetSurface(), &presentModesCount, nullptr);
    m_SurfaceProperties.presentModes.resize(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), VKInstance::GetInstanceManager()->GetSurface(), &presentModesCount, m_SurfaceProperties.presentModes.data());
}

VkExtent2D VKSwapchain::getSwapchainExtent()
{
    // choose the bnest swpachain resolution to present the image onto
    if(m_SurfaceProperties.capabilities.currentExtent.width != UINT32_MAX)
        return m_SurfaceProperties.capabilities.currentExtent;
    else
    {
        auto& capabilities = m_SurfaceProperties.capabilities;
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);

        VkExtent2D actualExtent =
        {
           static_cast<uint32_t>(width),
           static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkSurfaceFormatKHR VKSwapchain::chooseSurfaceFormats()
{
    // Get the right color space
    // Get the right image format for the swapchain iamges to present mode
    for (const auto& format : m_SurfaceProperties.formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return m_SurfaceProperties.formats[0];
}

VkPresentModeKHR VKSwapchain::choosePresentMode()
{
    // Choose the right kind of image presentation mode for the  swapchain images
    for (const auto& presentMode : m_SurfaceProperties.presentModes)
    {
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;

    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void VKSwapchain::RetrieveSwapchainImages()
{
    vkGetSwapchainImagesKHR(VKDEVICE, m_SwapchainKHR, &m_SwapchainImageCount, nullptr);
    m_SwapchainImages.resize(m_SwapchainImageCount);
    if(VK_CALL(vkGetSwapchainImagesKHR(VKDEVICE, m_SwapchainKHR, &m_SwapchainImageCount, m_SwapchainImages.data())))
        throw std::runtime_error("Cannot retrieve swapchain images!");
    else
        VK_LOG("Swapchain images(", m_SwapchainImageCount, ") have been retrieved succesfully!");
}

void VKSwapchain::CreateSwapchainImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImageCount);
    for (size_t i = 0; i < m_SwapchainImageCount; i++)
    {
        // Create the image view with the required properties
        VkImageViewCreateInfo imvCI{};
        imvCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imvCI.image = m_SwapchainImages[i];
        imvCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imvCI.format = m_SurfaceFormat.format;
        imvCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imvCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imvCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imvCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imvCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imvCI.subresourceRange.baseMipLevel = 0;
        imvCI.subresourceRange.levelCount = 1;
        imvCI.subresourceRange.baseArrayLayer = 0;
        imvCI.subresourceRange.layerCount = 1;
        if(VK_CALL(vkCreateImageView(VKDEVICE, &imvCI, nullptr, &m_SwapchainImageViews[i])))
            throw std::runtime_error("Cannot create image view!");
        else VK_LOG("Image view(id=", i ,") succesfully created!");
    }
}
