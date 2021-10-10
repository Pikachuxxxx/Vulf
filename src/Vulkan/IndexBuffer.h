#pragma once

#include "Buffer.h"
#include "CmdPool.h"

class IndexBuffer
{
public:
    IndexBuffer() = default;
    void Create(std::vector<uint16_t> vertices, CmdPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    Buffer m_IndexBuffer;
    Buffer  m_StagingBuffer;
};
