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
        
    // Create a default command pool to allocate command buffers on fly
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = m_GPUManager.GetQueueFamilyIndices().graphicsFamily.value();
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CALL(vkCreateCommandPool(m_Device, &cmdPoolInfo, nullptr, &m_InstantaneousCmdPool));
}

void VKLogicalDevice::Destroy()
{
    vkDestroyCommandPool(m_Device, m_InstantaneousCmdPool, nullptr);
    vkDestroyDevice(m_Device, nullptr);
}

VkCommandBuffer VKLogicalDevice::createCmdBuffer(VkCommandBufferLevel level, bool begin /*= false*/) {
    VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = m_InstantaneousCmdPool;
    cmdBufAllocateInfo.level = level;
    cmdBufAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    VK_CALL(vkAllocateCommandBuffers(m_Device, &cmdBufAllocateInfo, &cmdBuffer));

    // If requested, also start recording for the new command buffer
    if (begin) {
        VkCommandBufferBeginInfo commandBufferBI{};
        commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CALL(vkBeginCommandBuffer(cmdBuffer, &commandBufferBI));
    }

    return cmdBuffer;
}

void VKLogicalDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free /*= true*/) {

    VK_CALL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    VK_CALL(vkCreateFence(m_Device, &fenceInfo, nullptr, &fence));

    // Submit to the queue
    VK_CALL(vkQueueSubmit(queue, 1, &submitInfo, fence));
    // Wait for the fence to signal that command buffer has finished executing
    VK_CALL(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(m_Device, fence, nullptr);

    if (free) {
        vkFreeCommandBuffers(m_Device, m_InstantaneousCmdPool, 1, &commandBuffer);
    }
}

VkResult VKLogicalDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data /*= nullptr*/) {

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CALL(vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkGetBufferMemoryRequirements(m_Device, *buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = GetGPUManager().FindMemoryTypeIndex(memReqs.memoryTypeBits, memoryPropertyFlags);
    VK_CALL(vkAllocateMemory(m_Device, &memAlloc, nullptr, memory));

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr) {
        void* mapped;
        VK_CALL(vkMapMemory(m_Device, *memory, 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{};
            mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = *memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
        }
        vkUnmapMemory(m_Device, *memory);
    }

    // Attach the memory to the buffer object
    VK_CALL(vkBindBufferMemory(m_Device, *buffer, *memory, 0));

    return VK_SUCCESS;
}

void VKLogicalDevice::CreateQueues()
{
    auto indices = m_GPUManager.GetQueueFamilyIndices();
    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &presentQueue);
}
