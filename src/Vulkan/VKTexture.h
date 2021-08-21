#pragma once

#include <vulkan/vulkan.h>

#include "VKBuffer.h"
#include "VKCmdPool.h"

class VKTexture
{
public:
    VKTexture() = default;
    void CreateTexture(const std::string& path, VKCmdPool& cmdPool);
    VkImageView CreateImageView(VkImage image, VkFormat format);
    void Destroy();
    unsigned char* LoadImage(const std::string& path);
    void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VKCmdPool& pool);
    VkPipelineStageFlags GetCurrentImageSourceStage() { return m_ImageSourceStage; }
    VkPipelineStageFlags GetCurrentImageDestinationStage() { return m_ImageDestinationStage; }
    VkImage GetTextureImage() { return m_TextureImage; }
    VkDeviceMemory GetTextureImageMemory() { return m_TextureImageMemory; }
    VkImageView GetTextureImageView() { return m_TextureImageView; }
    VkSampler GetTextureImageSampler() { return m_TextureSampler; }
private:
    int m_Height;
    int m_Width;
    int m_BPP;
    VKBuffer m_ImageStagingBuffer;
    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;
    // TODO: Read about these in detail!
    VkPipelineStageFlags m_ImageSourceStage;
    VkPipelineStageFlags m_ImageDestinationStage;
};
