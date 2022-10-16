#pragma once

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"

class UniformBuffer;
class StorageBuffer;
class Texture;
class StorageImage;

enum DescriptorType
{
    UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    //STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,

    COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    STORAGE_IMAGE = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
};

struct DescriptorInfo
{
    DescriptorType  type;
    uint32_t        bindingID;
    ShaderType      shaderType;
    UniformBuffer*  uniformBuffer;
    Texture*        image;
    StorageImage*   storageImage;
    
    DescriptorInfo(DescriptorType type, uint32_t bindingID, ShaderType shaderType) : type(type), bindingID(bindingID), shaderType(shaderType) {}

    template <class T>
    void attach_resource(void* resource)
    {

    }

    template <>
    void attach_resource<Texture>(void* resource)
    {
        image = (Texture*)resource;
    }

    template <>
    void attach_resource<UniformBuffer>(void* resource)
    {
        uniformBuffer = (UniformBuffer*)resource;
    }

    template <>
    void attach_resource<StorageImage>(void* resource)
    {
        storageImage = (StorageImage*)resource;
    }
};  

/* Used to bind desc sets to shaders */
class DescriptorSet
{
public:
    void Init(std::vector<DescriptorInfo> descriptorInfos);
    void Destroy();

    void update_set();

    inline VkDescriptorSetLayout get_set_layout() { return m_SetLayout; }
    inline VkDescriptorSet get_set() { return m_DescriptorSet; }

    static inline const uint32_t& get_current_set_allocations() { return m_DescriptorPoolCurrentAllocations; }

private:
    static uint32_t                 m_DescriptorPoolCurrentAllocations; /* Tracking the current no of descriptor sets allocated from the pool */
    std::vector<DescriptorInfo>     m_DescriptorsInfos;
    VkDescriptorSetLayout           m_SetLayout;
    VkDescriptorSet                 m_DescriptorSet;
};

