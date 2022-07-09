#pragma once

#include "Buffer.h"

class IndexBuffer
{
public:
    IndexBuffer() = default;
    void Create(std::vector<uint16_t> vertices);
    void Destroy();

    void Bind(VkCommandBuffer cmdBuffer);
private:
    Buffer m_IndexBuffer;
};
