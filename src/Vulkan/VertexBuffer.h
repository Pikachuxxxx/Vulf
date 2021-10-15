#pragma once

#include "Buffer.h"
#include "CmdPool.h"

class VertexBuffer
{
public:
    VertexBuffer() = default;
    void Create(std::vector<Vertex> vertices, CmdPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    Buffer m_VertexBuffer;
    Buffer  m_StagingBuffer;
};
