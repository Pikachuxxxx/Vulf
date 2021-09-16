#pragma once

#include <vulkan/vulkan.h>
#include <array>

class VKDescriptorPool
{
public:
    VKDescriptorPool() = default;
    void CreatePool(uint32_t size);
    void Destroy();
    VkDescriptorPool GetDescriptorPool() { return m_DescriptorPool; }
private:
    VkDescriptorPool m_DescriptorPool;
};