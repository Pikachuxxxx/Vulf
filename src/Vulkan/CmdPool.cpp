#include "CmdPool.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

void CmdPool::Init(uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = queueFamilyIndex;

    if(VK_CALL(vkCreateCommandPool(VKDEVICE, &poolCI, nullptr, &m_CommandPool)))
        throw std::runtime_error("Cannot create command pool");
    else VK_LOG_SUCCESS("Command pool created succesfully!");

    // TODO: Use debug markers instead (will be refactored when all the vulkan functions are loaded manually, this makes extension functions loading easier)
    VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    name_info.objectType                    = VK_OBJECT_TYPE_COMMAND_POOL;
    name_info.objectHandle                  = (uint64_t) m_CommandPool;
    name_info.pObjectName                   = "Default Command Pool";

    auto vkSetDebugUtilsObjectNameEXT       = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetDeviceProcAddr(VKDEVICE, "vkSetDebugUtilsObjectNameEXT");
    vkSetDebugUtilsObjectNameEXT(VKDEVICE, &name_info);
}

void CmdPool::Destroy()
{
    vkDestroyCommandPool(VKDEVICE, m_CommandPool, nullptr);
}

std::vector<CmdBuffer> CmdPool::allocate_buffers(uint32_t count, bool isPrimary /*=true*/)
{
    std::vector<CmdBuffer> buffers(count);

    for (size_t i = 0; i < count; i++)
        buffers[i].Init(m_CommandPool, isPrimary);

    return buffers;
}
