#include "Semaphore.h"

#include "utils/VulkanCheckResult.h"
#include "Device.h"

void Semaphore::Init(std::string name)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    if(VK_CALL(vkCreateSemaphore(VKDEVICE, &semaphoreInfo, nullptr, &m_Semaphore)))
        throw std::runtime_error("Cannot create semaphore!");
    else
        VK_LOG("Semaphore successfully created");

    // Tag semaphores
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    nameInfo.objectHandle = (uint64_t)m_Semaphore;
    nameInfo.pObjectName = name.c_str();

    if (VK_CALL(CreateDebugObjName(&nameInfo)))
        throw std::runtime_error("cannot tag semaphore");
}

void Semaphore::Destroy()
{
    vkDestroySemaphore(VKDEVICE, m_Semaphore, nullptr);
}

void Semaphore::signal()
{

}
