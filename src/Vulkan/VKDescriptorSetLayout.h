#pragma once

#include <vulkan/vulkan.h>

class VKDescriptorSetLayout
{
public:
    VKDescriptorSetLayout() = default;
    void CreateDescriptorSetLayout();
    void Destroy();
    VkDescriptorSetLayout& GetLayout() { return m_DesciptorLayout; }
private:
    VkDescriptorSetLayout m_DesciptorLayout;
};
