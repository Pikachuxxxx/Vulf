#pragma once

/**
 * Fences, opposite to semaphores, are used to synchronize an application with
 * commands submitted to the graphics hardware. They inform the application when
 * the processing of a submitted work has been finished.Their state can be queried
 * by the application and the application can wait on fences until they become signaled.
 */
class Fence
{
public:
    Fence() = default;

    void Init(bool signalled);
    void Destroy();

    inline const VkFence& get_handle() { return m_Fence; }

private:
    VkFence m_Fence;
};
