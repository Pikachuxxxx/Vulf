#include "VKIndexBuffer.h"

void VKIndexBuffer::Create(std::vector<uint16_t> vertices, VkCommandPool pool)
{
    VkDeviceSize IndexDataSize = sizeof(vertices[0]) * vertices.size();
    m_StagingBuffer.CreateBuffer(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.MapIndexBufferData(vertices);

    m_IndexBuffer.CreateBuffer(IndexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.CopyBufferToDevice(pool, m_IndexBuffer.GetBuffer(), IndexDataSize);
    m_StagingBuffer.DestroyBuffer();
}

void VKIndexBuffer::Destroy()
{
    m_IndexBuffer.DestroyBuffer();
}

void VKIndexBuffer::Bind(VkCommandBuffer cmdBuffer)
{
    vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
}
