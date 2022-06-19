#include "Semaphore.h"

#include "VulkanCheckResult.h"
#include "Device.h"

void Semaphore::Init()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore.flags = 0;

    if(VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &m_Semaphore)))
        throw std::runtime_error("Cannot create semaphore!");
}

void Semaphore::Destroy()
{
    vkDestroySemaphore(VK_DEVICE, m_Semaphore, nullptr);
}

void Semaphore::signal()
{

}
