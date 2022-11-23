#pragma once

#include <vulkan/vulkan.h>
#include <vector>

enum class Topology
{
    POINTS, LINES, TRIANGLES, LINE_STRIP
};

class FixedPipelineFuncs
{
public:
    FixedPipelineFuncs() = default;
    void SetFixedPipelineStage(VkPrimitiveTopology topology, const VkExtent2D& swapchainExtent, bool enableWireFrameMode);
    VkPipelineVertexInputStateCreateInfo& GetVertexInputSCI() { return m_VertexInputSCI; }
    VkPipelineInputAssemblyStateCreateInfo& GetInputAssemblySCI() { return m_InputAssemblySCI; }
    VkPipelineViewportStateCreateInfo& GetViewportStateCI() { return m_ViewportSCI; }
    VkPipelineRasterizationStateCreateInfo& GetRazterizerSCI() { return m_RasterizerSCI; }
    VkPipelineMultisampleStateCreateInfo& GetMultiSampleSCI() { return m_MultiSampleSCI; }
    VkPipelineDepthStencilStateCreateInfo& GetDepthStencilSCI() { return m_DepthStencilSCI;}
    VkPipelineColorBlendStateCreateInfo& GetColorBlendSCI() { return m_ColorBlendSCI; }
    VkPipelineDynamicStateCreateInfo& GetDynmaicSCI() { return m_DynamicSCI; }
    VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }
    void SetVertexInputSCI();
    void SetInputAssemblyStageInfo(VkPrimitiveTopology topology);
    void SetViewportSCI(const VkExtent2D& swapchainExtent);
    void SetRasterizerSCI(int cullMode = VK_CULL_MODE_BACK_BIT, bool enableWireFrameMode = false);
    void SetMultiSampleSCI();
    void SetDepthStencilSCI(int depthCompareOp = VK_COMPARE_OP_LESS);
    void SetColorBlendSCI();
    void SetDynamicSCI();
    void SetPipelineLayout(VkDescriptorSetLayout layout, VkPushConstantRange* pushConstants);
    void DestroyPipelineLayout();
private:
    VkPipelineVertexInputStateCreateInfo m_VertexInputSCI;
    VkPipelineInputAssemblyStateCreateInfo m_InputAssemblySCI;
    VkPipelineViewportStateCreateInfo m_ViewportSCI;
    VkPipelineRasterizationStateCreateInfo m_RasterizerSCI;
    VkPipelineMultisampleStateCreateInfo m_MultiSampleSCI;
    VkPipelineDepthStencilStateCreateInfo m_DepthStencilSCI;
    VkPipelineColorBlendStateCreateInfo m_ColorBlendSCI;
    VkPipelineDynamicStateCreateInfo m_DynamicSCI;
    VkPipelineLayout m_PipelineLayout;
    VkViewport viewport;
    VkRect2D scissor;
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineLayoutCreateInfo layoutCI{};
};
