#include "Semaphore.h"

#include "utils/VulkanCheckResult.h"
#include "Device.h"

void Semaphore::Init()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    if(VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &m_Semaphore)))
        throw std::runtime_error("Cannot create semaphore!");
    else
        VK_LOG("Semaphore successfully created");
}

void Semaphore::Destroy()
{
    vkDestroySemaphore(VKDEVICE, m_Semaphore, nullptr);
}

void Semaphore::signal()
{

}
