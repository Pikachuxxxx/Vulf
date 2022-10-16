#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "DescriptorSet.h"

class UniformBuffer
{

public:
    UniformBuffer() = default;

    void Init(uint32_t bufferSize);
    void Destroy();

    void update_buffer(void* buffer, uint32_t bufferSize);

    VkBuffer get_handle() { return m_UniformBuffer.get_handle(); }
    uint32_t get_size() { return m_Size; }
    uint32_t get_offset() { return m_Offset; }

private:
    Buffer      m_UniformBuffer;
    uint32_t    m_Size;
    uint32_t    m_Offset;
};

//template<>
//void UniformBuffer::AddDescriptorSetConfig<UniformBuffer::DescriptorBufferInfo>(UniformBuffer::DescriptorBufferInfo setConfig) {
//    m_BufferInfos.push_back(setConfig);
//}
//
//template<>
//void UniformBuffer::AddDescriptorSetConfig<UniformBuffer::DescriptorImageInfo>(UniformBuffer::DescriptorImageInfo setConfig) {
//    m_ImageInfos.push_back(setConfig);
//}
