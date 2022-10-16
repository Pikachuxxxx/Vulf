#include "StorageImage.h"

#include "Device.h"

void StorageImage::Init(uint32_t width, uint32_t height)
{
    m_ImgHandle.Init(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_ImgHandle.set_image_layout(VK_IMAGE_LAYOUT_UNDEFINED);

    ImageTransitionInfo shaderRead(m_ImgHandle, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    ImageMemoryBarrier::insert_barrier(shaderRead, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    m_ImgHandle.set_image_layout(VK_IMAGE_LAYOUT_GENERAL);

    m_ImgHandle.create_image_view(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
}

void StorageImage::Destroy()
{
    m_ImgHandle.Destroy();
} 

void StorageImage::clear(glm::vec4 clearColor)
{
    VkCommandBuffer cmdBuffer = Device::Get()->begin_single_time_cmd_buffer();

    // Set it to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    m_ImgHandle.set_image_layout(VK_IMAGE_LAYOUT_GENERAL);

    ImageTransitionInfo transferRead(m_ImgHandle, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ImageMemoryBarrier::insert_barrier(transferRead, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkClearColorValue color = { .float32 = {clearColor.x, clearColor.y, clearColor.z, clearColor.w} };
    VkImageSubresourceRange imageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdClearColorImage(cmdBuffer, m_ImgHandle.get_handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &imageSubresourceRange);

    // Set it to back to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    m_ImgHandle.set_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    ImageTransitionInfo shaderRead(m_ImgHandle, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
    ImageMemoryBarrier::insert_barrier(shaderRead, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    Device::Get()->flush_cmd_buffer(cmdBuffer, Device::Get()->get_graphics_queue());
}
