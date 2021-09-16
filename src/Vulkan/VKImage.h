#pragma once

#include <vulkan/vulkan.h>

#include "VKBuffer.h"
#include "VKCmdPool.h"

class VKImage
{
public:
    VKImage() = default;
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VKCmdPool& cmdPool);
    void Destroy();
    void CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags);
    void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VKCmdPool& pool);
    // Getter and setter for the image resoruces
    VkImage GetImage() const { return m_Image; }
    VkImageView GetImageView() const { return m_ImageView; }
    VkDeviceMemory GetImageMemory() const { return m_ImageMemory; }
    VkSampler GetImageSampler() const { return m_ImageSampler; }
    VkDescriptorImageInfo GetDescriptorInfo() const { return m_DescrioptorInfo; }
private:
    VkImage m_Image;
    VkDeviceMemory m_ImageMemory;
    VkImageView m_ImageView;
    // TODO: Read about these in detail!
    VkPipelineStageFlags m_ImageSourceStage;
    VkPipelineStageFlags m_ImageDestinationStage;
    VkSampler m_ImageSampler;
    VkDescriptorImageInfo m_DescrioptorInfo;
};
