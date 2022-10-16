#pragma once

#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "CmdPool.h"

class Image;

// TODO: Identify transition by string IDs so that we can re-use registered transitions
struct ImageTransitionInfo
{
    Image&              image;
    VkAccessFlags       currentAccess;
    VkAccessFlags       newAccess;
    VkImageLayout       oldLayout;
    VkImageLayout       newLayout;
    uint32_t            currentQueueFamily  = VK_QUEUE_FAMILY_IGNORED;
    uint32_t            newQueueFamily      = VK_QUEUE_FAMILY_IGNORED;
    VkImageAspectFlags  aspect              = VK_IMAGE_ASPECT_COLOR_BIT; // Bit mask specifying which aspects of an image are included in a view such as Color, Depth, Stencil etc.

    ImageTransitionInfo(Image& img, VkAccessFlags currentAccess, VkAccessFlags newAccess, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED, uint32_t newQueueFamily = VK_QUEUE_FAMILY_IGNORED) : image(img), currentAccess(currentAccess), newAccess(newAccess), oldLayout(oldLayout), newLayout(newLayout), aspect(aspect), currentQueueFamily(currentQueueFamily), newQueueFamily(newQueueFamily) { }
};

class ImageMemoryBarrier
{
public:

    static std::vector<ImageTransitionInfo> s_Transitions;
    static std::vector<VkImageMemoryBarrier> s_ImgMemBarriers;

    static void register_for_transtion(ImageTransitionInfo transitionInfo);
    static void start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);
    static void insert_barrier(ImageTransitionInfo transitionInfo, VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);

    static void clear();
};

class Image
{
public:
    Image() = default;

    // TODO: Add type info to Init
    void Init(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void Destroy();

    // Uses VK_IMAGE_VIEW_TYPE_2D by default for now until we deal with 3D or cube maps
    void create_image_view(/*Type type, */VkFormat format, VkImageAspectFlags aspectFlags);

    VkImage get_handle() const { return m_Image; }
    VkDeviceMemory get_memory() const { return m_ImageMemory; }
    VkImageView get_view() const { return m_ImageView; }
    VkSampler get_sampler() const { return m_ImageSampler; }
    VkDescriptorImageInfo get_descriptor_info() const { return m_DescriptorInfo; }

    VkImageLayout get_image_layout() { return m_DescriptorInfo.imageLayout; }
    void set_image_layout(VkImageLayout layout) { m_DescriptorInfo.imageLayout = layout; }
private:
    VkImage                     m_Image;
    VkDeviceMemory              m_ImageMemory;
    VkImageView                 m_ImageView;

    VkSampler                   m_ImageSampler;
    VkDescriptorImageInfo       m_DescriptorInfo;
};
