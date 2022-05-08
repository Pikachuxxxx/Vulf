#include "Buffer.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

void Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(VK_CALL(vkCreateBuffer(VKDEVICE, &bufferInfo, nullptr, &m_Buffer)))
        throw std::runtime_error("Cannot create vertex buffer!");
    else VK_LOG_SUCCESS("Buffer successuflly created!");


    // Get the memory memRequirements of the vertex buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VKDEVICE, m_Buffer, &memRequirements);

    // Now allocate actual Physical memry for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Device::Get()->get_physical_device().find_memory_type_index(memRequirements.memoryTypeBits, properties);
    if(VK_CALL(vkAllocateMemory(VKDEVICE, &allocInfo, nullptr, &m_BufferMemory)))
        throw std::runtime_error("Cannot alocate memory for vertex buffer");

    // Bind the buffer to the allocated memory
    vkBindBufferMemory(VKDEVICE, m_Buffer, m_BufferMemory, 0);
}

void Buffer::MapVertexBufferData(const std::vector<Vertex>& vertexData)
{
    void* data;
    vkMapMemory(VKDEVICE, m_BufferMemory, 0, sizeof(vertexData[0]) * vertexData.size(), 0, &data);
    memcpy(data, vertexData.data(), (size_t) sizeof(vertexData[0]) * vertexData.size());
    vkUnmapMemory(VKDEVICE, m_BufferMemory);
}

void Buffer::MapIndexBufferData(const std::vector<uint16_t>& indexData)
{
    void* data;
    vkMapMemory(VKDEVICE, m_BufferMemory, 0, sizeof(indexData[0]) * indexData.size(), 0, &data);
    memcpy(data, indexData.data(), (size_t) sizeof(indexData[0]) * indexData.size());
    vkUnmapMemory(VKDEVICE, m_BufferMemory);
}

void Buffer::MapImage(unsigned char* imageData, VkDeviceSize imageSize)
{
    void* data;
    vkMapMemory(VKDEVICE, m_BufferMemory, 0, imageSize, 0, &data);
    std::cout << "\033[4;30;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    std::cout << "\033[4;33;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
    vkUnmapMemory(VKDEVICE, m_BufferMemory);
    std::cout << "\033[4;34;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
}

void Buffer::CopyBufferToDevice(CmdPool pool, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = pool.BeginSingleTimeBuffer();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, m_Buffer, dstBuffer, 1, &copyRegion);
    pool.EndSingleTimeBuffer(commandBuffer);
}

void Buffer::map_copy_unmap(void* data, VkDeviceSize size)
{
    vkMapMemory(VKDEVICE, m_BufferMemory, 0, size, 0, &m_Mapped);
    memcpy(m_Mapped, data, size);
    vkUnmapMemory(VKDEVICE, m_BufferMemory);
}

VkResult Buffer::map(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
{
    return vkMapMemory(VKDEVICE, m_BufferMemory, offset, size, 0, &m_Mapped);
}

void Buffer::unmap()
{
    if (m_Mapped == nullptr) {
        vkUnmapMemory(VKDEVICE, m_BufferMemory);
        //m_Mapped = nullptr;
    }
}

VkResult Buffer::flush(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_BufferMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(VKDEVICE, 1, &mappedRange);

}

void Buffer::DestroyBuffer()
{
    vkFreeMemory(VKDEVICE, m_BufferMemory, nullptr);
    vkDestroyBuffer(VKDEVICE, m_Buffer, nullptr);
}
