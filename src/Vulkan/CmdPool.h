#pragma once

#include <vulkan/vulkan.h>

#include <CmdBuffer.h>

/**
 * Command pools cannot be accessed at the same time from multiple threads (command buffers from the same pool cannot be recorded on multiple threads at the same time).
 * That's why each application thread on which a command buffer will be recorded should use separate command pools
 */

/* The pool from which the command buffers are Allocated from */
class CmdPool
{
public:
    CmdPool() = default;

    /**
     * Command buffers that are allocated from a given pool can be submitted only to queues from the specified family
     *
     * @param queueFamilyIndex The Queue family index which is used by the pool to allocate compatible command buffers
     * ex. Device::Get()->get_graphics_queue_index() for graphics commands and compute_queue for compute based commands
     */
    void Init(uint32_t queueFamilyIndex);
    void Destroy();

    // TODO: Provide both native and abstracted CommandBuffer Objects(native is first preference)

    /**
     * Allocates command buffers from the pool
     *
     * @param count The number of command buffers to allocate
     * @param isPrimary Whether or not to allocate primary command buffers
     *
     * @returns native vulkan handles to a vector of command buffers
     */
    std::vector<CmdBuffer> allocate_buffers(uint32_t count, bool isPrimary = true);

    inline const VkCommandPool& get_handle() { return m_CommandPool; }

private:
    VkCommandPool m_CommandPool;
};
