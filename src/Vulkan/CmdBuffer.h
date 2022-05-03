#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class CmdBuffer
{
public:
    CmdBuffer() = default;
    void AllocateBuffers(const VkCommandPool& pool, uint32_t count);
    void RecordBuffer(VkCommandBuffer& buffer);
    void EndRecordingBuffer(VkCommandBuffer& buffer);
    void Destroy(const VkCommandPool& pool);
    std::vector<VkCommandBuffer>& GetBuffers() { return m_CommandBuffers; }
    VkCommandBuffer& GetBufferAt(int index) { return m_CommandBuffers[index];}
    uint32_t GetBuffersCount() { return m_CommandBuffers.size(); }
    static VkCommandBuffer BeginSingleTimeBuffer();
    static void EndSingleTimeBuffer(const VkCommandBuffer& buffer);
private:
    std::vector<VkCommandBuffer> m_CommandBuffers;

};
