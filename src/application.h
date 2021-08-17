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

#include "vertex.h"

// Helper includes
#include "utils/Window.h"
#include "utils/Camera3D.h"

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
#include "Vulkan/VKVertexBuffer.h"
#include "Vulkan/VKIndexBuffer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;


class Application
{
public:
    void Run();
private:
    Window* window;
    Camera3D camera;
    bool framebufferResized = false;
    double lastTime;
    double nbFrames = 0;
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

VKVertexBuffer triVBO;
VKIndexBuffer triIBO;

VKVertexBuffer quadVBO;
VKIndexBuffer quadIBO;
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
