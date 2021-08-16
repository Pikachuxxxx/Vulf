#include "VKBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKBuffer::CreateBuffer(const std::vector<Vertex> vertexData)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertexData[0]) * vertexData.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(VK_CALL(vkCreateBuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &bufferInfo, nullptr, &m_VertexBuffer)))
        throw std::runtime_error("Cannot create vertex buffer!");

    // Get the memory memRequirements of the vertex buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBuffer, &memRequirements);

    // Now allocate actual Physical memry for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VKLogicalDevice::GetDeviceManager()->GetGPUManager().FindMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if(VK_CALL(vkAllocateMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, nullptr, &m_VertexBufferMemory)))
        throw std::runtime_error("Cannot alocate memory for vertex buffer");

    // Bind the buffer to the allocated memory
    vkBindBufferMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBuffer, m_VertexBufferMemory, 0);

    // Now fill the buffer with the vertex data
    void* buffer;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBufferMemory, 0, bufferInfo.size, 0, &buffer);
    memcpy(buffer, vertexData.data(), (uint32_t) bufferInfo.size);
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBufferMemory);
}

void VKBuffer::Bind(VkCommandBuffer& cmdBuffers)
{
    VkBuffer vertexBuffers[] = {m_VertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffers, 0, 1, vertexBuffers, offsets);
}

void VKBuffer::DestroyBuffer()
{
    vkDestroyBuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_VertexBufferMemory, nullptr);
}
