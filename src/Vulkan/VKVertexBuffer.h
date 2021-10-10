#pragma once

#include "VKBuffer.h"
#include "VKCmdPool.h"

// TODO: Rename to VertexBuffer
class VKVertexBuffer
{
public:
    VKVertexBuffer() = default;
    void Create(std::vector<Vertex> vertices, VKCmdPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    VKBuffer m_VertexBuffer;
    VKBuffer  m_StagingBuffer;
};
