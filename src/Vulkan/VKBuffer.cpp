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

void VKBuffer::CopyBufferToDevice(VkCommandPool pool, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, m_Buffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VKLogicalDevice::GetDeviceManager()->GetGraphicsQueue());
    vkFreeCommandBuffers(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), pool, 1, &commandBuffer);
}

void VKBuffer::BindVertexBuffer(VkCommandBuffer& cmdBuffers)
{
    VkBuffer vertexBuffers[] = {m_Buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffers, 0, 1, vertexBuffers, offsets);
}

void VKBuffer::BindIndexBuffer(VkCommandBuffer& cmdBuffers)
{
    vkCmdBindIndexBuffer(cmdBuffers, m_Buffer, 0, VK_INDEX_TYPE_UINT16);
}

void VKBuffer::DestroyBuffer()
{
    vkDestroyBuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_Buffer, nullptr);
    vkFreeMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_BufferMemory, nullptr);
}
