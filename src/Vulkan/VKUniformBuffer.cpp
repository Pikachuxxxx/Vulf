#include "VKUniformBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKUniformBuffer::CreateUniformBuffer(uint32_t swapImagesCount)
{
    // Create the binding layout for the descriptor sets to be generated
    CreateDescriptorSetLayout();

    m_UniformBuffers.resize(swapImagesCount);

    for (size_t i = 0; i < swapImagesCount; i++)
        m_UniformBuffers[i].CreateBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Create the pool
    CreatePool();

    // Create the sets from the pool with the layout
    CreateSets();

    // Config the sets to the buffer
    UpdateDescriptorSetConfig();
}

void VKUniformBuffer::CreateDescriptorSetLayout()
{
    // Create the binding layout info
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    // Create the descriptor set layout
    if(VK_CALL(vkCreateDescriptorSetLayout(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &layoutInfo, nullptr, &m_UBODescriptorSetLayout)))
        throw std::runtime_error("Cannot Create Descriptor Set Layout");
    else VK_LOG_SUCCESS("Descriptor Set Layout successuflly created!");
}

void VKUniformBuffer::CreatePool()
{
    // Create the descriptor pool to create the sets using the layouts
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(m_UniformBuffers.size());

    if(VK_CALL(vkCreateDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create the descriptor pool!");
    else VK_LOG_SUCCESS("successuflly create descriptor pool!");
}

void VKUniformBuffer::CreateSets()
{
    std::vector<VkDescriptorSetLayout> layouts(m_UniformBuffers.size(), m_UBODescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(m_UniformBuffers.size());
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(m_UniformBuffers.size());
    if(VK_CALL(vkAllocateDescriptorSets(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &allocInfo, m_DescriptorSets.data())))
        throw std::runtime_error("Coudld not create descriptor sets!!!");
    else VK_LOG_SUCCESS("Descriptor sets successuflly created!");
}

void VKUniformBuffer::UpdateDescriptorSetConfig()
{
    for (size_t i = 0; i < m_UniformBuffers.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}


void VKUniformBuffer::UpdateBuffer(UniformBufferObject buffer, uint32_t index)
{
    void* data;
    vkMapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_UniformBuffers[index].GetBufferMemory(), 0, sizeof(buffer), 0, &data);
    memcpy(data, &buffer, sizeof(buffer));
    vkUnmapMemory(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_UniformBuffers[index].GetBufferMemory());
}

void VKUniformBuffer::Destroy()
{
    vkDestroyDescriptorPool(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_DescriptorPool, nullptr);
    for (size_t i = 0; i < m_UniformBuffers.size(); i++)
        m_UniformBuffers[i].DestroyBuffer();
    vkDestroyDescriptorSetLayout(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_UBODescriptorSetLayout, nullptr);

}
