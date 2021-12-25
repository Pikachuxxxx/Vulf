#include "ImGuiOverlay.h"

#include "Vulkan/VKDevice.h"
#include "VulkanInitializers.hpp"
#include "utils/VulkanCheckResult.h"

#include "Vulkan/Buffer.h"

namespace Vulf {

    void ImGuiOverlay::init()
    {
        // Load the UI shaders
        m_OverlayVertShader.CreateShader((SHADER_BINARY_DIR) + std::string("/uioverlayVert.spv"), ShaderType::VERTEX_SHADER);
        m_OverlayFragShader.CreateShader((SHADER_BINARY_DIR) + std::string("/uioverlayFrag.spv"), ShaderType::FRAGMENT_SHADER);
        imguiOverlayShader.push_back(m_OverlayVertShader.GetShaderStageInfo());
        imguiOverlayShader.push_back(m_OverlayFragShader.GetShaderStageInfo());

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void) io;

        // Configure ImGui flags
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup Dear ImGui style
        // Color scheme - Red like Sascha Willems samples (https://github.com/SaschaWillems/Vulkan/blob/91958acad2c15f52bda74c58f6c39bd980207d2a/base/VulkanUIOverlay.cpp#L31)
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg]          = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
        style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header]           = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
        style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
        style.Colors[ImGuiCol_Button]           = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_ButtonActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    }

    void ImGuiOverlay::destroy()
    {
        if (ImGui::GetCurrentContext())
            ImGui::DestroyContext();
    }

    void ImGuiOverlay::upload_ui_font(const std::string& fontName)
    {
        ImGuiIO& io = ImGui::GetIO();

        // Create font texture
        unsigned char* fontData;
        int texWidth, texHeight;

        const std::string filename = ASSETS_DIR + std::string("/fonts/") + fontName;
        io.Fonts->AddFontFromFileTTF(filename.c_str(), 16.0f);

        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

        // Create and upload the font as a image texture to the GPU
        m_ImGuiFontTexture.UploadTexture(fontData, uploadSize, texWidth, texHeight);

        // Descriptor pool - Single pool for binding the font texture
        std::vector<VkDescriptorPoolSize> poolSizes = {
            Vulf::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
        VkDescriptorPoolCreateInfo descriptorPoolInfo = Vulf::initializers::descriptorPoolCreateInfo(poolSizes, 2);

        VK_CHECK_CALL(vkCreateDescriptorPool(VKDEVICE, &descriptorPoolInfo, nullptr, &m_ImguiDescriptorPool));

        // Descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            Vulf::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout = Vulf::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_CALL(vkCreateDescriptorSetLayout(VKDEVICE, &descriptorLayout, nullptr, &m_ImGuiDescriptorSetLayout));

        // Descriptor set
        VkDescriptorSetAllocateInfo allocInfo = Vulf::initializers::descriptorSetAllocateInfo(m_ImguiDescriptorPool, &m_ImGuiDescriptorSetLayout, 1);
        VK_CHECK_CALL(vkAllocateDescriptorSets(VKDEVICE, &allocInfo, &m_ImGuiDescriptorSet));
        VkDescriptorImageInfo fontDescriptor = Vulf::initializers::descriptorImageInfo(
            m_ImGuiFontTexture.get_sampler(),
            m_ImGuiFontTexture.get_image_view(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            Vulf::initializers::writeDescriptorSet(m_ImGuiDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
        };
        vkUpdateDescriptorSets(VKDEVICE, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }

    void ImGuiOverlay::prepare_pipeline(const VkRenderPass renderPass)
    {
        // Pipeline layout
        // Push constants for UI rendering parameters
        VkPushConstantRange pushConstantRange = Vulf::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Vulf::initializers::pipelineLayoutCreateInfo(&m_ImGuiDescriptorSetLayout, 1);
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        VK_CHECK_CALL(vkCreatePipelineLayout(VKDEVICE, &pipelineLayoutCreateInfo, nullptr, &m_ImGuiPipelineLayout));

        // Setup graphics pipeline for UI rendering
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            Vulf::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

        VkPipelineRasterizationStateCreateInfo rasterizationState =
            Vulf::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        // Enable blending
        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendState =
            Vulf::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

        VkPipelineDepthStencilStateCreateInfo depthStencilState =
            Vulf::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

        VkPipelineViewportStateCreateInfo viewportState =
            Vulf::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

        VkPipelineMultisampleStateCreateInfo multisampleState =
            Vulf::initializers::pipelineMultisampleStateCreateInfo(rasterizationSamples);

        std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState =
            Vulf::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = Vulf::initializers::pipelineCreateInfo(m_ImGuiPipelineLayout, renderPass);

        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(imguiOverlayShader.size());
        pipelineCreateInfo.pStages = imguiOverlayShader.data();
        pipelineCreateInfo.subpass = subpass;

        // Vertex bindings an attributes based on ImGui vertex definition
        std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
            Vulf::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
        };

        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
            Vulf::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
            Vulf::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
            Vulf::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
        };

        VkPipelineVertexInputStateCreateInfo vertexInputState = Vulf::initializers::pipelineVertexInputStateCreateInfo();
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
        vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        
        // TODO: Use pipeline cache for better optimization
        VK_CHECK_CALL(vkCreateGraphicsPipelines(VKDEVICE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_ImGuiPipeline));
    }

    bool ImGuiOverlay::update_imgui_buffers()
    {
        ImDrawData* imDrawData = ImGui::GetDrawData();
        bool updateCmdBuffers = false;

        if (!imDrawData) { return false; };

        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        // Update buffers only if vertex or index count has been changed compared to current buffer size
        if ((vertexBufferSize == 0) || (indexBufferSize == 0))
            return false;

        // Vertex buffer
        if ((m_ImGuiVBO.get_buffer() == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
            m_ImGuiVBO.unmap();
            VKLogicalDevice::Get()->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_ImGuiVBO, vertexBufferSize);

            vertexCount = imDrawData->TotalVtxCount;
            m_ImGuiVBO.unmap();
            m_ImGuiVBO.map();
            updateCmdBuffers = true;
        }


        // Index  buffer
        if ((m_ImGuiIBO.get_buffer() == VK_NULL_HANDLE) || (indexCount != imDrawData->TotalIdxCount)) {
            m_ImGuiIBO.unmap();

            VKLogicalDevice::Get()->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_ImGuiIBO, indexBufferSize);

            indexCount = imDrawData->TotalIdxCount;
            m_ImGuiIBO.unmap();
            m_ImGuiIBO.map();
            updateCmdBuffers = true;
        }

        // Upload vertex and index data to the GPU
        ImDrawVert* vtxDst = (ImDrawVert*) m_ImGuiVBO.get_mapped();
        ImDrawIdx* idxDst = (ImDrawIdx*) m_ImGuiIBO.get_mapped();

        for (int n = 0; n < imDrawData->CmdListsCount; n++) {
            const ImDrawList* cmd_list = imDrawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }

        m_ImGuiVBO.flush();
        m_ImGuiIBO.flush();

        return updateCmdBuffers;
    }

    void ImGuiOverlay::draw(const VkCommandBuffer& commandBuffer)
    {
        ImDrawData* imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();

        // Bind the pipeline and descriptor sets
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipelineLayout, 0, 1, &m_ImGuiDescriptorSet, 0, NULL);

        // Update the push constants
        pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        pushConstBlock.translate = glm::vec2(-1.0f);
        vkCmdPushConstants(commandBuffer, m_ImGuiPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

        // Bind the index and vertex buffers
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_ImGuiVBO.get_buffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_ImGuiIBO.get_buffer(), 0, VK_INDEX_TYPE_UINT16);

        for (uint32_t i = 0; i < imDrawData->CmdListsCount; ++i) {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissorRect;
                scissorRect.offset.x = std::max((int32_t) (pcmd->ClipRect.x), 0);
                scissorRect.offset.y = std::max((int32_t) (pcmd->ClipRect.y), 0);
                scissorRect.extent.width = (uint32_t) (pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = (uint32_t) (pcmd->ClipRect.w - pcmd->ClipRect.y);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
                vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmd_list->VtxBuffer.Size;
        }
    }

    void ImGuiOverlay::resize(uint32_t width, uint32_t height)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float) (width), (float) (height));
    }

    void ImGuiOverlay::free_resources()
    {
        m_ImGuiVBO.DestroyBuffer();
        m_ImGuiIBO.DestroyBuffer();
        vkDestroyImageView(VKDEVICE, m_ImGuiFontTexture.get_image_view(), nullptr);
        vkDestroyImage(VKDEVICE, m_ImGuiFontTexture.get_image(), nullptr);
        vkFreeMemory(VKDEVICE, m_ImGuiFontTexture.get_image_memory(), nullptr);
        vkDestroySampler(VKDEVICE, m_ImGuiFontTexture.get_sampler(), nullptr);
        vkDestroyDescriptorSetLayout(VKDEVICE, m_ImGuiDescriptorSetLayout, nullptr);
        //vkDestroyDescriptorPool(VKDEVICE, m_im, nullptr);
        vkDestroyPipelineLayout(VKDEVICE, m_ImGuiPipelineLayout, nullptr);
        vkDestroyPipeline(VKDEVICE, m_ImGuiPipeline, nullptr);
    }
}

