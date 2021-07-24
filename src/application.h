// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Std. Libraries
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <vector>
#include <set>
#include <cstdint>
#include <algorithm>

// Helper includes
// #include "utils/prettytable.h"
// #include "utils/VulkanCheckResult.h"

#include "Vulkan/VKInstance.h"
#include "Vulkan/VKDevice.h"
#include "Vulkan/VKSwapchain.h"

class Application
{
public:
    void Run();
private:
    GLFWwindow* window;
/***************************** Vulkan Variables *******************************/
private:
/**************************** Application Flow ********************************/
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void DrawFrame();
    void CleanUp();
/**************************** Vulkan Initialization ***************************/
VKSwapchain swapchainManager;
/******************************* Vulkan Helper ********************************/


};
