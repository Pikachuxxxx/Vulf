#include "Buffer.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

//------------------------------------------------------------------------------
// class : BufferTransitionManager

std::vector<BufferTransitionInfo> BufferTransitionManager::s_Transitions;
std::vector<VkBufferMemoryBarrier> BufferTransitionManager::s_BufMemBarriers;

void BufferTransitionManager::register_for_transtion(BufferTransitionInfo& transitionInfo)
{
    s_Transitions.push_back(transitionInfo);

    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = transitionInfo.currentAccess;
    barrier.dstAccessMask = transitionInfo.newAccess;
    barrier.srcQueueFamilyIndex = transitionInfo.currentQueueFamily;
    barrier.dstQueueFamilyIndex = transitionInfo.newQueueFamily;
    barrier.buffer = transitionInfo.buffer.get_handle();
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    s_BufMemBarriers.push_back(barrier);
}

void BufferTransitionManager::start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
    VkCommandBuffer cmdBuf = Device::Get()->begin_single_time_cmd_buffer();
    {
        vkCmdPipelineBarrier(cmdBuf, srcStages, dstStages, 0, 0, nullptr, static_cast<uint32_t>(s_BufMemBarriers.size()), &s_BufMemBarriers[0], 0, nullptr);
    }
    Device::Get()->end_single_time_cmd_buffer(cmdBuf);
}

//------------------------------------------------------------------------------

void Buffer::Init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    //--------------------------------------------------------------------------
    // Creating the buffer object

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // ummm.. the buffer is exclusive to a single queue familt at a time; cross sharing is not available

    if(VK_CALL(vkCreateBuffer(VKDEVICE, &bufferInfo, nullptr, &m_Buffer)))
        throw std::runtime_error("Cannot create buffer!");
    else VK_LOG_SUCCESS("Buffer successuflly created!");

    //--------------------------------------------------------------------------
    // Creating memory for the buffer

    // Get the memory memRequirements of the vertex buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VKDEVICE, m_Buffer, &memRequirements);

    // Now allocate actual Physical memory for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Device::Get()->get_physical_device().find_memory_type_index(memRequirements.memoryTypeBits, properties);
    if(VK_CALL(vkAllocateMemory(VKDEVICE, &allocInfo, nullptr, &m_BufferMemory)))
        throw std::runtime_error("Cannot alocate memory for vertex buffer");

    // Bind the buffer to the allocated memory
    vkBindBufferMemory(VKDEVICE, m_Buffer, m_BufferMemory, 0);
}

void Buffer::Destroy()
{
    vkFreeMemory(VKDEVICE, m_BufferMemory, nullptr);
    vkDestroyBuffer(VKDEVICE, m_Buffer, nullptr);
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
    VkCommandBuffer commandBuffer = Device::Get()->begin_single_time_cmd_buffer();
    {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, m_Buffer, dstBuffer, 1, &copyRegion);
    // Device::Get()->end_single_time_cmd_buffer(commandBuffer);
    // Flush it
    Device::Get()->flush_cmd_buffer(commandBuffer, Device::Get()->get_graphics_queue());
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

void Buffer::copy_to_mapped(void* data, VkDeviceSize size)
{
    vkMapMemory(VKDEVICE, m_BufferMemory, 0, size, 0, &m_Mapped);
    memcpy(m_Mapped, data, size);
    vkUnmapMemory(VKDEVICE, m_BufferMemory);
}

VkBufferView Buffer::create_buffer_view(VkFormat viewFormat, VkDeviceSize mem_offset, VkDeviceSize mem_range)
{
    VkBufferView bufferView;

    VkBufferViewCreateInfo viewCI{};
    viewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewCI.pNext = nullptr;
    viewCI.flags = 0;
    viewCI.buffer = m_Buffer;
    viewCI.format = viewFormat;
    viewCI.offset = mem_offset;
    viewCI.range = mem_range;

    if(VK_CALL(vkCreateBufferView(VKDEVICE, &viewCI, nullptr, &bufferView)))
        throw std::runtime_error("Cannot create buffer view!");

    return bufferView;
}
