#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../utils/vertex.h"

#include "CmdPool.h"

class Buffer;

// What is the concern?
// buffer type how do I specify that in an API efficinet manner
// Q transition is used when we use EXCLUSIVE_SHARING_MODE, use VK_QUEUE_FAMILY_IGNORED if not interested
struct BufferTransitionInfo
{
    Buffer&         buffer;
    VkAccessFlags   currentAccess;
    VkAccessFlags   newAccess;
    uint32_t        currentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t        newQueueFamily = VK_QUEUE_FAMILY_IGNORED;

    BufferTransitionInfo(Buffer& buf, VkAccessFlags currentAccess, VkAccessFlags newAccess, uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED, uint32_t newQueueFamily = VK_QUEUE_FAMILY_IGNORED) : buffer(buf), currentAccess(currentAccess), newAccess(newAccess), currentQueueFamily(currentQueueFamily), newQueueFamily(newQueueFamily) {}
};

class BufferTransitionManager
{
public:

    static std::vector<BufferTransitionInfo> s_Transitions;
    static std::vector<VkBufferMemoryBarrier> s_BufMemBarriers;

    static void register_for_transtion(BufferTransitionInfo& buf);
    static void start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);
};


class Buffer
{
public:
    Buffer() = default;

    void Init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void Destroy();

    void MapVertexBufferData(const std::vector<Vertex>& vertexData);
    void MapIndexBufferData(const std::vector<uint16_t>& indexData);
    void MapImage(unsigned char* imageData, VkDeviceSize imageSize);
    void CopyBufferToDevice(CmdPool pool, VkBuffer dstBuffer, VkDeviceSize size);

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void copy_to_mapped(void* data, VkDeviceSize size);
    VkBufferView create_buffer_view(VkFormat viewFormat, VkDeviceSize offset, VkDeviceSize range);

    inline VkBuffer& get_handle() { return m_Buffer; }
    inline VkDeviceMemory& get_memory() { return m_BufferMemory; }
    void* get_mapped() { return m_Mapped; }
private:
    VkBuffer        m_Buffer;
    VkDeviceMemory  m_BufferMemory;
    void*           m_Mapped;
};
