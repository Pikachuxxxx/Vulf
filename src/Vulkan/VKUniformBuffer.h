#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "VKBuffer.h"

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class VKUniformBuffer
{
public:
    VKUniformBuffer() = default;
    void Destroy();
    void UpdateBuffer(UniformBufferObject buffer, uint32_t index);
    void CreateUniformBuffer(uint32_t swapImagesCount);
    void CreateDescriptorSetLayout();
    void CreatePool();
    void CreateSets();
    void UpdateDescriptorSetConfig();
    const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_UBODescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
    const VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
private:
    VkDescriptorSetLayout m_UBODescriptorSetLayout;
    // Uniform buffer for each swap chain image, which is usually 3
    std::vector<VKBuffer> m_UniformBuffers;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;
};
