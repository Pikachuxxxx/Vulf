#include "UniformBuffer.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

void UniformBuffer::CreateUniformBuffer(uint32_t swapImagesCount, uint32_t bufferSize)
{
    // First create the descriptor sets manually by adding them
    CreateDescriptorSetLayout();

    m_UniformBuffers.resize(swapImagesCount);

    for (size_t i = 0; i < swapImagesCount; i++)
        m_UniformBuffers[i].Init(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Create the pool
    CreatePool();

    // Create the sets from the pool with the layout
    CreateSets();

    // Manually update the descriptor configs by adding them
    UpdateDescriptorSetConfig();
}

void UniformBuffer::AddDescriptor(DescriptorInfo setInfo) {

    // First create the layout binding
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding            = setInfo.bindingID;
    uboLayoutBinding.descriptorType     = (VkDescriptorType)setInfo.type;
    uboLayoutBinding.descriptorCount    = 1;
    uboLayoutBinding.stageFlags         = setInfo.shaderType;

    m_LayoutBindings.push_back(uboLayoutBinding);

    m_Descriptors.push_back(setInfo);

}

void UniformBuffer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(m_LayoutBindings.size());
    layoutInfo.pBindings    = m_LayoutBindings.data();

    // Create the descriptor set layout
    if(VK_CALL(vkCreateDescriptorSetLayout(VKDEVICE, &layoutInfo, nullptr, &m_UBODescriptorSetLayout)))
        throw std::runtime_error("Cannot Create Descriptor Set Layout");
    else VK_LOG_SUCCESS("Descriptor Set Layout successuflly created!");
}

void UniformBuffer::CreatePool()
{
    // Create the descriptor pool to create the sets using the layouts
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (size_t i = 0; i < m_Descriptors.size(); i++) {
        VkDescriptorPoolSize poolSize;
        poolSize.type = (VkDescriptorType) m_Descriptors[i].type;
        poolSize.descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());

        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_UniformBuffers.size());

    if(VK_CALL(vkCreateDescriptorPool(VKDEVICE, &poolInfo, nullptr, &m_DescriptorPool)))
        throw std::runtime_error("Cannot create the descriptor pool!");
    else VK_LOG_SUCCESS("successuflly create descriptor pool!");
}

void UniformBuffer::CreateSets()
{
    std::vector<VkDescriptorSetLayout> layouts(m_UniformBuffers.size(), m_UBODescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = m_DescriptorPool;
    allocInfo.descriptorSetCount    = static_cast<uint32_t>(m_UniformBuffers.size());
    allocInfo.pSetLayouts           = layouts.data();

    m_DescriptorSets.clear();
    m_DescriptorSets.resize(m_UniformBuffers.size());

    if(VK_CALL(vkAllocateDescriptorSets(VKDEVICE, &allocInfo, m_DescriptorSets.data())))
        throw std::runtime_error("Could not create descriptor sets!!!");
    else VK_LOG_SUCCESS("Descriptor sets successuflly created!");
}

void UniformBuffer::UpdateDescriptorSetConfig( )
{
    for (size_t i = 0; i < m_UniformBuffers.size(); i++) {

        std::vector<VkDescriptorBufferInfo> m_VkDescriptorBufferInfos;
        std::vector<VkDescriptorImageInfo> m_VkDescriptorImageInfos;

        for (size_t j = 0; j < m_Descriptors.size(); j++) {
            switch (m_Descriptors[j].type) {
                case DescriptorInfo::DescriptorType::BUFFER:
                {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer   = m_UniformBuffers[i].get_handle();
                    bufferInfo.offset   = m_Descriptors[j].offset;
                    bufferInfo.range    = m_Descriptors[j].size;
                    m_VkDescriptorBufferInfos.push_back(bufferInfo);
                    break;
                }
                case DescriptorInfo::DescriptorType::IMAGE:
                {
                    VkDescriptorImageInfo bufferInfo{};
                    bufferInfo.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    bufferInfo.imageView    = m_Descriptors[j].image.get_image_view();
                    bufferInfo.sampler      = m_Descriptors[j].image.get_sampler();
                    m_VkDescriptorImageInfos.push_back(bufferInfo);
                    break;
                }
            }
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        int buffCount = 0;
        for (size_t k = 0; k < m_Descriptors.size(); k++) {
            switch (m_Descriptors[k].type) {
                case DescriptorInfo::DescriptorType::BUFFER:
                {
                    VkWriteDescriptorSet descriptorWrite{};
                    descriptorWrite.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet              = m_DescriptorSets[i];
                    descriptorWrite.dstBinding          = m_Descriptors[k].bindingID;
                    descriptorWrite.dstArrayElement     = 0;
                    descriptorWrite.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.descriptorCount     = 1;
                    descriptorWrite.pBufferInfo         = &m_VkDescriptorBufferInfos[k];
                    descriptorWrite.pImageInfo          = nullptr;
                    descriptorWrite.pTexelBufferView    = nullptr;

                    descriptorWrites.push_back(descriptorWrite);
                    buffCount++;
                    break;
                }
                case DescriptorInfo::DescriptorType::IMAGE:
                {
                    VkWriteDescriptorSet descriptorWrite{};
                    descriptorWrite.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet              = m_DescriptorSets[i];
                    descriptorWrite.dstBinding          = m_Descriptors[k].bindingID;
                    descriptorWrite.dstArrayElement     = 0;
                    descriptorWrite.descriptorType      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount     = 1;

                    descriptorWrite.pImageInfo          = &m_VkDescriptorImageInfos[k - buffCount];

                    descriptorWrites.push_back(descriptorWrite);
                    break;
                }
            }
        }

        vkUpdateDescriptorSets(VKDEVICE, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void UniformBuffer::UpdateBuffer(void* buffer, uint32_t bufferSize, uint32_t index) {
    void* data;
    vkMapMemory(VKDEVICE, m_UniformBuffers[index].get_memory(), 0, bufferSize, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(VKDEVICE, m_UniformBuffers[index].get_memory());
}

void UniformBuffer::Destroy()
{
    m_LayoutBindings.clear();
    m_Descriptors.clear();
    m_UniformBuffers.clear();
    m_DescriptorSets.clear();
    vkDestroyDescriptorPool(VKDEVICE, m_DescriptorPool, nullptr);
    for (size_t i = 0; i < m_UniformBuffers.size(); i++)
        m_UniformBuffers[i].Destroy();
    vkDestroyDescriptorSetLayout(VKDEVICE, m_UBODescriptorSetLayout, nullptr);

}
