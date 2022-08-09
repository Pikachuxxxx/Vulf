#pragma once

#include "Buffer.h"
#include "CmdPool.h"

class VertexBuffer
{
public:
    VertexBuffer() = default;
    void Init(std::vector<Vertex> vertices);
    void Destroy();

    void bind(VkCommandBuffer cmdBuffer);
    inline const uint32_t& get_vtx_count() { return m_VtxCount; }
private:
    uint32_t m_VtxCount;
    Buffer  m_VertexBuffer;
};
