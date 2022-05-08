#include "Texture.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"
#include "CmdPool.h"

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void Texture::CreateTexture(const std::string& path, CmdPool& cmdPool)
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
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);    // Memory Property Flags


    /*
     *    There are two transitions we need to handle:
     *      Undefined → transfer destination: transfer writes that don't need to wait on anything
     *      Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
     */
    // change the image layout
    m_TextureImage.TransitionImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy the image to the buffer
    cmdPool.CopyBufferToImage(m_ImageStagingBuffer.get_buffer(), m_TextureImage.GetImage(), m_Width, m_Height);

    // Change the formate such that we can sample it from the shader
    m_TextureImage.TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create the image view for the texture (since a image can only be accessed through a view)
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
    vkGetPhysicalDeviceProperties(Device::Get()->get_gpu(), &properties);
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


    VkDescriptorPoolSize poolSize;
    poolSize.type = (VkDescriptorType) VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(1);
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(1);

    if (VK_CALL(vkCreateDescriptorPool(VKDEVICE, &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create the descriptor pool!");
    else VK_LOG_SUCCESS("successuflly create descriptor pool!");


    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;
    vkCreateDescriptorSetLayout(VKDEVICE, &info, nullptr, &g_DescriptorSetLayout);

    // if(allocated < 2)
    // Create Descriptor Set:
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_DescriptorPool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &g_DescriptorSetLayout;

        if (VK_CALL(vkAllocateDescriptorSets(VKDEVICE, &alloc_info, &descriptor_set)))
            throw std::runtime_error("failed to create texture descriptor set for texture!");
    }

    update_set();
}

void Texture::UploadTexture(const void* imageData, VkDeviceSize imageSize, uint32_t width, uint32_t height)
{

    m_ImageStagingBuffer.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_ImageStagingBuffer.MapImage((unsigned char*)imageData, imageSize);

    // Create the image
    m_TextureImage.CreateImage(width, height,
        VK_FORMAT_R8G8B8A8_UNORM,   //Format
        VK_IMAGE_TILING_OPTIMAL,    // Tiling
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // Image Usage Flags
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);    // Memory Property Flags

    /*
     *    There are two transitions we need to handle:
     *      Undefined → transfer destination: transfer writes that don't need to wait on anything
     *      Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
     */
    // change the image layout
    m_TextureImage.TransitionImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy the image to the buffer
    //cmdPool.CopyBufferToImage(m_ImageStagingBuffer.GetBuffer(), m_TextureImage.GetImage(), m_Width, m_Height);
    Device::Get()->copy_buffer_to_image(m_ImageStagingBuffer.get_buffer(), m_TextureImage.GetImage(), width, height);

    // Change the formate such that we can sample it from the shader
    m_TextureImage.TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create the image view for the texture (since a image can only be accessed through a view)
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
    vkGetPhysicalDeviceProperties(Device::Get()->get_gpu(), &properties);
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

unsigned char* Texture::LoadImage(const std::string& path)
{
    unsigned char* pixels = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, STBI_rgb_alpha);

    if(!pixels) {
        std::cout << "Cannot load image from  " << path << std::endl;
        return nullptr;
    }
    else VK_LOG("Image loaded succesfully! Dim : [", m_Width, ", ", m_Height, "]");
    return pixels;
}

void Texture::update_set()
{

    // Update the Descriptor Set:
    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = get_sampler();
        desc_image[0].imageView = get_image_view();
        desc_image[0].imageLayout = get_layout();
        VkWriteDescriptorSet write_desc[1] = {};
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = descriptor_set;
        write_desc[0].dstBinding = 0;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;
        vkUpdateDescriptorSets(VKDEVICE, 1, write_desc, 0, NULL);
    }
}

void Texture::Destroy()
{
    m_TextureImage.Destroy();
    vkDestroySampler(VKDEVICE, m_TextureSampler, nullptr);
}
