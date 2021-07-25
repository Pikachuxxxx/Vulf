#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VKCmdBuffer
{
public:
    VKCmdBuffer() = default;
    void AllocateBuffers(const VkCommandPool& pool);
    void RecordBuffers();
    void EndRecordingBuffers();
    void DestroyBuffers();
    std::vector<VkCommandBuffer>& GetBuffers() { return m_CommandBuffers; }
    VkCommandBuffer& GetBufferAt(int index) { return m_CommandBuffers[index];}
private:
    std::vector<VkCommandBuffer> m_CommandBuffers;
};
