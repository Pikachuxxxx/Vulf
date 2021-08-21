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
    VkCommandBuffer BeginSingleTimeBuffer();
    void EndSingleTimeBuffer(const VkCommandBuffer& buffer);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
private:
    VkCommandPool m_CommandPool;
};
