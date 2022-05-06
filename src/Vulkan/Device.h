#pragma once

// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

#include "Buffer.h"

//------------------------------------------------------------------------------

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete()
    {
#ifdef __APPLE__
        return graphicsFamily.has_value() && presentFamily.has_value();
#endif

#ifdef _WIN32
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
#endif
    }
};

//------------------------------------------------------------------------------

class PhysicalDevice
{
public:
    PhysicalDevice() = default;

    void Init();

    uint32_t find_memory_type_index(uint32_t typeBitFieldFilter, VkMemoryPropertyFlags flags);
    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();

    inline VkPhysicalDevice& get_gpu() { return m_GPU; }
    inline QueueFamilyIndices& get_queue_family_indices() { return m_QueueFamilyIndices; }
    inline const uint32_t& get_graphics_family_index() const { return m_QueueFamilyIndices.graphicsFamily.value(); }
    inline const uint32_t& get_present_family_index() const { return m_QueueFamilyIndices.presentFamily.value(); }
    inline const uint32_t& get_compute_family_index() const { return m_QueueFamilyIndices.computeFamily.value(); }
    inline const char* get_device_name() { return m_DeviceProperties.deviceName; }

private:
    VkPhysicalDevice            m_GPU;
    VkPhysicalDeviceProperties  m_DeviceProperties;
    VkPhysicalDeviceFeatures    m_DeviceFeatures;
    QueueFamilyIndices          m_QueueFamilyIndices;

private:
    bool is_device_suitable(VkPhysicalDevice& gpu);
    void find_queue_family_indices(VkPhysicalDevice gpu);
};

//------------------------------------------------------------------------------

#define VKDEVICE Device::Get()->get_handle()

extern std::vector<const char*> deviceExtensions; // User specifie device extensions

class Device
{
public:
    Device() = default;

    void Init();
    void Destroy();

    static Device* Get() { if(s_Instance ==  nullptr) s_Instance = new Device; return s_Instance; }

    VkCommandBuffer begin_single_time_cmd_buffer(VkCommandBufferLevel level, bool begin = true);
    void end_single_time_cmd_buffer(VkCommandBuffer cmdBuf);
    void flush_cmd_buffer(VkCommandBuffer cmdBuf, VkQueue queue, bool free = true);

    VkResult create_buffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory, void* data = nullptr);
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data = nullptr);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    inline VkDevice& get_handle() { return m_Device; }
    inline PhysicalDevice& get_physical_device() { return m_GPUManager; }
    inline VkPhysicalDevice& get_gpu() { return m_GPUManager.get_gpu(); }
    inline const uint32_t& get_graphics_queue_index() { return m_GPUManager.get_graphics_family_index(); }
    inline const uint32_t& get_present_queue_index() { return m_GPUManager.get_present_family_index(); }
    inline const uint32_t& get_compute_queue_index() { return m_GPUManager.get_compute_family_index(); }
    inline const VkQueue& get_graphics_queue() const { return m_GraphicsQueue; }
    inline const VkQueue& get_present_queue() const { return m_PresentQueue; }
    inline const VkQueue& get_compute_queue() const { return m_ComputeQueue; }
    inline VkCommandPool& get_instaneous_cmd_pool() { return m_InstantaneousCmdPool; }

private:
    static Device*          s_Instance;
    VkDevice                m_Device;
    PhysicalDevice          m_GPUManager;
    VkQueue                 m_GraphicsQueue;
    VkQueue                 m_PresentQueue;
    VkQueue                 m_ComputeQueue;
    VkQueue                 m_TransferQueue;
    VkCommandPool           m_InstantaneousCmdPool;

private:
    void create_queues();
};
