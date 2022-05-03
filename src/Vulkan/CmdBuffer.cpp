#include "CmdBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

#include <string>

void CmdBuffer::AllocateBuffers(const VkCommandPool& pool, uint32_t count)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    m_CommandBuffers.resize(count);
    if(VK_CALL(vkAllocateCommandBuffers(VKDEVICE, &allocInfo, m_CommandBuffers.data())))
        throw std::runtime_error("Cannot create command buffers!");
    else VK_LOG("Command Buffers succesfully Allocated!");

    for (size_t i = 0; i < m_CommandBuffers.size(); i++) {

        VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        name_info.objectHandle = (uint64_t) m_CommandBuffers[i];
        name_info.pObjectName = (std::string("Command Buffer : ") + std::to_string(i)).c_str();
        auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetDeviceProcAddr(VKDEVICE, "vkSetDebugUtilsObjectNameEXT");
        vkSetDebugUtilsObjectNameEXT(VKDEVICE, &name_info);
    }
}

void CmdBuffer::Destroy(const VkCommandPool& pool)
{
    vkFreeCommandBuffers(VKDEVICE, pool, m_CommandBuffers.size(), m_CommandBuffers.data());
}

void CmdBuffer::RecordBuffer(VkCommandBuffer& buffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;
    // Resetting command buffer before starting tor record
    vkResetCommandBuffer(buffer, 0);
	if (VK_CALL(vkBeginCommandBuffer(buffer, &beginInfo)))
		throw std::runtime_error("Cannot record onto CommandBuffers");
	//else VK_LOG("Recording to command buffer...");
}

void CmdBuffer::EndRecordingBuffer(VkCommandBuffer& buffer)
{
	if (VK_CALL(vkEndCommandBuffer(buffer))) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
