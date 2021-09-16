#pragma once

#include <vulkan/vulkan.h>

#include "VKBuffer.h"
#include "VKCmdPool.h"
#include "VKImage.h"

class VKTexture
{
public:
    VKTexture() = default;
    void CreateTexture(const std::string& path, VKCmdPool& cmdPool);
    VkImageView CreateImageView(VkImage image, VkFormat format);
    void Destroy();
    unsigned char* LoadImage(const std::string& path);
    VkPipelineStageFlags GetCurrentImageSourceStage() { return m_ImageSourceStage; }
    VkPipelineStageFlags GetCurrentImageDestinationStage() { return m_ImageDestinationStage; }
    VkImage GetTextureImage() { return m_TextureImage.GetImage(); }
    VkDeviceMemory GetTextureImageMemory() { return m_TextureImage.GetImageMemory(); }
    VkImageView GetTextureImageView() { return m_TextureImage.GetImageView(); }
    VkSampler GetTextureImageSampler() { return m_TextureSampler; }
private:
    int m_Height;
    int m_Width;
    int m_BPP;
    VKBuffer m_ImageStagingBuffer;
    VkSampler m_TextureSampler;
    // TODO: Read about these in detail!
    VkPipelineStageFlags m_ImageSourceStage;
    VkPipelineStageFlags m_ImageDestinationStage;
    // Abstarcted Image
    VKImage m_TextureImage;
};
