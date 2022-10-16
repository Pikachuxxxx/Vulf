#pragma once

#include <vulkan/vulkan.h>

#include "Image.h"

class StorageImage
{
public:
    void Init(uint32_t width, uint32_t height);
    void Destroy();


private:
    Image m_ImgHandle;


};

