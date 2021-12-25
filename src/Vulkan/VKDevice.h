#pragma once

// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

#include "Vulkan/Buffer.h"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VKPhysicalDevice
{
public:
    VKPhysicalDevice() = default;
    void Init();
    VkPhysicalDevice GetGPU() { return m_GPU; }
    QueueFamilyIndices& GetQueueFamilyIndices() { return m_QueueFamilyIndices; }
    uint32_t FindMemoryTypeIndex(uint32_t typeBitFieldFilter, VkMemoryPropertyFlags flags);
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    uint32_t GetGraphicsFamilyIndex() { return m_QueueFamilyIndices.graphicsFamily.value(); }
    const std::string& get_device_name() { return deviceProperties.deviceName; }
private:
    VkPhysicalDevice m_GPU;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    QueueFamilyIndices m_QueueFamilyIndices;
private:
    bool isDeviceSuitable(VkPhysicalDevice& gpu);
    void findQueueFamilyIndices(VkPhysicalDevice gpu);
};

extern std::vector<const char*> deviceExtensions;

class VKLogicalDevice
{
public:
    VKLogicalDevice() = default;
    void Init();
    void Destroy();
    static VKLogicalDevice* Get() { if(s_Instance ==  nullptr) s_Instance = new VKLogicalDevice; return s_Instance; }
    VkDevice GetLogicalDevice() { return m_Device; }
    VKPhysicalDevice& GetGPUManager() { return m_GPUManager; }
    VkQueue GetGraphicsQueue() { return graphicsQueue; }
    VkQueue GetPresentQueue() { return presentQueue; }
    VkCommandPool& getDevicecmdPool() { return m_InstantaneousCmdPool; }

    VkCommandBuffer begin_single_time_command_buffer(VkCommandBufferLevel level, bool begin = false);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data = nullptr);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
private:
    VkDevice m_Device;
    VKPhysicalDevice m_GPUManager;
    static VKLogicalDevice* s_Instance;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool m_InstantaneousCmdPool;
private:
    void CreateQueues();
};
