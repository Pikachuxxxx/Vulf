#pragma once

#include <vulkan/vulkan.h>
#include "CmdBuffer.h"

class RenderPass
{
public:
    RenderPass() = default;
    void Init(const VkFormat& format);
    void Destroy();

    void begin_pass(VkCommandBuffer& cmdBuffer, VkFramebuffer& framebuffer, const VkExtent2D& swapextent);
    void end_pass(VkCommandBuffer& cmdBuffer);

    void add_subpass() {}

    inline const VkRenderPass& get_handle() { return m_RenderPass; }
    inline void set_clear_color(float r, float g, float b) { m_ClearColor[0] = r;  m_ClearColor[1] = g; m_ClearColor[2] = b; m_ClearColor[3] = 1.0f;}
    inline void set_clear_color(float* color) { m_ClearColor[0] = color[0];  m_ClearColor[1] = color[1]; m_ClearColor[2] = color[2]; m_ClearColor[3] = color[3];}
private:
    VkRenderPass    m_RenderPass;
    float           m_ClearColor[4];
};
