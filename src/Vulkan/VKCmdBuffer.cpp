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
        throw std::runtime_error("Cannot create command buffers!");
    // else VK_LOG_SUCCESS("Command Buffers (3) succesfully Allocated!");
}

void VKCmdBuffer::DestroyBuffer(VkCommandBuffer& buffer)
{
    // vkFreeCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), buffer, nullptr);
	throw std::runtime_error("Unmiplemented method!");
}

void VKCmdBuffer::RecordBuffer(VkCommandBuffer& buffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;
    // Resetting command buffer before starting tor record
    vkResetCommandBuffer(buffer, 0);
	if (VK_CALL(vkBeginCommandBuffer(buffer, &beginInfo)))
		throw std::runtime_error("Cannot record onto CommandBuffers");
	// else VK_LOG("Recording to command buffer...");
}

void VKCmdBuffer::EndRecordingBuffer(VkCommandBuffer& buffer)
{
	if (VK_CALL(vkEndCommandBuffer(buffer))) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
