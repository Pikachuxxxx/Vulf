#include "CmdBuffer.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

#include <string>

void CmdBuffer::Init(const VkCommandPool& pool, bool isPrimary /*= true*/)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool           = pool;
    allocInfo.level                 = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount    = 1;

    if(VK_CALL(vkAllocateCommandBuffers(VKDEVICE, &allocInfo, &m_CommandBuffer)))
        throw std::runtime_error("Cannot create command buffer!");
    else VK_LOG_SUCCESS("Command Buffer succesfully Allocated!");
}

void CmdBuffer::Destroy(const VkCommandPool& pool)
{
    vkFreeCommandBuffers(VKDEVICE, pool, 1, &m_CommandBuffer);
}

void CmdBuffer::reset()
{
    vkResetCommandBuffer(m_CommandBuffer, 0);
}

void CmdBuffer::begin_recording()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // set the glags externally if needed, for now we reset/rerecord it before we submit
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

    // Resetting command buffer before starting to record
    vkResetCommandBuffer(m_CommandBuffer, 0);

	if (VK_CALL(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo)))
		throw std::runtime_error("Cannot record onto CommandBuffer!");
	//else VK_LOG("Recording to command buffer...");
}

void CmdBuffer::end_recording()
{
	if (VK_CALL(vkEndCommandBuffer(m_CommandBuffer))) {
		throw std::runtime_error("failed to record CommandBuffer!");
	}
}
