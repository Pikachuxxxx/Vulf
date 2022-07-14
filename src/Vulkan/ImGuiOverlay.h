#pragma once
#include <string>

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// ImGuizmo
#include <ImGuizmo/ImGuizmo.h>

#include "Vulkan/Texture.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/Shader.h"

namespace Vulf{

    /* Creates and Manager ImGui for the Vulkan application */
    class ImGuiOverlay
    {
    public:
        struct PushConstBlock
        {
            glm::vec2 scale;
            glm::vec2 translate;
        };

    public:
        PushConstBlock                                  pushConstBlock;
        std::vector<VkPipelineShaderStageCreateInfo>    imguiOverlayShader;
        VkSampleCountFlagBits                           rasterizationSamples    = VK_SAMPLE_COUNT_1_BIT;
        uint32_t                                        subpass                 = 0;

    public:
        ImGuiOverlay() = default;
        virtual ~ImGuiOverlay() {}

        void init();
        void destroy();

        void upload_ui_font(const std::string& fontName);
        void prepare_pipeline(const VkRenderPass renderPass);
        bool update_imgui_buffers();
        void draw(const VkCommandBuffer& commandBuffer);
        void resize(uint32_t width, uint32_t height);
        void free_resources();

        void setImageSet(VkDescriptorSet* set) { m_ImguiImageSet = set; }
    private:
        VkDeviceMemory          m_FontMemory = VK_NULL_HANDLE;
        Texture                 m_ImGuiFontTexture;

        VkDescriptorPool        m_ImguiDescriptorPool;
        VkDescriptorSetLayout   m_ImGuiDescriptorSetLayout;
        VkDescriptorSet         m_ImGuiDescriptorSet;
        VkDescriptorSet*        m_ImguiImageSet;

        VkPipelineLayout        m_ImGuiPipelineLayout;
        VkPipeline              m_ImGuiPipeline;

        Buffer                  m_ImGuiVBO;
        Buffer                  m_ImGuiIBO;

        int32_t                 vertexCount = 0;
        int32_t                 indexCount = 0;

        Shader                  m_OverlayVertShader;
        Shader                  m_OverlayFragShader;

        // ImGui render pass and command buffers
        VkRenderPass m_ImGuiRenderpass;
        VkCommandBuffer m_ImGuiCmdBuffers;

    };
}
