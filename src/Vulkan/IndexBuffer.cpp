#include "IndexBuffer.h"

void IndexBuffer::Init(std::vector<uint16_t> indices)
{
    Buffer m_StagingBuffer;

    m_IndexCount = indices.size();

    VkDeviceSize IndexDataSize = sizeof(indices[0]) * indices.size();
    m_StagingBuffer.Init(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.copy_to_mapped(indices.data(), IndexDataSize);

    m_IndexBuffer.Init(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.copy_buffer_to_device(m_IndexBuffer.get_handle(), IndexDataSize);
    m_StagingBuffer.Destroy();
}

void IndexBuffer::Destroy()
{
    m_IndexBuffer.Destroy();
}

void IndexBuffer::bind(VkCommandBuffer cmdBuffer)
{
    vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.get_handle(), 0, VK_INDEX_TYPE_UINT16);
}
