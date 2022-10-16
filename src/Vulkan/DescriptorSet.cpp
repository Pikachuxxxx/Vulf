#include "DescriptorSet.h"

#include "Device.h"

#include "UniformBuffer.h"
#include "StorageImage.h"

#include "../utils/VulkanCheckResult.h"

uint32_t DescriptorSet::m_DescriptorPoolCurrentAllocations = 0;

void DescriptorSet::Init(std::vector<DescriptorInfo> descriptorInfos)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayouts;
    for (auto& descriptorInfo : descriptorInfos)
    {
        m_DescriptorsInfos.push_back(descriptorInfo);

        // 1. Create the Layout for the descriptor binding info
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = descriptorInfo.bindingID;
        layoutBinding.descriptorType = (VkDescriptorType)descriptorInfo.type;
        layoutBinding.descriptorCount = 1; // If it's an array
        layoutBinding.stageFlags = descriptorInfo.shaderType;

        setLayouts.push_back(layoutBinding);
    }
    // 2. Create the set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayouts.size());
    layoutInfo.pBindings = setLayouts.data();

    // Create the descriptor set layout
    if (VK_CALL(vkCreateDescriptorSetLayout(VKDEVICE, &layoutInfo, nullptr, &m_SetLayout)))
        throw std::runtime_error("Cannot Create Descriptor Set Layout");
    else VK_LOG_SUCCESS("Descriptor Set Layout successuflly created!");

    // 3. Create the sets
    // TODO: Allocate one per frame
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = Device::Get()->get_descriptor_pool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_SetLayout;

    m_DescriptorPoolCurrentAllocations++;

    if (VK_CALL(vkAllocateDescriptorSets(VKDEVICE, &allocInfo, &m_DescriptorSet)))
        throw std::runtime_error("Could not create descriptor sets!!!");
    else VK_LOG_SUCCESS("Descriptor sets successuflly created!");

    update_set();
}

void DescriptorSet::Destroy()
{
v    m_DescriptorPoolCurrentAllocations--;
    m_DescriptorsInfos.clear();
    vkDestroyDescriptorSetLayout(VKDEVICE, m_SetLayout, nullptr);
}

void DescriptorSet::update_set()
{

    std::vector<VkWriteDescriptorSet> descriptorWrites(m_DescriptorsInfos.size());


    for (size_t j = 0; j < m_DescriptorsInfos.size(); j++) {
        switch (m_DescriptorsInfos[j].type) {
        case DescriptorType::UNIFORM_BUFFER:
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_DescriptorsInfos[j].uniformBuffer->get_handle();
            bufferInfo.offset = m_DescriptorsInfos[j].uniformBuffer->get_offset();
            bufferInfo.range = m_DescriptorsInfos[j].uniformBuffer->get_size();

            //-------------------------------------------------

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSet;
            descriptorWrite.dstBinding = m_DescriptorsInfos[j].bindingID;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.pTexelBufferView = nullptr;

            descriptorWrites[j] = descriptorWrite;
            break;
        }
        case DescriptorType::COMBINED_IMAGE_SAMPLER:
        {
            VkDescriptorImageInfo imageBufferInfo{};
            Texture image = *m_DescriptorsInfos[j].image;
            imageBufferInfo.imageLayout = image.get_layout();
            imageBufferInfo.imageView = image.get_view();
            imageBufferInfo.sampler = image.get_sampler();

            //-------------------------------------------------

            VkWriteDescriptorSet descriptorImageWrite{};
            descriptorImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorImageWrite.dstSet = m_DescriptorSet;
            descriptorImageWrite.dstBinding = m_DescriptorsInfos[j].bindingID;
            descriptorImageWrite.dstArrayElement = 0;
            descriptorImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorImageWrite.descriptorCount = 1;
            descriptorImageWrite.pImageInfo = &imageBufferInfo;

            descriptorWrites[j] = descriptorImageWrite;

            break;
        }
        case DescriptorType::STORAGE_IMAGE:
        {
            VkDescriptorImageInfo storageImageInfo{};
            storageImageInfo.imageLayout = m_DescriptorsInfos[j].storageImage->get_image().get_descriptor_info().imageLayout;
            storageImageInfo.imageView = m_DescriptorsInfos[j].storageImage->get_image().get_view();
            storageImageInfo.sampler = m_DescriptorsInfos[j].storageImage->get_image().get_sampler();

            //--------------------------------------------

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSet;
            descriptorWrite.dstBinding = m_DescriptorsInfos[j].bindingID;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &storageImageInfo;

            descriptorWrites[j] = descriptorWrite;
            break;
        }
        }
    }
    vkUpdateDescriptorSets(VKDEVICE, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}