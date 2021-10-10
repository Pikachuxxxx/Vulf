#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../utils/vertex.h"

#include "CmdPool.h"

class Buffer
{
public:
    Buffer() = default;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void DestroyBuffer();
    void MapVertexBufferData(const std::vector<Vertex>& vertexData);
    void MapIndexBufferData(const std::vector<uint16_t>& indexData);
    void MapImage(unsigned char* imageData, VkDeviceSize imageSize);
    void CopyBufferToDevice(CmdPool pool, VkBuffer dstBuffer, VkDeviceSize size);
    const VkBuffer& GetBuffer() { return m_Buffer; }
    const VkDeviceMemory& GetBufferMemory() { return m_BufferMemory; }
private:
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
};
