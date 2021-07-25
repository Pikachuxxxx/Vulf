#include "VKCmdPool.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKCmdPool::Init()
{
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.queueFamilyIndex = VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetQueueFamilyIndices().graphicsFamily.value();

    if(VK_CALL(vkCreateCommandPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &poolCI, nullptr, &m_CommandPool)))
        throw std::runtime_error("Cannot create command pool");
    else VK_LOG_SUCCESS("Command pool created succesfully!");
}

void VKCmdPool::Destroy()
{
    vkDestroyCommandPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_CommandPool, nullptr);
}
