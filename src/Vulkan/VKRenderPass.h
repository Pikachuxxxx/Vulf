#pragma once

#include <vulkan/vulkan.h>
#include "VKCmdBuffer.h"

class VKRenderPass
{
public:
    VKRenderPass() = default;
    void Init(const VkFormat& format);
    VkRenderPass& GetRenderPass() { return m_RenderPass; }
    void SetClearColor(float r, float g, float b) { m_ClearColor[0] = r;  m_ClearColor[1] = g; m_ClearColor[2] = b;}
    void BeginRenderPass(VKCmdBuffer& cmdBuffers, std::vector<VkFramebuffer> framebuffers, const VkExtent2D& swapextent);
    void EndRenderPass(VKCmdBuffer& cmdBuffers);
    void Destroy();
private:
    VkRenderPass m_RenderPass;
    float m_ClearColor[3];
};
