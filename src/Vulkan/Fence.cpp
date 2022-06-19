#include "Fence.h"

#include "VulkanCheckResult.h"
#include "Device.h"

void Fence::Init(bool signalled)
{
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = signalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(VK_CALL(vkCreateFence(VK_DEVICE, &fenceCI, nullptr, &m_Fence)))
        throw std::runtime_error("Cannot create Fence!");
}

void Fence::Destroy()
{
    // Destroy the Fence
    vkDestroyFence(VK_DEVICE, m_Fence, nullptr);
}
