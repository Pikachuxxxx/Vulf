#pragma once

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "CmdPool.h"
#include "Image.h"

// IDK how to properly and efficiently use these and create them efficientl so unless I get some pracitcal examples on 2D texture and CubeMap I can't refactor the API

class Texture
{
public:
    Texture() = default;

    void Init(const std::string& path);
    void Destroy();

    void upload_to_device(const void* imageData, VkDeviceSize imageSize, uint32_t width, uint32_t height);

    void update_set();

    Image get_handle() { return m_Image; }
    VkImage get_image() { return m_Image.get_handle(); }
    VkDeviceMemory get_image_memory() { return m_Image.get_memory(); }
    VkImageView get_view() { return m_Image.get_view(); }
    VkImageLayout get_layout() { return m_Image.get_descriptor_info().imageLayout; }

    VkDescriptorImageInfo get_descriptor_info() { return m_Image.get_descriptor_info(); }
    VkSampler get_sampler() { return m_TextureSampler; }
    VkDescriptorSet* get_descriptor_set() { return &descriptor_set; }
private:
    int     m_Height;
    int     m_Width;
    int     m_BPP;
    Image   m_Image; /* Abstracted Image implementation */

    // TODO: Refine these variables names and creation properly
    VkSampler m_TextureSampler;
    VkDescriptorPool m_DescriptorPool;
    VkDescriptorSetLayout g_DescriptorSetLayout;
    VkDescriptorSet descriptor_set;

private:
    unsigned char* load_image(const std::string& path);

};
