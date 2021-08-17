#pragma once

#include "VKBuffer.h"

class VKIndexBuffer
{
public:
    VKIndexBuffer() = default;
    void Create(std::vector<uint16_t> vertices, VkCommandPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    VKBuffer m_IndexBuffer;
    VKBuffer  m_StagingBuffer;
};
