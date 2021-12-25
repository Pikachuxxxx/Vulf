#pragma once

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "CmdPool.h"
#include "Image.h"

class Texture
{
public:
    Texture() = default;
    void CreateTexture(const std::string& path, CmdPool& cmdPool);
    void UploadTexture(const void* imageData, VkDeviceSize imageSize, uint32_t width, uint32_t height);
    VkImageView CreateImageView(VkImage image, VkFormat format);
    void Destroy();
    unsigned char* LoadImage(const std::string& path);


    VkPipelineStageFlags GetCurrentImageSourceStage() { return m_ImageSourceStage; }
    VkPipelineStageFlags GetCurrentImageDestinationStage() { return m_ImageDestinationStage; }
    VkImage get_image() { return m_TextureImage.GetImage(); }
    VkDeviceMemory get_image_memory() { return m_TextureImage.GetImageMemory(); }
    VkImageView get_image_view() { return m_TextureImage.GetImageView(); }
    VkSampler get_sampler() { return m_TextureSampler; }
private:
    int m_Height;
    int m_Width;
    int m_BPP;
    Buffer m_ImageStagingBuffer;
    VkSampler m_TextureSampler;
    VkPipelineStageFlags m_ImageSourceStage;
    VkPipelineStageFlags m_ImageDestinationStage;
    // Abstracted Image
    Image m_TextureImage;
};