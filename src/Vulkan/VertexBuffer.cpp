#include "VertexBuffer.h"

void VertexBuffer::Create(std::vector<Vertex> vertices, CmdPool pool)
{
    VkDeviceSize vertexDataSize = sizeof(vertices[0]) * vertices.size();
    m_StagingBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.MapVertexBufferData(vertices);

    m_VertexBuffer.CreateBuffer(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.CopyBufferToDevice(pool, m_VertexBuffer.GetBuffer(), vertexDataSize);
    m_StagingBuffer.DestroyBuffer();
}

void VertexBuffer::Destroy()
{
    m_VertexBuffer.DestroyBuffer();
}

void VertexBuffer::Bind(VkCommandBuffer cmdBuffer)
{
    VkBuffer vertexBuffers[] = {m_VertexBuffer.GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
}
