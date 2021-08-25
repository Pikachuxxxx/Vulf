#include "VKFramebuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKFramebuffer::Create(const VkRenderPass& renderpass, std::vector<VkImageView> swapViews, VkExtent2D swapExtent)
{
    m_Framebuffers.resize(swapViews.size());

for (size_t i = 0; i < swapViews.size(); i++) {
    VkImageView attachments[] = {
     swapViews[i]
 };

    VkFramebufferCreateInfo bufCI{};
    bufCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    bufCI.renderPass = renderpass;
    bufCI.attachmentCount = 1;
    bufCI.pAttachments = &swapViews[i];
    bufCI.width = swapExtent.width;
    bufCI.height = swapExtent.height;
    bufCI.layers = 1;

    if(VK_CALL(vkCreateFramebuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &bufCI, nullptr, &m_Framebuffers[i])))
        throw std::runtime_error("Cannot create framebuffer!");
    // else VK_LOG("Framebuffer ", i ,"succesfully created!");
}
}

void VKFramebuffer::Destroy()
{
    for (auto framebuffer : m_Framebuffers) {
        vkDestroyFramebuffer(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), framebuffer, nullptr);
    }
}
