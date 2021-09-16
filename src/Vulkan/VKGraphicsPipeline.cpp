#include "VKGraphicsPipeline.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"
#include "../utils/vertex.h"

void VKGraphicsPipeline::Create(std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VKFixedPipelineFuncs& fixedfunctions, VkRenderPass& renderpass)
{
    VkGraphicsPipelineCreateInfo graphicsCI{};
    graphicsCI.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsCI.stageCount           = shaderStages.size();
    graphicsCI.pStages              = shaderStages.data();

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();
    // we need the vertex binding information and attribute info to create the Input create info struct
    VkPipelineVertexInputStateCreateInfo m_VertexInputSCI = {};
    m_VertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_VertexInputSCI.flags = 0;
    m_VertexInputSCI.pNext = nullptr;
    m_VertexInputSCI.vertexBindingDescriptionCount = 1;
    m_VertexInputSCI.vertexAttributeDescriptionCount = attributeDescriptions.size();
    m_VertexInputSCI.pVertexBindingDescriptions = &bindingDescription;
    m_VertexInputSCI.pVertexAttributeDescriptions = attributeDescriptions.data();

    graphicsCI.pVertexInputState    = &m_VertexInputSCI;
    graphicsCI.pInputAssemblyState  = &fixedfunctions.GetInputAssemblySCI();
    graphicsCI.pViewportState       = &fixedfunctions.GetViewportStateCI();
    graphicsCI.pRasterizationState  = &fixedfunctions.GetRazterizerSCI();
    graphicsCI.pMultisampleState    = &fixedfunctions.GetMultiSampleSCI();
    graphicsCI.pDepthStencilState   = &fixedfunctions.GetDepthStencilSCI();
    graphicsCI.pColorBlendState     = &fixedfunctions.GetColorBlendSCI();
    graphicsCI.pDynamicState        = nullptr;
    graphicsCI.layout               = fixedfunctions.GetPipelineLayout();
    graphicsCI.renderPass           = renderpass;
    graphicsCI.subpass              = 0;
    graphicsCI.basePipelineHandle   = VK_NULL_HANDLE;
    graphicsCI.basePipelineIndex    = -1;

    if(VK_CALL(vkCreateGraphicsPipelines(VKDEVICE, VK_NULL_HANDLE, 1, &graphicsCI, nullptr, &m_GraphicsPipeline)))
        throw std::runtime_error("Cannot create Graphics pipeline!");
    else VK_LOG_SUCCESS("Graphics Pipeline succesfully crreated!");
}

void VKGraphicsPipeline::Destroy()
{
    vkDestroyPipeline(VKDEVICE, m_GraphicsPipeline, nullptr);
}

void VKGraphicsPipeline::Bind(VkCommandBuffer& cmdBuffer)
{
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
}
