#pragma once

#include <vulkan/vulkan.h>
#include <vector>

/**
 * - Primary command buffers can be directly submitted to queues. They can also execute (call) secondary command buffers.
 * - Secondary command buffers can only be executed from primary command buffers, and we are not allowed to submit them.
 */

class CmdBuffer
{
public:

public:
    CmdBuffer() = default;

    void Init(const VkCommandPool& pool, bool isPrimary = true);
    void Destroy(const VkCommandPool& pool);

    void begin_recording();
    void end_recording();

    inline const VkCommandBuffer& get_handle() { return m_CommandBuffer; }
private:
    VkCommandBuffer m_CommandBuffer;

};
