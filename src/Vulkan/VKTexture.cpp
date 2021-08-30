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
    VkDeviceSize imageSize = m_Height * m_Width * m_BPP;
    std::cout << "\033[4;33;49m Hmmmmmmmmmmmm \033[0m" << std::endl;
    std::cout << "\033[4;33;49m Image Height : " << m_Height << ", Width : " << m_Width << " BPP : " << m_BPP << " Size : " << imageSize << "\033[0m" << std::endl;
    m_ImageStagingBuffer.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_ImageStagingBuffer.MapImage(imageData, imageSize);

    stbi_image_free(imageData);
    // Now us a image to copy staging buffer data to it, image is faster for GPU to acces pixel data
    // (hence we make a VkImage abd Memory for it apart frm staging buffer and it's memory)
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_Width;
    imageInfo.extent.height = m_Height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1; // Currently we do not support mipmaps yet!
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Because of staging bufffer the linearity of it not necessay as we won't be interacting with it at all
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    // Create the Image
    if(VK_CALL(vkCreateImage(VKDEVICE, &imageInfo, nullptr, &m_TextureImage)))
        throw std::runtime_error("Cannot create texture image");
    else VK_LOG_SUCCESS("Texture VK Image created successuflly");

    // Allocate the memory for the image(get the memRequirements) and bind it to the VKTextureImage
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(VKDEVICE, m_TextureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VKLogicalDevice::GetDeviceManager()->GetGPUManager().FindMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(VK_CALL(vkAllocateMemory(VKDEVICE, &allocInfo, nullptr, &m_TextureImageMemory)))
        throw std::runtime_error("Cannot allocate texture image memroy");
    else VK_LOG_SUCCESS("Successuflly allocated memory for texture");

    // Bind the memory
    vkBindImageMemory(VKDEVICE, m_TextureImage, m_TextureImageMemory, 0);

/*
 *    There are two transitions we need to handle:
 *      Undefined → transfer destination: transfer writes that don't need to wait on anything
 *      Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
 */
    // change the image layout
    TransitionImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdPool);

    // Copy the image to the buffer
    cmdPool.CopyBufferToImage(m_ImageStagingBuffer.GetBuffer(), m_TextureImage, m_Width, m_Height);

    // Change the formate such that we can sample it from the shader
    TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool);

    // Create the image view for the texture (since a image can onl be accesed through a view)
    m_TextureImageView = CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM);

    // Create the sampler for image view
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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

VkImageView VKTexture::CreateImageView(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if(VK_CALL(vkCreateImageView(VKDEVICE, &viewInfo, nullptr, &imageView)))
        throw std::runtime_error("failed to create texture image view!");
    return imageView;
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

void VKTexture::TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VKCmdPool& pool)
{
    VkCommandBuffer commandBuffer = pool.BeginSingleTimeBuffer();

    // Transition the image layout using a proper memory barrier to suncrhonise write cycle before we read from it
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_TextureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    /*
     *    There are two transitions we need to handle:
     *      Undefined → transfer destination: transfer writes that don't need to wait on anything
     *      Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
     */

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // Transfer writes must occur in the pipeline transfer stage. Since the writes don't have to wait on anything,
        // you may specify an empty access mask and the earliest possible pipeline stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
        // for the pre-barrier operations.
        m_ImageSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // This is more like an undefined layout stata (similar meaning!)
        m_ImageDestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        m_ImageSourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        m_ImageDestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        m_ImageSourceStage, m_ImageDestinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    pool.EndSingleTimeBuffer(commandBuffer);
}

void VKTexture::Destroy()
{
    vkDestroySampler(VKDEVICE, m_TextureSampler, nullptr);
    vkDestroyImageView(VKDEVICE, m_TextureImageView, nullptr);
    vkDestroyImage(VKDEVICE, m_TextureImage, nullptr);
    vkFreeMemory(VKDEVICE, m_TextureImageMemory, nullptr);
}
