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
#include "Vulkan/VKShader.h"
#include "Vulkan/VKFixedPipelineFuncs.h"
#include "Vulkan/VKRenderPass.h"
#include "Vulkan/VKGraphicsPipeline.h"
#include "Vulkan/VKFramebuffer.h"
#include "Vulkan/VKCmdPool.h"
#include "Vulkan/VKCmdBuffer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

class Application
{
public:
    void Run();
private:
    GLFWwindow* window;
    bool framebufferResized = false;
/***************************** Vulkan Variables *******************************/
private:
/****************************** Application Flow ******************************/
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void DrawFrame();
    void CleanUp();
/***************************** Vulkan Encapsulation ***************************/
VKSwapchain swapchainManager;
VKShader vertexShader;
VKShader fragmentShader;
VKFixedPipelineFuncs fixedPipelineFuncs;
VKRenderPass renderPassManager;
VKGraphicsPipeline graphicsPipeline;
VKFramebuffer framebufferManager;
VKCmdPool cmdPoolManager;
VKCmdBuffer swapCmdBuffers;
/******************************* Vulkan Variables *****************************/
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderingFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;
/******************************************************************************/
void RecreateSwapchain();
/******************************* GLFW Callbacks *******************************/
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void resize_callback(GLFWwindow* window, int width, int height);
};
