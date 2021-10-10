#pragma once

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "CmdPool.h"
#include "Image.h"

class DepthImage
{
public:
    DepthImage() = default;
    void CreateDepthImage(uint32_t width, uint32_t height, CmdPool cmdPool);
    void Destroy();
    bool HasStencilComponent(VkFormat format);
    VkFormat GetDepthFormat() { return m_DepthFormat; }
    VkImageView GetDepthImageView() const { return m_DepthImage.GetImageView(); }
    Image GetImage() const { return m_DepthImage; }
private:
    VkFormat m_DepthFormat;
    Image m_DepthImage;
};
