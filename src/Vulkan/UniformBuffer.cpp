#include "UniformBuffer.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

void UniformBuffer::Init(uint32_t bufferSize)
{
    m_Size = bufferSize;
    m_Offset = 0;
    m_UniformBuffer.Init(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

}

void UniformBuffer::Destroy()
{
    m_UniformBuffer.Destroy();
}

void UniformBuffer::update_buffer(void* buffer, uint32_t bufferSize) {
    void* data;
    vkMapMemory(VKDEVICE, m_UniformBuffer.get_memory(), 0, bufferSize, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(VKDEVICE, m_UniformBuffer.get_memory());
}