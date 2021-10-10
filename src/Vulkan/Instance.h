#pragma once

// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

extern std::vector<const char*> validationLayers;
extern std::vector<const char*> instanceExtensions;

class Instance
{
public:
    Instance() = default;
    void Init(std::string appname, GLFWwindow* window, bool enableValidationLayers = true);
    void Destroy();
    static Instance* GetInstanceManager() { if(s_Instance ==  nullptr) s_Instance = new Instance; return s_Instance; }
    VkInstance& GetInstance() { return m_VkInstance; }
    VkSurfaceKHR& GetSurface() { return m_Surface; }
private:
    VkInstance m_VkInstance;
    VkDebugUtilsMessengerEXT m_DebugMessengerHandle;
    bool m_EnableValidationLayers;
    VkDebugUtilsMessengerCreateInfoEXT debugCI;
    VkSurfaceKHR m_Surface;
    static Instance* s_Instance;
private:
    // Check whether or not the validation layers are supported
    bool checkValidationLayersSupport();
    // retrieves the list of extensions to be loaded
    std::vector<const char*> getRequiredExtensions();
    // Setup the deubug callback function
    void SetupDebugCallback();
    // Setup the debug messenger and load the symbols necessary to instantiate it
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data);
    // Create the surface after making sure the WSI instance extension is supported
    void CreateSurface(GLFWwindow* window);
};
