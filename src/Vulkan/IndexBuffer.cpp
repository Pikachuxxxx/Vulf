#include "IndexBuffer.h"

void IndexBuffer::Create(std::vector<uint16_t> vertices)
{
    Buffer m_StagingBuffer;

    VkDeviceSize IndexDataSize = sizeof(vertices[0]) * vertices.size();
    m_StagingBuffer.Init(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.copy_to_mapped(vertices.data(), IndexDataSize);

    m_IndexBuffer.Init(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.copy_buffer_to_device(m_IndexBuffer.get_handle(), IndexDataSize);
    m_StagingBuffer.Destroy();
}

void IndexBuffer::Destroy()
{
    m_IndexBuffer.Destroy();
}

void IndexBuffer::Bind(VkCommandBuffer cmdBuffer)
{
    vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.get_handle(), 0, VK_INDEX_TYPE_UINT16);
}
