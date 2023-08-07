#include "Instance.h"

#include "../utils/prettytable.h"
#include "../utils/VulkanCheckResult.h"

// TODO: Move these 2 functions to somewhere we load dynamic vulkan stuff or use reusbale macros for loading vulkan functions
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

/******************************************************************************/
/*                              Instance class                                */
/******************************************************************************/

//------------------------------------------------------------------------------
Instance* Instance::s_Instance;
//------------------------------------------------------------------------------

void Instance::Init(std::string appname, GLFWwindow* window, bool enablegValidationLayers /*= true*/)
{
    if (s_Instance == nullptr)
        s_Instance = new Instance();

    m_EnableValidationLayers = enablegValidationLayers;

    VkApplicationInfo appCI{};
    appCI.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appCI.pNext = nullptr;
    appCI.pApplicationName = appname.c_str();
    appCI.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appCI.pEngineName = "Vulf Renderer";
    appCI.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appCI.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCI{};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    m_DebugUtilsCI = {};
    m_DebugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    m_DebugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    m_DebugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    m_DebugUtilsCI.pfnUserCallback = DebugCallback;

    instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&m_DebugUtilsCI;
    instanceCI.pApplicationInfo = &appCI;

    if (enablegValidationLayers && check_validation_layers_support())
    {
        instanceCI.enabledLayerCount = g_ValidationLayers.size();
        instanceCI.ppEnabledLayerNames = g_ValidationLayers.data();
    }
    auto extensions = get_required_extensions();
    instanceCI.enabledExtensionCount = extensions.size();
    instanceCI.ppEnabledExtensionNames = extensions.data();

    if (VK_CALL(vkCreateInstance(&instanceCI, nullptr, &m_VkInstance)))
        throw std::runtime_error("Failed to create Instance!");
    else
        VK_LOG_SUCCESS("Instance was succesfully created!")

        // Now create the debug messenger
        setup_debug_callback();
    // Create the surface
    create_surface(window);
}

void Instance::Destroy()
{
    vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
    DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessengerHandle, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
}

bool Instance::check_validation_layers_support()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layer : g_ValidationLayers)
    {
        bool layerFound = false;
        for (const auto& availLayer : availableLayers)
        {
            if (strcmp(layer, availLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }

    // Now print the layers only for debug purposes
#ifndef NDEBUG
    PrettyTable<std::string, std::string, std::string> vt({ "Available Layers", "Requested Layers", "Found" });
    for (uint32_t i = 0; i < layerCount; i++)
    {
        for (uint32_t j = 0; j < g_ValidationLayers.size(); j++)
        {
            if (strcmp(availableLayers[i].layerName, g_ValidationLayers[j]) == 0)
            {
                vt.addRow(availableLayers[i].layerName, g_ValidationLayers[j], "Yes");
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
std::vector<const char*> Instance::get_required_extensions()
{
    // First get the extensions needed by GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VK_LOG("GLFW Required Instance Extensions list :");
    for (int i = 0; i < glfwExtensionCount; ++i)
        VK_LOG("\t", i, ". ", glfwExtensions[i]);
    // std::cout << i << ". " << glfwExtensions[i] << "\n";
// VK_LOG("-----------------------------------------------");

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    // Add extensions for validation layers
    // if (m_EnableValidationLayers) {
    //     extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // }
    // Add any user defined extensions
    extensions.insert(extensions.end(), g_InstanceExtensions.begin(), g_InstanceExtensions.end());
    return extensions;
}

void Instance::setup_debug_callback()
{
    if (VK_CALL(CreateDebugUtilsMessengerEXT(m_VkInstance, &m_DebugUtilsCI, nullptr, &m_DebugMessengerHandle)))
        throw std::runtime_error("Failed to create debug messenger!");
    else
        VK_LOG_SUCCESS("Debug messenger succesfully created!")
}

void Instance::create_surface(GLFWwindow* window)
{
    if (VK_CALL(glfwCreateWindowSurface(m_VkInstance, window, nullptr, &m_Surface)))
        throw std::runtime_error("Cannot create surface!");
    else
        VK_LOG_SUCCESS("Window surface succesfully created!");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    return VK_FALSE;
    // Select prefix depending on flags passed to the callback
    // Note that multiple flags may be set for a single validation message
    // Error that may result in undefined behavior
    // TODO: Add option to choose minimum severity level and use <=> to select levels
    // TODO: Formate the message id and stuff for colors etc

    //std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;

    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cout << "\033[1;31m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;31m - Validation ERROR : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage << std::endl;
        if (callback_data->pObjects->pObjectName != nullptr) {
            auto objName = callback_data->pObjects->pObjectName;
            std::cout << "\nObJect Name : " << std::string(objName) << std::endl;
        }
        std::cout << "\033[1;31m *****************************************************************" << std::endl;
    };
    // Warnings may hint at unexpected / non-spec API usage
    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cout << "\033[1;33m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;33m - Validation WARNING : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage << "\nObJect Name : " << callback_data->pObjects->pObjectName << std::endl;
        std::cout << "\033[1;33m *****************************************************************" << std::endl;

    };
    // Informal messages that may become handy during debugging
    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        std::cout << "\033[1;36m *****************************************************************" << std::endl;
        std::cout << "\033[1;32m[VULKAN] \033[1;36m - Validation INFO : \033[0m \nmessage ID : " << callback_data->messageIdNumber << "\nID Name : " << callback_data->pMessageIdName << "\nMessage : " << callback_data->pMessage << std::endl;
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
