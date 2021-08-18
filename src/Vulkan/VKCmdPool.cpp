#include "VKCmdPool.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKCmdPool::Init()
{
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetQueueFamilyIndices().graphicsFamily.value();

    if(VK_CALL(vkCreateCommandPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &poolCI, nullptr, &m_CommandPool)))
        throw std::runtime_error("Cannot create command pool");
    else VK_LOG_SUCCESS("Command pool created succesfully!");
}

VkCommandBuffer VKCmdPool::AllocateBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer cmdBuffer;
    if(VK_CALL(vkAllocateCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, &cmdBuffer)))
        throw std::runtime_error("Cannot create command buffer!");
    else VK_LOG_SUCCESS("Command Buffer (1) succesfully Allocated!");

    return cmdBuffer;
}

void VKCmdPool::Destroy()
{
    vkDestroyCommandPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_CommandPool, nullptr);
}
