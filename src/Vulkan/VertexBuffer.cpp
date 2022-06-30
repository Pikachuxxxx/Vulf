#include "VertexBuffer.h"

void VertexBuffer::Create(std::vector<Vertex> vertices, CmdPool pool)
{
    VkDeviceSize vertexDataSize = sizeof(vertices[0]) * vertices.size();
    m_StagingBuffer.Init(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_StagingBuffer.MapVertexBufferData(vertices);

    m_VertexBuffer.Init(vertexDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_StagingBuffer.CopyBufferToDevice(pool, m_VertexBuffer.get_handle(), vertexDataSize);
    m_StagingBuffer.Destroy();
}

void VertexBuffer::Destroy()
{
    m_VertexBuffer.Destroy();
}

void VertexBuffer::Bind(VkCommandBuffer cmdBuffer)
{
    VkBuffer vertexBuffers[] = {m_VertexBuffer.get_handle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
}
