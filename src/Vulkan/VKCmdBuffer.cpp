#include "VKCmdBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKCmdBuffer::AllocateBuffers(const VkCommandPool& pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 3;
    m_CommandBuffers.resize(3);
    if(VK_CALL(vkAllocateCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, m_CommandBuffers.data())))
        throw std::runtime_error("Cannot creae commandbuffers!");
    else VK_LOG_SUCCESS("CommandBuffers succesfully Allocated!");
}

void VKCmdBuffer::RecordBuffers()
{
    int i = 0;
    for(const auto& commandBuffer : m_CommandBuffers)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        i++;
        if(VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo)))
            throw std::runtime_error("Cannot record onto CommandBuffers");
        else VK_LOG("Recoring to command buffer(", i, ")...");
    }
}

void VKCmdBuffer::EndRecordingBuffers()
{
    for(const auto& commandBuffer : m_CommandBuffers)
    {
        if (VK_CALL(vkEndCommandBuffer(commandBuffer))) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}
