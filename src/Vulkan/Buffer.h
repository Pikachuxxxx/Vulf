#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../utils/vertex.h"

class Buffer;

struct BufferTransitionInfo
{
    Buffer&         buffer;
    VkAccessFlags   currentAccess;
    VkAccessFlags   newAccess;
    uint32_t        currentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t        newQueueFamily = VK_QUEUE_FAMILY_IGNORED;

    BufferTransitionInfo(Buffer& buf, VkAccessFlags currentAccess, VkAccessFlags newAccess, uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED, uint32_t newQueueFamily = VK_QUEUE_FAMILY_IGNORED) : buffer(buf), currentAccess(currentAccess), newAccess(newAccess), currentQueueFamily(currentQueueFamily), newQueueFamily(newQueueFamily) {}
};

class BufferMemoryBarrier
{
public:

    static std::vector<BufferTransitionInfo> s_Transitions;
    static std::vector<VkBufferMemoryBarrier> s_BufMemBarriers;

    static void register_for_transtion(BufferTransitionInfo transitionInfo);
    static void start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);
    static void insert_barrier(BufferTransitionInfo transitionInfo, VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);

    static void clear();
};

class Buffer
{
public:
    Buffer() = default;

    void Init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void Destroy();

    VkBufferView create_buffer_view(VkFormat viewFormat, VkDeviceSize offset, VkDeviceSize range);

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void copy_to_mapped(void* data, VkDeviceSize size);
    void copy_buffer_to_device(VkBuffer dstBuffer, VkDeviceSize size);

    inline VkBuffer& get_handle() { return m_Buffer; }
    inline VkDeviceMemory& get_memory() { return m_BufferMemory; }
    void* get_mapped() { return m_Mapped; }
private:
    VkBuffer        m_Buffer;
    VkDeviceMemory  m_BufferMemory;
    void*           m_Mapped;
};
