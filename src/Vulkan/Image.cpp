#include "Image.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"
#include "CmdPool.h"

void Image::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    // Fill the image create info
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1; // Currently we do not support mip maps yet!
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling; // Because of staging buffer the linearity of it not necessary as we won't be interacting with it at all
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    // Create the Image
    if(VK_CALL(vkCreateImage(VKDEVICE, &imageInfo, nullptr, &m_Image)))
        throw std::runtime_error("Cannot create texture image");
    else VK_LOG_SUCCESS("Texture VK Image created successuflly");

    // Allocate the memory for the image(get the memRequirements) and bind it to the VKTextureImage
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(VKDEVICE, m_Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Device::Get()->get_physical_device().find_memory_type_index(memRequirements.memoryTypeBits, properties);

    if(VK_CALL(vkAllocateMemory(VKDEVICE, &allocInfo, nullptr, &m_ImageMemory)))
        throw std::runtime_error("Cannot allocate image memory!");
    else VK_LOG_SUCCESS("Successuflly allocated image memory");

    // Bind the image with the memory
    vkBindImageMemory(VKDEVICE, m_Image, m_ImageMemory, 0);

    // Create the sampler for image
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(Device::Get()->get_gpu(), &physicalDeviceProperties);
    samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    if (VK_CALL(vkCreateSampler(VKDEVICE, &samplerInfo, nullptr, &m_ImageSampler)))
       throw std::runtime_error("failed to create texture sampler!");

    // Create the descriptor Info for the Image
    m_DescrioptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_DescrioptorInfo.imageView = m_ImageView;
    m_DescrioptorInfo.sampler = m_ImageSampler;
}

void Image::CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if(VK_CALL(vkCreateImageView(VKDEVICE, &viewInfo, nullptr, &m_ImageView)))
        throw std::runtime_error("failed to create texture image view!");
}

void Image::TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer copyCmdBuffer = Device::Get()->begin_single_time_cmd_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);


    // Transition the image layout using a proper memory barrier to suncrhonise write cycle before we read from it
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Image;
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
        copyCmdBuffer,
        m_ImageSourceStage, m_ImageDestinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    Device::Get()->flush_cmd_buffer(copyCmdBuffer, Device::Get()->get_graphics_queue());
}


void Image::Destroy()
{
    vkFreeMemory(VKDEVICE, m_ImageMemory, nullptr);
    vkDestroySampler(VKDEVICE, m_ImageSampler, nullptr);
    vkDestroyImageView(VKDEVICE, m_ImageView, nullptr);
    vkDestroyImage(VKDEVICE, m_Image, nullptr);
}
