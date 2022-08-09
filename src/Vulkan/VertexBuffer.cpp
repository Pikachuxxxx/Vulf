#include "VertexBuffer.h"

void VertexBuffer::Init(std::vector<Vertex> vertices)
{
    Buffer  m_StagingBuffer;

    m_VtxCount = vertices.size();

    VkDeviceSize vertexDataSize = sizeof(vertices[0]) * vertices.size();
    m_StagingBuffer.Init(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.copy_to_mapped(vertices.data(), vertexDataSize);

    m_VertexBuffer.Init(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.copy_buffer_to_device(m_VertexBuffer.get_handle(), vertexDataSize);
    m_StagingBuffer.Destroy();
}

void VertexBuffer::Destroy()
{
    m_VertexBuffer.Destroy();
}

void VertexBuffer::bind(VkCommandBuffer cmdBuffer)
{
    VkBuffer vertexBuffers[] = {m_VertexBuffer.get_handle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
}
