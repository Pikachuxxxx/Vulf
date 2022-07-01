#include "DepthImage.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"
#include "CmdPool.h"

void DepthImage::CreateDepthImage(uint32_t width, uint32_t height, CmdPool cmdPool)
{
    // Get the proper format for the depth image
    m_DepthFormat = Device::Get()->get_physical_device().find_depth_format();

    // Create the proper image views for the depth texture
    m_DepthImage.Init(width, height,
        m_DepthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_DepthImage.CreateImageView(m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

bool DepthImage::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void DepthImage::Destroy()
{
    m_DepthImage.Destroy();
}
