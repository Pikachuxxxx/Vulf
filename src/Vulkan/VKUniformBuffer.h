#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "VKBuffer.h"
#include "VKTexture.h"

struct UniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 _padding1;
    alignas(16) glm::mat4 _padding2;
};

struct LightUniformBufferObject
{
    alignas(16) glm::vec3 objectColor;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 _padding;
};

// TODO: Rename to UniformBuffer
class VKUniformBuffer
{
public:
    VKUniformBuffer() = default;
    void Destroy();
    void UpdateBuffer(UniformBufferObject buffer, uint32_t index);
    void UpdateLightBuffer(LightUniformBufferObject buffer, uint32_t index);
    void CreateUniformBuffer(uint32_t swapImagesCount, VKTexture& texture);
    void CreateDescriptorSetLayout();
    void CreatePool();
    void CreateSets();
    void UpdateDescriptorSetConfig();
    const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_UBODescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
    const VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
private:
    VKTexture m_Texture;
    VkDescriptorSetLayout m_UBODescriptorSetLayout;
    // Uniform buffer for each swap chain image, which is usually 3
    std::vector<VKBuffer> m_UniformBuffers;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;
};
