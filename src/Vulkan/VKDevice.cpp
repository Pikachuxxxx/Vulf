#include "VKDevice.h"

#include "Instance.h"
#include <iostream>
#include <set>

#include "../utils/prettytable.h"
#include "../utils/VulkanCheckResult.h"

void VKPhysicalDevice::Init()
{
    uint32_t numGPUs = 0;
    vkEnumeratePhysicalDevices(Instance::GetInstanceManager()->GetInstance(), &numGPUs, nullptr);
    std::vector<VkPhysicalDevice> availableGPUs(numGPUs);
    vkEnumeratePhysicalDevices(Instance::GetInstanceManager()->GetInstance(), &numGPUs, availableGPUs.data());
    for (size_t i = 0; i < numGPUs; ++i)
    {
        if(isDeviceSuitable(availableGPUs[i]))
        {
            m_GPU = availableGPUs[i];
            break;
        }
    }
}

bool VKPhysicalDevice::isDeviceSuitable(VkPhysicalDevice& gpu)
{
    vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
    vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
    // For now only finding the supportted uqueues will suffice
    findQueueFamilyIndices(gpu);
    return m_QueueFamilyIndices.isComplete();
}

void VKPhysicalDevice::findQueueFamilyIndices(VkPhysicalDevice gpu)
{
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, queueFamilyProperties.data());

    uint32_t i = 0;
    for (const auto& queue : queueFamilyProperties)
    {
        if(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_QueueFamilyIndices.graphicsFamily = i;

            // Check for presentation support
            VkBool32 presentationSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, Instance::GetInstanceManager()->GetSurface(), &presentationSupported);

            if(presentationSupported)
                m_QueueFamilyIndices.presentFamily = i;

            if(m_QueueFamilyIndices.isComplete())
                break;

        i++;
    }
}

uint32_t VKPhysicalDevice::FindMemoryTypeIndex(uint32_t typeBitFieldFilter, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_GPU, &memProperties);
    for (size_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if(typeBitFieldFilter & (1 << i) && memProperties.memoryTypes[i].propertyFlags & flags)
        return i;
    }
    return 0;
}

VkFormat VKPhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_GPU, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

VkFormat VKPhysicalDevice::FindDepthFormat()
{
    return VKLogicalDevice::GetDeviceManager()->GetGPUManager().FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VKLogicalDevice* VKLogicalDevice::s_Instance;

void VKLogicalDevice::Init()
{
    // Select the Physical device and it's supported Queue Families
    m_GPUManager.Init();

    auto indices = m_GPUManager.GetQueueFamilyIndices();
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Create the Logica Device
    VkDeviceCreateInfo deviceCI{};
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.queueCreateInfoCount = queueCreateInfos.size();
    deviceCI.pQueueCreateInfos = queueCreateInfos.data();
    deviceCI.enabledExtensionCount = deviceExtensions.size();
    deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCI.pEnabledFeatures = &deviceFeatures;

    if(VK_CALL(vkCreateDevice(m_GPUManager.GetGPU(), &deviceCI, nullptr, &m_Device)))
        throw std::runtime_error("Cannot create Logical Device!");
    else
        VK_LOG_SUCCESS("Logical Device succesfully created!");

    CreateQueues();
}

void VKLogicalDevice::Destroy()
{
    vkDestroyDevice(m_Device, nullptr);
}

void VKLogicalDevice::CreateQueues()
{
    auto indices = m_GPUManager.GetQueueFamilyIndices();
    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &presentQueue);
}
