#include "Device.h"

#include "Instance.h"
#include <iostream>
#include <set>

#include "../utils/prettytable.h"
#include "../utils/VulkanCheckResult.h"

/******************************************************************************/
/*                          PhysicalDevice class                              */
/******************************************************************************/

void PhysicalDevice::Init()
{
    uint32_t numGPUs = 0;
    vkEnumeratePhysicalDevices(Instance::Get()->get_handle(), &numGPUs, nullptr);
    std::vector<VkPhysicalDevice> availableGPUs(numGPUs);
    vkEnumeratePhysicalDevices(Instance::Get()->get_handle(), &numGPUs, availableGPUs.data());
    for (size_t i = 0; i < numGPUs; ++i)
    {
        if(is_device_suitable(availableGPUs[i]))
        {
            m_GPU = availableGPUs[i];
            // Print the GPU details
            VK_LOG("Vulkan API Version : ", VK_VERSION_MAJOR(m_DeviceProperties.apiVersion), ".", VK_VERSION_MINOR(m_DeviceProperties.apiVersion), ".",  VK_VERSION_PATCH(m_DeviceProperties.apiVersion));
            // VK_LOG("Device Type        : ", std::string(getPhysicalDeviceTypeString(m_DeviceProperties.deviceType)));
            VK_LOG("GPU Name           : ", std::string(m_DeviceProperties.deviceName));
            VK_LOG("Vendor ID          : ", std::to_string(m_DeviceProperties.vendorID));
            VK_LOG("Driver Version     : ", VK_VERSION_MAJOR(m_DeviceProperties.driverVersion), ".", VK_VERSION_MINOR(m_DeviceProperties.driverVersion), ".", VK_VERSION_PATCH(m_DeviceProperties.driverVersion));
            break;
        }
    }

    // TODO: Store the device properties and features
}

uint32_t PhysicalDevice::find_memory_type_index(uint32_t typeBitFieldFilter, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_GPU, &memProperties);
    for (size_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if(typeBitFieldFilter & (1 << i) && memProperties.memoryTypes[i].propertyFlags & flags)
        return i;
    }
    return 0;
}

VkFormat PhysicalDevice::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

VkFormat PhysicalDevice::find_depth_format()
{
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool PhysicalDevice::is_device_suitable(VkPhysicalDevice& gpu)
{
    vkGetPhysicalDeviceProperties(gpu, &m_DeviceProperties);
    vkGetPhysicalDeviceFeatures(gpu, &m_DeviceFeatures);
    // For now only finding the supported queues will suffice
    find_queue_family_indices(gpu);
    return m_QueueFamilyIndices.isComplete();
}

void PhysicalDevice::find_queue_family_indices(VkPhysicalDevice gpu)
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

            // Check for presentation support (usually we see that both graphics and queue family is same)
            VkBool32 presentationSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, Instance::Get()->get_surface(), &presentationSupported);

            if(presentationSupported)
                m_QueueFamilyIndices.presentFamily = i;

            // TODO: Check for Compute queue

            if(m_QueueFamilyIndices.isComplete())
                break;

        i++;
    }
}

/******************************************************************************/
/*                              Device class                                  */
/******************************************************************************/

//------------------------------------------------------------------------------
Device* Device::s_Instance = nullptr;
//------------------------------------------------------------------------------

void Device::Init()
{
    // Select the Physical device and it's supported Queue Families indices
    m_GPUManager.Init();

    auto indices = m_GPUManager.get_queue_family_indices();
    // TODO: add compute and transfer queue family indices
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

    VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeatures{};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
    meshShaderFeatures.pNext = nullptr;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.meshShader = VK_TRUE;

    // Get the list of all the extensions supported by the physical device
    uint32_t extensionsCount = 0;
    if (VK_CALL(vkEnumerateDeviceExtensionProperties(get_gpu(), nullptr, &extensionsCount, nullptr)))
        throw std::runtime_error("cannot query device extensions!");
    std::vector<VkExtensionProperties> availableDeviceExtensions;
    availableDeviceExtensions.resize(extensionsCount);
    vkEnumerateDeviceExtensionProperties(m_GPUManager.get_gpu(), nullptr, &extensionsCount, availableDeviceExtensions.data());

    // check if the user requested extensions are supported or not
    // Now print the layers only for debug purposes
#ifndef NDEBUG
    PrettyTable<std::string, std::string> vt({"Requested Extensions", "isAvailable"});
    for (uint32_t i = 0; i < extensionsCount; i++) {
        for (uint32_t j = 0; j < g_DeviceExtensions.size(); j++) {
            if (strcmp(availableDeviceExtensions[i].extensionName, g_DeviceExtensions[j]) == 0) {
                vt.addRow(g_DeviceExtensions[j], "Yes");
            }
            //else {
            //    vt.addRow(availableDeviceExtensions[i].extensionName, "", "");
            //}
        }
    }
    vt.print(std::cout);
#endif

    // Create the Logical Device
    VkDeviceCreateInfo deviceCI{};
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pNext = &meshShaderFeatures;
    deviceCI.queueCreateInfoCount = queueCreateInfos.size();
    deviceCI.pQueueCreateInfos = queueCreateInfos.data();
    deviceCI.enabledExtensionCount = g_DeviceExtensions.size();
    deviceCI.ppEnabledExtensionNames = g_DeviceExtensions.data();
    deviceCI.pEnabledFeatures = &deviceFeatures;

    if(VK_CALL(vkCreateDevice(m_GPUManager.get_gpu(), &deviceCI, nullptr, &m_Device)))
        throw std::runtime_error("Cannot create Logical Device!");
    else
        VK_LOG_SUCCESS("Logical Device succesfully created!");

    //--------------------------------------------------------------------------
    // Create the queue handles
    create_queues();
    //--------------------------------------------------------------------------

    // Create a default instantaneous command pool to allocate command buffers on fly(Graphics Only)
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = get_graphics_queue_index();
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CALL(vkCreateCommandPool(m_Device, &cmdPoolInfo, nullptr, &m_InstantaneousCmdPool));
}

void Device::Destroy()
{
    vkDestroyCommandPool(m_Device, m_InstantaneousCmdPool, nullptr);

    vkDestroyDevice(m_Device, nullptr);
}

VkCommandBuffer Device::begin_single_time_cmd_buffer(VkCommandBufferLevel level, bool begin /*= true*/) {
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

void Device::end_single_time_cmd_buffer(VkCommandBuffer cmdBuf)
{
    if (VK_CALL(vkEndCommandBuffer(cmdBuf))) {
		throw std::runtime_error("failed to record single time command buffer!");
	}
}

void Device::flush_cmd_buffer(VkCommandBuffer cmdBuf, VkQueue queue, bool free /*= true*/) {

    VK_CALL(vkEndCommandBuffer(cmdBuf));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;

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
        vkFreeCommandBuffers(m_Device, m_InstantaneousCmdPool, 1, &cmdBuf);
    }
}

VkResult Device::create_buffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory, void* data /*= nullptr*/) {

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CALL(vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, &buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = get_physical_device().find_memory_type_index(memReqs.memoryTypeBits, memoryPropertyFlags);
    VK_CALL(vkAllocateMemory(m_Device, &memAlloc, nullptr, &memory));

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr) {
        void* mapped;
        VK_CALL(vkMapMemory(m_Device, memory, 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{};
            mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
        }
        vkUnmapMemory(m_Device, memory);
    }

    // Attach the memory to the buffer object
    VK_CALL(vkBindBufferMemory(m_Device, buffer, memory, 0));

    return VK_SUCCESS;
}

VkResult Device::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data /*= nullptr*/)
{
    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CALL(vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, &buffer->get_handle()));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkGetBufferMemoryRequirements(m_Device, buffer->get_handle(), &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = get_physical_device().find_memory_type_index(memReqs.memoryTypeBits, memoryPropertyFlags);
    VK_CALL(vkAllocateMemory(m_Device, &memAlloc, nullptr, &buffer->get_memory()));

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr) {
        void* mapped;
        VK_CALL(vkMapMemory(m_Device, buffer->get_memory(), 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{};
            mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = buffer->get_memory();
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
        }
        vkUnmapMemory(m_Device, buffer->get_memory());
    }

    // Attach the memory to the buffer object
    VK_CALL(vkBindBufferMemory(m_Device, buffer->get_handle(), buffer->get_memory(), 0));

    return VK_SUCCESS;
}

void Device::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = begin_single_time_cmd_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    // Actual copy command
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    flush_cmd_buffer(commandBuffer, m_GraphicsQueue, true);
}

void Device::create_queues()
{
    auto indices = m_GPUManager.get_queue_family_indices();
    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
}
