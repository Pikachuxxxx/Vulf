#pragma once

#include <vulkan/vulkan.h>

#include <string>
/**
 * Semaphores allow us to synchronize operations submitted to queues, not only
 * within one queue, but also between different queues (of any family type) in
 * one logical device.
 *
 * @note Basically they are GPU-GPU sync primitives, they are automatically reset
 * when the command buffers waiting on them are resumed/finished their work.
 * The application doesn't have access to their state, for CPU-GPU sync we need to use Fences
 */
class Semaphore
{
public:
    Semaphore() = default;

    void Init(std::string name = "Semaphore");
    void Destroy();

    // signal/un-signal and reset the semaphore API
    void signal();

    inline const VkSemaphore& get_handle() { return m_Semaphore; }
    inline const bool& is_signalled() const { return m_IsSignalled; }

private:
    VkSemaphore m_Semaphore;
    bool        m_IsSignalled;
};
