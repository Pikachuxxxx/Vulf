#pragma once

#include <vulkan/vulkan.h>
#include "VKCmdBuffer.h"

class VKRenderPass
{
public:
    VKRenderPass() = default;
    void Init(const VkFormat& format);
    VkRenderPass& GetRenderPass() { return m_RenderPass; }
    void SetClearColor(float* color) { m_ClearColor[0] = color[0];  m_ClearColor[1] = color[1]; m_ClearColor[2] = color[2]; m_ClearColor[3] = color[3];}
    void SetClearColor(float r, float g, float b) { m_ClearColor[0] = r;  m_ClearColor[1] = g; m_ClearColor[2] = b; m_ClearColor[3] = 1.0f;}
    void BeginRenderPass(VkCommandBuffer& cmdBuffer, VkFramebuffer& framebuffer, const VkExtent2D& swapextent);
    void EndRenderPass(VkCommandBuffer& cmdBuffer);
    void Destroy();
private:
    VkRenderPass m_RenderPass;
    float m_ClearColor[4];
};
