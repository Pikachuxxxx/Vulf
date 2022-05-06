#pragma once

// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

extern std::vector<const char*> g_ValidationLayers;
extern std::vector<const char*> g_InstanceExtensions;

// TODO: Get all the supported extension in each instance layer
// TODO: Macros for Debug markers(name based tagging for all vulkan resources namely buffers and textures) and API markers(single vs region)
// TODO: Macros for loading functions dynamically

class Instance
{
public:
    Instance() = default;
    ~Instance() {}

    void Init(std::string appname, GLFWwindow* window, bool enableValidationLayers = true);
    void Destroy();

    static Instance* Get() { if(s_Instance ==  nullptr) s_Instance = new Instance; return s_Instance; }

    inline VkInstance& get_handle() { return m_VkInstance; }
    inline VkSurfaceKHR& get_surface() { return m_Surface; }

private:
    static Instance*                    s_Instance;

    VkInstance                          m_VkInstance;
    bool                                m_EnableValidationLayers;
    VkDebugUtilsMessengerEXT            m_DebugMessengerHandle;
    VkDebugUtilsMessengerCreateInfoEXT  m_DebugUtilsCI;
    VkSurfaceKHR                        m_Surface;

private:
    // Check whether or not the validation layers are supported
    bool check_validation_layers_support();
    // retrieves the list of extensions to be loaded
    std::vector<const char*> get_required_extensions();
    // Setup the deubug callback function
    void setup_debug_callback();
    // Create the surface after making sure the WSI instance extension is supported
    void create_surface(GLFWwindow* window);

    // Setup the debug messenger and load the symbols necessary to instantiate it
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);
};
