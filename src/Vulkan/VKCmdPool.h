#pragma once

#include <vulkan/vulkan.h>

class VKCmdPool
{
public:
    VKCmdPool() = default;
    VkCommandBuffer AllocateBuffer();
    void Init();
    void Destroy();
    const VkCommandPool& GetPool() { return m_CommandPool; }
private:
    VkCommandPool m_CommandPool;
};
