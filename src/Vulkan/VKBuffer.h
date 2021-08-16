#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../vertex.h"

class VKBuffer
{
public:
    VKBuffer() = default;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void DestroyBuffer();
    void MapVertexBufferData(const std::vector<Vertex>& vertexData);
    void MapIndexBufferData(const std::vector<uint16_t>& indexData);
    void CopyBufferToDevice(VkCommandPool pool, VkBuffer dstBuffer, VkDeviceSize size);
    void BindVertexBuffer(VkCommandBuffer& cmdBuffers);
    void BindIndexBuffer(VkCommandBuffer& cmdBuffers);
    const VkBuffer& GetBuffer() { return m_Buffer; }
private:
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
};
