#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "Buffer.h"
#include "Texture.h"

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

class UniformBuffer
{
public:
    UniformBuffer() = default;
    void Destroy();
    void UpdateBuffer(UniformBufferObject buffer, uint32_t index);
    void UpdateLightBuffer(LightUniformBufferObject buffer, uint32_t index);
    void CreateUniformBuffer(uint32_t swapImagesCount, Texture& texture);
    void CreateDescriptorSetLayout();
    void CreatePool();
    void CreateSets();
    void UpdateDescriptorSetConfig();
    const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_UBODescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
    const VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
private:
    Texture m_Texture;
    VkDescriptorSetLayout m_UBODescriptorSetLayout;
    // Uniform buffer for each swap chain image, which is usually 3
    std::vector<Buffer> m_UniformBuffers;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;
};
