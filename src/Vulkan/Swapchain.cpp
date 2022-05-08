#include "Swapchain.h"

#include "Instance.h"
#include "Device.h"

#include "../utils/VulkanCheckResult.h"

#include <cstdint>
#include <algorithm>
#include <sstream>

void Swapchain::Init(GLFWwindow* window)
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
    swcCI.surface = Instance::Get()->get_surface();
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
    QueueFamilyIndices indices = Device::Get()->get_physical_device().get_queue_family_indices();
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

void Swapchain::Destroy()
{
    // Destroy the swpachain image views first
    for (const auto & imageView : m_SwapchainImageViews)
        vkDestroyImageView(VKDEVICE, imageView, nullptr);
    vkDestroySwapchainKHR(VKDEVICE, m_SwapchainKHR, nullptr);
}

void Swapchain::querySwapchainProperties()
{
    // Get the surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device::Get()->get_gpu(), Instance::Get()->get_surface(), &m_SurfaceProperties.capabilities);
    // Get the surface formats supported
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device::Get()->get_gpu(), Instance::Get()->get_surface(), &formatsCount, nullptr);
    m_SurfaceProperties.formats.resize(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device::Get()->get_gpu(), Instance::Get()->get_surface(), &formatsCount, m_SurfaceProperties.formats.data());
    // Get the surface available presentModes
    uint32_t presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device::Get()->get_gpu(), Instance::Get()->get_surface(), &presentModesCount, nullptr);
    m_SurfaceProperties.presentModes.resize(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device::Get()->get_gpu(), Instance::Get()->get_surface(), &presentModesCount, m_SurfaceProperties.presentModes.data());
}

VkExtent2D Swapchain::getSwapchainExtent()
{
    // choose the bnest swpachain resolution to present the image onto
    if(m_SurfaceProperties.capabilities.currentExtent.width != UINT32_MAX)
        return m_SurfaceProperties.capabilities.currentExtent;
    else
    {
        auto& capabilities = m_SurfaceProperties.capabilities;
        int width = 3840, height = 2160;
        // glfwGetFramebufferSize(m_Window, &width, &height);

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

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormats()
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

VkPresentModeKHR Swapchain::choosePresentMode()
{
    // Choose the right kind of image presentation mode for the  swapchain images
    for (const auto& presentMode : m_SurfaceProperties.presentModes)
    {
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;

    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Swapchain::RetrieveSwapchainImages()
{
    vkGetSwapchainImagesKHR(VKDEVICE, m_SwapchainKHR, &m_SwapchainImageCount, nullptr);
    m_SwapchainImages.resize(m_SwapchainImageCount);
    if(VK_CALL(vkGetSwapchainImagesKHR(VKDEVICE, m_SwapchainKHR, &m_SwapchainImageCount, m_SwapchainImages.data())))
        throw std::runtime_error("Cannot retrieve swapchain images!");
    else
        VK_LOG("Swapchain images(", m_SwapchainImageCount, ") have been retrieved succesfully!");
}

void Swapchain::CreateSwapchainImageViews()
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


void Swapchain::create_sets()
{

    VkDescriptorPoolSize poolSize;
    poolSize.type = (VkDescriptorType) VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(1);
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(3);

    if (VK_CALL(vkCreateDescriptorPool(VKDEVICE, &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create the descriptor pool!");
    else VK_LOG_SUCCESS("successuflly create descriptor pool!");

    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;
    vkCreateDescriptorSetLayout(VKDEVICE, &info, nullptr, &g_DescriptorSetLayout);

    // if(allocated < 2)
    // Create Descriptor Set:
        std::vector<VkDescriptorSetLayout> layouts(3, g_DescriptorSetLayout);
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_DescriptorPool;
        alloc_info.descriptorSetCount = layouts.size();
        alloc_info.pSetLayouts = layouts.data();

        m_DescriptorSets.clear();
        m_DescriptorSets.resize(3);

        if (VK_CALL(vkAllocateDescriptorSets(VKDEVICE, &alloc_info, m_DescriptorSets.data())))
            throw std::runtime_error("failed to create texture descriptor set for texture!");
    }
}

void Swapchain::update_sets()
{
    // Update the Descriptor Sets

    for (size_t i = 0; i < 3; i++)
    {
        VkDescriptorImageInfo desc_image[1] = {};
        VkWriteDescriptorSet write_desc[1] = {};

        desc_image[0].sampler = m_ImageSampler;
        desc_image[0].imageView = m_SwapchainImageViews[i];
        desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = m_DescriptorSets[i];
        write_desc[0].dstBinding = 0;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;
        
        vkUpdateDescriptorSets(VKDEVICE, 1, write_desc, 0, nullptr);
    }
}
