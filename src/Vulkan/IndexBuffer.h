#pragma once

#include "Buffer.h"

class IndexBuffer
{
public:
    IndexBuffer() = default;
    void Init(std::vector<uint16_t> indices);
    void Destroy();

    void bind(VkCommandBuffer cmdBuffer);

    inline const uint32_t& get_index_count() { return m_IndexCount; }
private:
    Buffer m_IndexBuffer;
    uint32_t m_IndexCount = 0;
};
