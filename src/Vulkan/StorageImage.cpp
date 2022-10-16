#include "StorageImage.h"

void StorageImage::Init(uint32_t width, uint32_t height)
{
    m_ImgHandle.Init(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //ImageTransitionInfo shaderRead(m_ImgHandle, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
    //ImageMemoryBarrier::insert_barrier(shaderRead, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    m_ImgHandle.create_image_view(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

}

void StorageImage::Destroy()
{
    m_ImgHandle.Destroy();
} 
