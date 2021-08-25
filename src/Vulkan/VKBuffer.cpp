#include "VKBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(VK_CALL(vkCreateBuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &bufferInfo, nullptr, &m_Buffer)))
        throw std::runtime_error("Cannot create vertex buffer!");
    // else VK_LOG_SUCCESS("Buffer successuflly created!");


    // Get the memory memRequirements of the vertex buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_Buffer, &memRequirements);

    // Now allocate actual Physical memry for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VKLogicalDevice::GetDeviceManager()->GetGPUManager().FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);
    if(VK_CALL(vkAllocateMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory)))
        throw std::runtime_error("Cannot alocate memory for vertex buffer");

    // Bind the buffer to the allocated memory
    vkBindBufferMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);
}

void VKBuffer::MapVertexBufferData(const std::vector<Vertex>& vertexData)
{
    void* data;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory, 0, sizeof(vertexData[0]) * vertexData.size(), 0, &data);
    memcpy(data, vertexData.data(), (size_t) sizeof(vertexData[0]) * vertexData.size());
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory);
}

void VKBuffer::MapIndexBufferData(const std::vector<uint16_t>& indexData)
{
    void* data;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory, 0, sizeof(indexData[0]) * indexData.size(), 0, &data);
    memcpy(data, indexData.data(), (size_t) sizeof(indexData[0]) * indexData.size());
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory);
}

void VKBuffer::MapImage(unsigned char* imageData, VkDeviceSize imageSize)
{
    void* data;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory, 0, imageSize, 0, &data);
    std::cout << "\033[4;30;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    std::cout << "\033[4;33;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory);
    std::cout << "\033[4;34;49m Hereeeeeee!!!!!!!!!!! \033[0m" << std::endl;
}

void VKBuffer::CopyBufferToDevice(VKCmdPool pool, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = pool.BeginSingleTimeBuffer();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, m_Buffer, dstBuffer, 1, &copyRegion);
    pool.EndSingleTimeBuffer(commandBuffer);
}

void VKBuffer::DestroyBuffer()
{
    vkDestroyBuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_Buffer, nullptr);
    vkFreeMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory, nullptr);
}
