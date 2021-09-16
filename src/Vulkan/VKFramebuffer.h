#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VKFramebuffer
{
public:
    VKFramebuffer() = default;
    void Create(const VkRenderPass& renderpass, std::vector<VkImageView> swapViews, VkImageView depthImageView, VkExtent2D swapExtent);
    void Destroy();
    std::vector<VkFramebuffer> GetFramebuffers() { return m_Framebuffers; }
    VkFramebuffer& GetFramebufferAt(int index) { return m_Framebuffers[index]; }
private:
    std::vector<VkFramebuffer> m_Framebuffers;
};
