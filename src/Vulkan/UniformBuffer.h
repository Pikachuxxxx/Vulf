#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"

// TODO: calculate uniform buffer size using descriptors total size
class UniformBuffer
{
public:
    struct DescriptorInfo
    {
        enum DescriptorType
        {
            BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, IMAGE = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        } type;
        uint32_t bindingID;
        ShaderType shaderType;
        uint32_t size;
        uint32_t offset;
        Texture image;

        DescriptorInfo(uint32_t bindingID, ShaderType shaderType, uint32_t size, uint32_t offset) : bindingID(bindingID), shaderType(shaderType), size(size), offset(offset), type(BUFFER) {}
        DescriptorInfo(uint32_t bindingID, ShaderType shaderType, Texture image) : bindingID(bindingID), shaderType(shaderType), image(image), type(IMAGE) {}
    };

public:
    UniformBuffer() = default;
    void AddDescriptor(DescriptorInfo setInfo);
    void CreateDescriptorSetLayout();

    void CreateUniformBuffer(uint32_t swapImagesCount, uint32_t bufferSize);

    void UpdateDescriptorSetConfig();

    void UpdateBuffer(void* buffer, uint32_t bufferSize, uint32_t index);
    
    void Destroy();

    // Getter/Setter for private vars
    const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_UBODescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
    const VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
private:
    std::vector<Buffer>                         m_UniformBuffers;           /* Uniform buffer for each swap chain image, which is usually 3 */
    std::vector<VkDescriptorSetLayoutBinding>   m_LayoutBindings;           /* Layout binding info for the descriptor sets */
    VkDescriptorSetLayout                       m_UBODescriptorSetLayout;
    VkDescriptorPool                            m_DescriptorPool;
    std::vector<VkDescriptorSet>                m_DescriptorSets;

    /************************************************************************/
    std::vector<DescriptorInfo>                 m_Descriptors;

private:
    void CreatePool();
    void CreateSets();
};

//template<>
//void UniformBuffer::AddDescriptorSetConfig<UniformBuffer::DescriptorBufferInfo>(UniformBuffer::DescriptorBufferInfo setConfig) {
//    m_BufferInfos.push_back(setConfig);
//}
//
//template<>
//void UniformBuffer::AddDescriptorSetConfig<UniformBuffer::DescriptorImageInfo>(UniformBuffer::DescriptorImageInfo setConfig) {
//    m_ImageInfos.push_back(setConfig);
//}
