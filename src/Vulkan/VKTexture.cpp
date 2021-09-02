#include "VKTexture.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"
#include "VKCmdPool.h"

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void VKTexture::CreateTexture(const std::string& path, VKCmdPool& cmdPool)
{
    stbi_uc* imageData = LoadImage(path);
    VkDeviceSize imageSize = m_Height * m_Width * 4;
    std::cout << "\033[4;33;49m Hmmmmmmmmmmmm \033[0m" << std::endl;
    std::cout << "\033[4;33;49m Image Height : " << m_Height << ", Width : " << m_Width << " BPP : " << m_BPP << " Size : " << imageSize << "\033[0m" << std::endl;
    m_ImageStagingBuffer.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_ImageStagingBuffer.MapImage(imageData, imageSize);

    stbi_image_free(imageData);

    // Create the image
    m_TextureImage.CreateImage(m_Width, m_Height,
        VK_FORMAT_R8G8B8A8_UNORM,   //Format
        VK_IMAGE_TILING_OPTIMAL,    // Tiling
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // Image Usage Flags
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,    // Memory Property Flags
        cmdPool);

    /*
     *    There are two transitions we need to handle:
     *      Undefined → transfer destination: transfer writes that don't need to wait on anything
     *      Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
     */
    // change the image layout
    m_TextureImage.TransitionImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdPool);

    // Copy the image to the buffer
    cmdPool.CopyBufferToImage(m_ImageStagingBuffer.GetBuffer(), m_TextureImage.GetImage(), m_Width, m_Height);

    // Change the formate such that we can sample it from the shader
    m_TextureImage.TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool);

    // Create the image view for the texture (since a image can onl be accesed through a view)
    m_TextureImage.CreateImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    // Create the sampler for image view
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(VKLogicalDevice::GetDeviceManager()->GetGPUManager().GetGPU(), &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (VK_CALL(vkCreateSampler(VKDEVICE, &samplerInfo, nullptr, &m_TextureSampler)))
       throw std::runtime_error("failed to create texture sampler!");


    // Delete the staging buffer
    m_ImageStagingBuffer.DestroyBuffer();
}

unsigned char* VKTexture::LoadImage(const std::string& path)
{
    unsigned char* pixels = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, STBI_rgb_alpha);

    if(!pixels) {
        std::cout << "Cannot load image from  " << path << std::endl;
        return nullptr;
    }
    else VK_LOG("Image loaded succesfully! Dim : [", m_Width, ", ", m_Height, "]");
    return pixels;
}

void VKTexture::Destroy()
{
    m_TextureImage.Destroy();
    vkDestroySampler(VKDEVICE, m_TextureSampler, nullptr);
}
