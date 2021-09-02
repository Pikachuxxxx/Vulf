#pragma once

#include <vulkan/vulkan.h>

#include "VKBuffer.h"
#include "VKCmdPool.h"
#include "VKImage.h"

class VKDepthImage
{
public:
    VKDepthImage() = default;
    void CreateDepthImage(uint32_t width, uint32_t height, VKCmdPool cmdPool);
    void Destroy();
    bool HasStencilComponent(VkFormat format);
    VkFormat GetDepthFormat() { return m_DepthFormat; }
    VkImageView GetDepthImageView() const { return m_DepthImage.GetImageView(); }
private:
    VkFormat m_DepthFormat;
    VKImage m_DepthImage;
};
