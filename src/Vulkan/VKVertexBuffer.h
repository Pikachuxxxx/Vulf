#pragma once

#include "VKBuffer.h"


class VKVertexBuffer
{
public:
    VKVertexBuffer() = default;
    void Create(std::vector<Vertex> vertices, VkCommandPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    VKBuffer m_VertexBuffer;
    VKBuffer  m_StagingBuffer;
};
