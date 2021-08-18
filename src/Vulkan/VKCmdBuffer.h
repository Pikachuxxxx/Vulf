#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VKCmdBuffer
{
public:
    VKCmdBuffer() = default;
    void AllocateBuffers(const VkCommandPool& pool);
    void RecordBuffer(VkCommandBuffer& buffer);
    void EndRecordingBuffer(VkCommandBuffer& buffer);
    void DestroyBuffer(VkCommandBuffer& buffer);
    std::vector<VkCommandBuffer>& GetBuffers() { return m_CommandBuffers; }
    VkCommandBuffer& GetBufferAt(int index) { return m_CommandBuffers[index];}
    uint32_t GetBuffersCount() { return m_CommandBuffers.size(); }
private:
    std::vector<VkCommandBuffer> m_CommandBuffers;

};
