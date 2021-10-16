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
    const VkPipelineVertexInputStateCreateInfo& GetVertexInputSCI() { return m_VertexInputSCI; }
    const VkPipelineInputAssemblyStateCreateInfo& GetInputAssemblySCI() { return m_InputAssemblySCI; }
    const VkPipelineViewportStateCreateInfo& GetViewportStateCI() { return m_ViewportSCI; }
    const VkPipelineRasterizationStateCreateInfo& GetRazterizerSCI() { return m_RasterizerSCI; }
    const VkPipelineMultisampleStateCreateInfo& GetMultiSampleSCI() { return m_MultiSampleSCI; }
    const VkPipelineDepthStencilStateCreateInfo& GetDepthStencilSCI() { return m_DepthStencilSCI;}
    const VkPipelineColorBlendStateCreateInfo& GetColorBlendSCI() { return m_ColorBlendSCI; }
    const VkPipelineDynamicStateCreateInfo& GetDynmaicSCI() { return m_DynamicSCI; }
    const VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }
    void SetVertexInputSCI();
    void SetInputAssemblyStageInfo(VkPrimitiveTopology topology);
    void SetViewportSCI(const VkExtent2D& swapchainExtent);
    void SetRasterizerSCI(bool enableWireFrameMode);
    void SetMultiSampleSCI();
    void SetDepthStencilSCI();
    void SetColorBlendSCI();
    void SetDynamicSCI();
    void SetPipelineLayout(VkDescriptorSetLayout layout, VkPushConstantRange& pushConstants);
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
