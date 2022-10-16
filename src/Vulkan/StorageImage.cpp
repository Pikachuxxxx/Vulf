#include "StorageImage.h"

void StorageImage::Init(uint32_t width, uint32_t height)
{
    m_ImgHandle.Init(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void StorageImage::Destroy()
{
    m_ImgHandle.Destroy();
} 
