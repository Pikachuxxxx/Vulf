#include "FixedPipelineFuncs.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"
#include "../utils/vertex.h"


void FixedPipelineFuncs::SetFixedPipelineStage(VkPrimitiveTopology topology, const VkExtent2D& swapchainExtent, bool enableWireFrameMode)
{
    SetInputAssemblyStageInfo(topology);
    SetViewportSCI(swapchainExtent);
    SetRasterizerSCI(enableWireFrameMode);
    SetMultiSampleSCI();
    SetDepthStencilSCI();
    SetColorBlendSCI();
    SetDynamicSCI();
}
// TODO: Add methods to add bindings and attributes dynamically

void FixedPipelineFuncs::SetVertexInputSCI()
{
    // Get the Vertex Attribuite descrioption and binding information
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();
    std::cout << "Binding Description Input Rate : " << bindingDescription.inputRate << std::endl;
    // we need the vertex binding information and attribute info to create the Input create info struct
   // m_VertexInputSCI = {};
    m_VertexInputSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_VertexInputSCI.flags = 0;
    m_VertexInputSCI.pNext = nullptr;
    m_VertexInputSCI.vertexBindingDescriptionCount = 1;
    m_VertexInputSCI.pVertexBindingDescriptions = &bindingDescription;
    m_VertexInputSCI.vertexAttributeDescriptionCount = attributeDescriptions.size();
    m_VertexInputSCI.pVertexAttributeDescriptions = attributeDescriptions.data();
}

void FixedPipelineFuncs::SetInputAssemblyStageInfo(VkPrimitiveTopology topology)
{
    m_InputAssemblySCI = {};
    m_InputAssemblySCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    // switch (topology) {
    //     case Topology::POINTS:
    //         m_InputAssemblySCI.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    //         break;
    //     case Topology::TRIANGLES:
    //         m_InputAssemblySCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //         break;
    //     case Topology::LINES:
    //         m_InputAssemblySCI.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    //         break;
    //     case Topology::LINE_STRIP:
            m_InputAssemblySCI.topology = topology;
    // }
    m_InputAssemblySCI.primitiveRestartEnable = VK_FALSE;
}

void FixedPipelineFuncs::SetViewportSCI(const VkExtent2D& swapchainExtent)
{
    viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapchainExtent.width;
    viewport.height = swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    // Create the swapchain create info struct
    m_ViewportSCI = {};
    m_ViewportSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_ViewportSCI.viewportCount = 1;
    m_ViewportSCI.pViewports = &viewport;
    m_ViewportSCI.scissorCount = 1;
    m_ViewportSCI.pScissors = &scissor;
}

void FixedPipelineFuncs::SetRasterizerSCI(int cullMode, bool enableWireFrameMode)
{
    m_RasterizerSCI = {};
    m_RasterizerSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_RasterizerSCI.depthClampEnable = VK_FALSE;
    m_RasterizerSCI.rasterizerDiscardEnable = VK_FALSE;
    if(enableWireFrameMode)
        m_RasterizerSCI.polygonMode = VK_POLYGON_MODE_LINE;
    else
        m_RasterizerSCI.polygonMode = VK_POLYGON_MODE_FILL;
    m_RasterizerSCI.cullMode = cullMode;//VK_CULL_MODE_NONE;//
    m_RasterizerSCI.frontFace = VK_FRONT_FACE_CLOCKWISE;//VK_FRONT_FACE_COUNTER_CLOCKWISE;//
    m_RasterizerSCI.depthBiasEnable = VK_FALSE;
    m_RasterizerSCI.depthBiasConstantFactor = 0.0f;
    m_RasterizerSCI.depthBiasClamp = 0.0f;
    m_RasterizerSCI.depthBiasSlopeFactor = 0.0f;
    m_RasterizerSCI.lineWidth = 1.0f;
}

void FixedPipelineFuncs::SetMultiSampleSCI()
{
    m_MultiSampleSCI = {};
    m_MultiSampleSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_MultiSampleSCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_MultiSampleSCI.sampleShadingEnable = VK_FALSE;
    m_MultiSampleSCI.minSampleShading = 1.0f;
    m_MultiSampleSCI.pSampleMask = nullptr;
    m_MultiSampleSCI.alphaToCoverageEnable = VK_FALSE;
    m_MultiSampleSCI.alphaToOneEnable = VK_FALSE;
}

void FixedPipelineFuncs::SetDepthStencilSCI(int depthCompareOp)
{
    // TODO: Fill this later (for now pass a nullptr)
    m_DepthStencilSCI = {};
    m_DepthStencilSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_DepthStencilSCI.depthTestEnable = VK_TRUE;
    m_DepthStencilSCI.depthWriteEnable = VK_TRUE;
    ////////////////////////////////////////////////////
    m_DepthStencilSCI.depthCompareOp = (VkCompareOp)depthCompareOp;
    ////////////////////////////////////////////////////
    m_DepthStencilSCI.depthBoundsTestEnable = VK_FALSE;
    m_DepthStencilSCI.minDepthBounds = 0.0f; // Optional
    m_DepthStencilSCI.maxDepthBounds = 1.0f; // Optional
    m_DepthStencilSCI.stencilTestEnable = VK_FALSE;
    m_DepthStencilSCI.front = {}; // Optional
    m_DepthStencilSCI.back = {}; // Optional

}

void FixedPipelineFuncs::SetColorBlendSCI()
{
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    m_ColorBlendSCI = {};
    m_ColorBlendSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_ColorBlendSCI.logicOpEnable = VK_FALSE;
    m_ColorBlendSCI.logicOp = VK_LOGIC_OP_COPY;
    m_ColorBlendSCI.attachmentCount = 1;
    m_ColorBlendSCI.pAttachments = &colorBlendAttachment;
    m_ColorBlendSCI.blendConstants[0] = 0.0f;
    m_ColorBlendSCI.blendConstants[1] = 0.0f;
    m_ColorBlendSCI.blendConstants[2] = 0.0f;
    m_ColorBlendSCI.blendConstants[3] = 0.0f;
}

void FixedPipelineFuncs::SetDynamicSCI() {

    std::vector<VkDynamicState> dynamicStateDescriptors;
    dynamicStateDescriptors.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateDescriptors.push_back(VK_DYNAMIC_STATE_SCISSOR);

    //if (pipelineInfo.depthBiasEnabled)
      //  dynamicStateDescriptors.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

    m_DynamicSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_DynamicSCI.pNext = NULL;
    m_DynamicSCI.dynamicStateCount = uint32_t(dynamicStateDescriptors.size());
    m_DynamicSCI.pDynamicStates = dynamicStateDescriptors.data();
}

// TODO: Make this dynamic to support usage of multiple uniform buffers
void FixedPipelineFuncs::SetPipelineLayout(VkDescriptorSetLayout layout, VkPushConstantRange* pushConstants)
{
    layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCI.setLayoutCount = layout == VK_NULL_HANDLE ? 0 : 1;
    layoutCI.pSetLayouts = &layout;
    if (pushConstants != nullptr) {
        layoutCI.pushConstantRangeCount = 1;
        layoutCI.pPushConstantRanges = pushConstants;
    }

    if(VK_CALL(vkCreatePipelineLayout(VKDEVICE, &layoutCI, nullptr, &m_PipelineLayout)))
        throw std::runtime_error("Cannot create pipeline layout");
    else VK_LOG_SUCCESS("Pipeline layout successfully created!");
}

void FixedPipelineFuncs::DestroyPipelineLayout()
{
    vkDestroyPipelineLayout(VKDEVICE, m_PipelineLayout, nullptr);
}
