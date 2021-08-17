#include "VKDescriptorPool.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKDescriptorPool::CreatePool(uint32_t size)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(size);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(size);

    if(VK_CALL(vkCreateDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create descriptor pool");
}

void VKDescriptorPool::Destroy()
{
    vkDestroyDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_DescriptorPool, nullptr);
}
