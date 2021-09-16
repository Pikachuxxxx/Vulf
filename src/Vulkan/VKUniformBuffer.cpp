#include "VKUniformBuffer.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKUniformBuffer::CreateUniformBuffer(uint32_t swapImagesCount, VKTexture& texture)
{
    m_Texture = texture;
    // Create the binding layout for the descriptor sets to be generated
    CreateDescriptorSetLayout();

    m_UniformBuffers.resize(swapImagesCount);

    for (size_t i = 0; i < swapImagesCount; i++)
        m_UniformBuffers[i].CreateBuffer((sizeof(UniformBufferObject) + sizeof(LightUniformBufferObject)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Create the pool
    CreatePool();

    // Create the sets from the pool with the layout
    CreateSets();

    // Config the sets to the buffer
    UpdateDescriptorSetConfig();

    // m_Set.CreateSets()
}

void VKUniformBuffer::CreateDescriptorSetLayout()
{
    // Create the binding layout info
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Texture layouts
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Lighting data
    VkDescriptorSetLayoutBinding lightLayoutBinding{};
    lightLayoutBinding.binding = 2;
    lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightLayoutBinding.descriptorCount = 1;
    lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // Create the descriptor set layout
    if(VK_CALL(vkCreateDescriptorSetLayout(VKDEVICE, &layoutInfo, nullptr, &m_UBODescriptorSetLayout)))
        throw std::runtime_error("Cannot Create Descriptor Set Layout");
    else VK_LOG_SUCCESS("Descriptor Set Layout successuflly created!");
}

void VKUniformBuffer::CreatePool()
{
    // Create the descriptor pool to create the sets using the layouts
    // VkDescriptorPoolSize poolSize{};
    // poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // poolSize.descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(m_UniformBuffers.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_UniformBuffers.size());

    if(VK_CALL(vkCreateDescriptorPool(VKDEVICE, &poolInfo, nullptr, &m_DescriptorPool)))
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
    if(VK_CALL(vkAllocateDescriptorSets(VKDEVICE, &allocInfo, m_DescriptorSets.data())))
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

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_Texture.GetTextureImageView();
        imageInfo.sampler = m_Texture.GetTextureImageSampler();

        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
        lightBufferInfo.offset = sizeof(UniformBufferObject);
        lightBufferInfo.range = sizeof(LightUniformBufferObject);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        // VkWriteDescriptorSet descriptorWrite{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr; // Optional
        descriptorWrites[0].pTexelBufferView = nullptr; // Optional

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_DescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_DescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &lightBufferInfo;
        descriptorWrites[2].pImageInfo = nullptr; // Optional
        descriptorWrites[2].pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(VKDEVICE, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}


void VKUniformBuffer::UpdateBuffer(UniformBufferObject buffer, uint32_t index)
{
    void* data;
    vkMapMemory(VKDEVICE, m_UniformBuffers[index].GetBufferMemory(), 0, sizeof(UniformBufferObject), 0, &data);
    memcpy(data, &buffer, sizeof(buffer));
    vkUnmapMemory(VKDEVICE, m_UniformBuffers[index].GetBufferMemory());
}

void VKUniformBuffer::UpdateLightBuffer(LightUniformBufferObject buffer, uint32_t index)
{
    void* data;
    vkMapMemory(VKDEVICE, m_UniformBuffers[index].GetBufferMemory(), sizeof(UniformBufferObject), sizeof(LightUniformBufferObject), 0, &data);
    memcpy(data, &buffer, sizeof(buffer));
    vkUnmapMemory(VKDEVICE, m_UniformBuffers[index].GetBufferMemory());
}

void VKUniformBuffer::Destroy()
{
    // m_Texture.Destroy();
    vkDestroyDescriptorPool(VKDEVICE, m_DescriptorPool, nullptr);
    for (size_t i = 0; i < m_UniformBuffers.size(); i++)
        m_UniformBuffers[i].DestroyBuffer();
    vkDestroyDescriptorSetLayout(VKDEVICE, m_UBODescriptorSetLayout, nullptr);

}