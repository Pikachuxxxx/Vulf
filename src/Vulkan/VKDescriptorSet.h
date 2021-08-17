#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "VKBuffer.h"

class VKDescriptorSet
{
public:
    VKDescriptorSet() = default;
    void CreateSets(uint32_t size, VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VKBuffer> uniformBuffers, uint32_t uboSize);
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
private:
    std::vector<VkDescriptorSet> m_DescriptorSets;

};
