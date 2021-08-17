#pragma once

// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

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
    VkPhysicalDevice& GetGPU() { return m_GPU; }
    QueueFamilyIndices& GetQueueFamilyIndices() { return m_QueueFamilyIndices; }
    uint32_t FindMemoryTypeIndex(uint32_t typeBitFieldFilter, VkMemoryPropertyFlags flags);
    uint32_t GetGraphicsFamilyIndex() { return m_QueueFamilyIndices.graphicsFamily.value(); }
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
    void Init();
    void Destroy();
    static VKLogicalDevice* GetDeviceManager() { if(s_Instance ==  nullptr) s_Instance = new VKLogicalDevice; return s_Instance; }
    VkDevice& GetLogicalDevice() { return m_Device; }
    VKPhysicalDevice& GetGPUManager() { return m_GPUManager; }
    VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    VkQueue& GetPresentQueue() { return presentQueue; }
private:
    VkDevice m_Device;
    VKPhysicalDevice m_GPUManager;
    static VKLogicalDevice* s_Instance;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
private:
    VKLogicalDevice() = default;
    void CreateQueues();
};
