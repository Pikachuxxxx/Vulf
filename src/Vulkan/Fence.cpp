#include "Fence.h"

#include "utils/VulkanCheckResult.h"
#include "Device.h"

void Fence::Init(bool signalled /*= true*/)
{
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = signalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(VK_CALL(vkCreateFence(VKDEVICE, &fenceCI, nullptr, &m_Fence)))
        throw std::runtime_error("Cannot create Fence!");
}

void Fence::Destroy()
{
    // Destroy the Fence
    vkDestroyFence(VKDEVICE, m_Fence, nullptr);
}
