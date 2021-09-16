#include "VKFramebuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"
#include <array>

void VKFramebuffer::Create(const VkRenderPass& renderpass, std::vector<VkImageView> swapViews, VkImageView depthImageView, VkExtent2D swapExtent)
{
    m_Framebuffers.resize(swapViews.size());

    for (size_t i = 0; i < swapViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo bufCI{};
        bufCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        bufCI.renderPass = renderpass;
        bufCI.attachmentCount = static_cast<uint32_t>(attachments.size());
        bufCI.pAttachments = attachments.data();
        bufCI.width = swapExtent.width;
        bufCI.height = swapExtent.height;
        bufCI.layers = 1;

        if(VK_CALL(vkCreateFramebuffer(VKDEVICE, &bufCI, nullptr, &m_Framebuffers[i])))
            throw std::runtime_error("Cannot create framebuffer!");
        // else VK_LOG("Framebuffer ", i ,"succesfully created!");
    }
}

void VKFramebuffer::Destroy()
{
    for (auto framebuffer : m_Framebuffers) {
        vkDestroyFramebuffer(VKDEVICE, framebuffer, nullptr);
    }
}
