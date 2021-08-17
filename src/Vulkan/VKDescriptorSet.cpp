#include "VKDescriptorSet.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKDescriptorSet::CreateSets(uint32_t size, VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VKBuffer> uniformBuffers, uint32_t uboSize)
{
    std::vector<VkDescriptorSetLayout> layouts(size, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(size);
    if (VK_CALL(vkAllocateDescriptorSets(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()))) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < size; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(uboSize);


        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

    }
}
