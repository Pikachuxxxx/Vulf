#include "VKInstance.h"

#include "../utils/prettytable.h"
#include "../utils/VulkanCheckResult.h"

VKInstance* VKInstance::s_Instance;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

void VKInstance::Init(std::string appname, GLFWwindow* window, bool enableValidationLayers /*= true*/)
{
    if(s_Instance ==  nullptr)
        s_Instance = new VKInstance();
    m_EnableValidationLayers = enableValidationLayers;

    VkApplicationInfo appCI{};
    appCI.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appCI.pNext = nullptr;
    appCI.pApplicationName = appname.c_str();
    appCI.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appCI.pEngineName = "Vulf";
    appCI.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appCI.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCI{};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    debugCI = {};
    debugCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCI.pfnUserCallback = DebugCallback;

    instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCI;
    instanceCI.pApplicationInfo = &appCI;

    if(enableValidationLayers && checkValidationLayersSupport())
    {
        instanceCI.enabledLayerCount = validationLayers.size();
        instanceCI.ppEnabledLayerNames = validationLayers.data();
    }
    auto extensions = getRequiredExtensions();
    instanceCI.enabledExtensionCount = extensions.size();
    instanceCI.ppEnabledExtensionNames = extensions.data();

    if(VK_CALL(vkCreateInstance(&instanceCI, nullptr, &m_VkInstance)))
        throw std::runtime_error("Failed to create Instance!");
    else
        VK_LOG_SUCCESS("Instance was succesfully created!")

    // Now create the debug messenger
    SetupDebugCallback();
    // Create the surface
    CreateSurface(window);
}

void VKInstance::Destroy()
{
    vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
    DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessengerHandle, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
}

bool VKInstance::checkValidationLayersSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layer : validationLayers)
    {
        bool layerFound = false;
        for (const auto& availLayer : availableLayers)
        {
            if(strcmp(layer, availLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if(!layerFound)
            return false;
    }

    // Now print the layers only for debug purposes
#ifndef NDEBUG
    PrettyTable<std::string, std::string, std::string> vt({"Available Layers", "Requested Layers", "Found"});
    for (uint32_t i = 0; i < layerCount; i++)
    {
        for(uint32_t j = 0; j < validationLayers.size(); j++)
        {
            if(strcmp(availableLayers[i].layerName, validationLayers[j]) == 0)
            {
                vt.addRow(availableLayers[i].layerName, validationLayers[j], "Yes");
            }
            else
            {
                vt.addRow(availableLayers[i].layerName, "", "");
                break;
            }
        }
    }
    vt.print(std::cout);
#endif

    return true;
}
// retrieves the list of extensions to be loaded
std::vector<const char*> VKInstance::getRequiredExtensions()
{
    // First get the extensions needed by GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (int i = 0; i < glfwExtensionCount; ++i)
        std::cout << glfwExtensions[i] << "\n";

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    // Add extensions for validation layers
    // if (m_EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // }
    // Add any user defined extensions
    extensions.insert(extensions.end(), instanceExtensions.begin(), instanceExtensions.end());
    return extensions;
}

void VKInstance::SetupDebugCallback()
{
    if(VK_CALL(CreateDebugUtilsMessengerEXT(m_VkInstance, &debugCI, nullptr, &m_DebugMessengerHandle)))
        throw std::runtime_error("Failed to create debug messenger!");
    else
        VK_LOG_SUCCESS("Debug messenger succesfully created!")
}

VKAPI_ATTR VkBool32 VKAPI_CALL VKInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data)
{
    // Select prefix depending on flags passed to the callback
    // Note that multiple flags may be set for a single validation message
    // Error that may result in undefined behaviour
    // TODO: Add option to choose minimum severity level and use <=> to select levels
    // TODO: Formate the message id and stuff for colors etc

    if(!message_severity)
        return VK_FALSE;

    if(message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cout << "\033[1;31m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;31m - Validation ERROR : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage  << std::endl;
        std::cout << "\033[1;31m *****************************************************************" << std::endl;
    };
    // Warnings may hint at unexpected / non-spec API usage
    if(message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cout << "\033[1;33m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;33m - Validation WARNING : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage  << std::endl;
        std::cout << "\033[1;33m *****************************************************************" << std::endl;
    };
    // Informal messages that may become handy during debugging
    if(message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        std::cout << "\033[1;36m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;36m - Validation INFO : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage  << std::endl;
        std::cout << "\033[1;36m *****************************************************************" << std::endl;
    }
    // Diagnostic info from the Vulkan loader and layers
    // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
    // if(message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    // {
    //     std::cout << "\033[1;35m*****************************************************************" << std::endl;
    //     std::cout << "\033[1;32m[VULKAN] \033[1;35m - DEBUG : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage  << std::endl;
    //     std::cout << "\033[1;35m*****************************************************************" << std::endl;
    // }

    return VK_FALSE;
}

void VKInstance::CreateSurface(GLFWwindow* window)
{
    if(VK_CALL(glfwCreateWindowSurface(m_VkInstance, window, nullptr, &m_Surface)))
        throw std::runtime_error("Cannot create surface!");
    else
        VK_LOG_SUCCESS("Window surface succesfully created!");
}
