#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../vertex.h"

class VKBuffer
{
public:
    VKBuffer() = default;
    void CreateBuffer(const std::vector<Vertex> vertexData);
    void DestroyBuffer();
    void Bind(VkCommandBuffer& cmdBuffers);
    const VkBuffer& GetBuffer() { return m_VertexBuffer; }
private:
    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
};
