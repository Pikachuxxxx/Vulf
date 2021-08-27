#include "VKRenderPass.h"

#include "../utils/VulkanCheckResult.h"
#include "VKDevice.h"

void VKRenderPass::Init(const VkFormat& format)
{
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.format = format;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Now this not the final presentation coz, of imgui (we are outputting to the frame buffer color attachment)
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // This binds the attachment usage description with the Attahcment on the Vulkan side i.e. with the color attachment

    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
////////////////////////////////////////////////////////////////////////////////
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
////////////////////////////////////////////////////////////////////////////////
    VkRenderPassCreateInfo renderpassCI{};
    renderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassCI.attachmentCount = 1;
    renderpassCI.pAttachments = &attachment;
    renderpassCI.subpassCount = 1;
    renderpassCI.pSubpasses = &subpassDesc;
    renderpassCI.dependencyCount = 1;
    renderpassCI.pDependencies = &dependency;

    if(VK_CALL(vkCreateRenderPass(VKDEVICE, &renderpassCI, nullptr, &m_RenderPass)))
        throw std::runtime_error("Cannot create render pass");
    else VK_LOG_SUCCESS("Render pass succesfully created!");
}

void VKRenderPass::Destroy()
{
    vkDestroyRenderPass(VKDEVICE, m_RenderPass, nullptr);
}

void VKRenderPass::BeginRenderPass(VkCommandBuffer& cmdBuffer, VkFramebuffer& framebuffer, const VkExtent2D& swapextent)
{
    // VK_LOG("Starting render pass!");
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = m_RenderPass;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent = swapextent;
	beginInfo.clearValueCount = 1;
	VkClearValue clearColor = { {{m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]}} };
	beginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKRenderPass::EndRenderPass(VkCommandBuffer& cmdBuffer)
{
    vkCmdEndRenderPass(cmdBuffer);
}
