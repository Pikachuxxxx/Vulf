#include "VKDescriptorSetLayout.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

// TODO: Make this more generic for the stage flags and descriptorCount
void VKDescriptorSetLayout::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (VK_CALL(vkCreateDescriptorSetLayout(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &layoutInfo, nullptr, &m_DesciptorLayout))) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VKDescriptorSetLayout::Destroy()
{
    vkDestroyDescriptorSetLayout(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_DesciptorLayout, nullptr);
}
