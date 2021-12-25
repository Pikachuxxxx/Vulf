#include "CmdPool.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void CmdPool::Init()
{
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = VKLogicalDevice::Get()->GetGPUManager().GetQueueFamilyIndices().graphicsFamily.value();

    if(VK_CALL(vkCreateCommandPool(VKDEVICE, &poolCI, nullptr, &m_CommandPool)))
        throw std::runtime_error("Cannot create command pool");
    else VK_LOG_SUCCESS("Command pool created succesfully!");

    VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
    name_info.objectHandle = (uint64_t) m_CommandPool;
    name_info.pObjectName = "Default Command Pool";
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetDeviceProcAddr(VKDEVICE, "vkSetDebugUtilsObjectNameEXT");
    vkSetDebugUtilsObjectNameEXT(VKDEVICE, &name_info);
}

VkCommandBuffer CmdPool::AllocateBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer cmdBuffer;
    if(VK_CALL(vkAllocateCommandBuffers(VKDEVICE, &allocInfo, &cmdBuffer)))
        throw std::runtime_error("Cannot create command buffer!");
    else VK_LOG_SUCCESS("Command Buffer (1) succesfully Allocated!");

    return cmdBuffer;
}

void CmdPool::Destroy()
{
    vkDestroyCommandPool(VKDEVICE, m_CommandPool, nullptr);
}

VkCommandBuffer CmdPool::BeginSingleTimeBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VKDEVICE, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CmdPool::EndSingleTimeBuffer(const VkCommandBuffer& buffer)
{
    vkEndCommandBuffer(buffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    vkQueueSubmit(VKLogicalDevice::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VKLogicalDevice::Get()->GetGraphicsQueue());

    vkFreeCommandBuffers(VKDEVICE, m_CommandPool, 1, &buffer);
}

void CmdPool::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeBuffer();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
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

   EndSingleTimeBuffer(commandBuffer);
}
