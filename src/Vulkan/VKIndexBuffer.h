#pragma once

#include "VKBuffer.h"
#include "VKCmdPool.h"

// TODO: Rename to IndexBuffer
class VKIndexBuffer
{
public:
    VKIndexBuffer() = default;
    void Create(std::vector<uint16_t> vertices, VKCmdPool pool);
    void Destroy();
    void Bind(VkCommandBuffer cmdBuffer);
private:
    VKBuffer m_IndexBuffer;
    VKBuffer  m_StagingBuffer;
};
