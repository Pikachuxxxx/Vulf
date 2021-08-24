// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLM_DEPTH_ZERO_TO_ONE

// Std. Libraries
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

#include "vertex.h"

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

// Helper includes
#include "utils/Window.h"
#include "utils/Camera3D.h"
#include "utils/Transform.h"
#include "utils/cube.h"

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
#include "Vulkan/VKTexture.h"

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
// ImGuizmo
#include <ImGuizmo/ImGuizmo.h>


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
    void OnImGui();
    void OnUpdate(double dt);
    void LoadModel(std::string path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
/***************************** Vulkan Encapsulation ***************************/
VKSwapchain swapchainManager;
VKShader vertexShader;
VKShader fragmentShader;
VKFixedPipelineFuncs fixedPipelineFuncs;
VKFixedPipelineFuncs wireframeFixedPipelineFuncs;
VKRenderPass renderPassManager;
VKGraphicsPipeline graphicsPipeline;
VKGraphicsPipeline wireframeGraphicsPipeline;
VKFramebuffer framebufferManager;
VKCmdPool cmdPoolManager;
VKCmdBuffer swapCmdBuffers;

VKVertexBuffer triVBO;
VKIndexBuffer triIBO;

VKVertexBuffer quadVBO;
VKIndexBuffer quadIBO;

// Happy budda model
std::vector<Vertex> buddaVertices;
std::vector<uint16_t> buddaIndices;
VKVertexBuffer buddaVBO;
VKIndexBuffer buddaIBO;

VKVertexBuffer cubeVBO;

// Descriptor and uniforms shit!
VKUniformBuffer mvpUniformBuffer;

// Imgui Render pass
VkRenderPass imguiRenderPass;
// VKCmdPool imguiCmdPool;
// ImGui Descriptor pool
VkDescriptorPool imguiDescriptorPool;
VkCommandBuffer imguiCmdBuffer;
VKCmdBuffer imguiCmdBuffers;
float clearColor[4] = {0.24, 0.24, 0.24, 1.0f};
bool enableWireframe = false;

//Grid image texure
VKTexture gridTexture;
// Model Push contant data
struct DefaultPushConstantData
{
    alignas(16) glm::mat4 model;
}modelPCData;
Transform modelTransform;
ImGuizmo::OPERATION globalOperation = ImGuizmo::TRANSLATE;
/******************************************************************************/
// Light settings
glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.32f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.4, 0.5, 1.0f);
/******************************* Vulkan Variables *****************************/
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderingFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;
/******************************************************************************/
void RecreateSwapchain();
void RecreateCommandPipeline();
void RecordCommands();
void CleanUpCommandListResources();
void UpdateMVPUBO(uint32_t currentImageIndex);
void CleanUpImGuiResources();
/******************************* ImGui Callbacks *******************************/
static void ImGuiError(VkResult err);
};
