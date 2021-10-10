#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

#include "Buffer.h"
#include "Texture.h"

class DescriptorSet
{
public:
    DescriptorSet() = default;
    void CreateSets(uint32_t size, VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, std::vector<Buffer> uniformBuffers, uint32_t uboSize, Texture texture);
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
private:
    std::vector<VkDescriptorSet> m_DescriptorSets;

};
