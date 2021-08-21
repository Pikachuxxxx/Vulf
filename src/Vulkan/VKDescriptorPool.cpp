#include "VKDescriptorPool.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKDescriptorPool::CreatePool(uint32_t size)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(size);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(size);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(size);

    if(VK_CALL(vkCreateDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create descriptor pool");
}

void VKDescriptorPool::Destroy()
{
    vkDestroyDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_DescriptorPool, nullptr);
}
