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
    Buffer m_ImageStagingBuffer;
    VkSampler m_TextureSampler;
    // TODO: Read about these in detail!
    VkPipelineStageFlags m_ImageSourceStage;
    VkPipelineStageFlags m_ImageDestinationStage;
    // Abstarcted Image
    Image m_TextureImage;
};
