// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLM_DEPTH_ZERO_TO_ONE

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
#include "Vulkan/VKUniformBuffer.h"

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

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
    double delta = 0;
/***************************** Vulkan Variables *******************************/
private:
/****************************** Application Flow ******************************/
    void InitWindow();
    void InitVulkan();
    void InitImGui();
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

// Descriptor and uniforms shit!
VKUniformBuffer mvpUniformBuffer;
/******************************* Vulkan Variables *****************************/
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderingFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;
/******************************************************************************/
void RecreateSwapchain();
void RecordCommandLists();
void CleanUpCommandListResources();
void UpdateMVPUBO(uint32_t currentImageIndex);
/******************************* ImGui Callbacks *******************************/
static void ImGuiError(VkResult err);
};
