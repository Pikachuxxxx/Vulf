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

    void map_copy_unmap(void* data, VkDeviceSize size);
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    VkBuffer& get_buffer() { return m_Buffer; }
    VkDeviceMemory& get_memory() { return m_BufferMemory; }
    void* get_mapped() { return m_Mapped; }
private:
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
    void* m_Mapped;
};
