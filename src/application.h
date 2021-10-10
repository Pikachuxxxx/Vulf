// Vulkan Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_DEPTH_ZERO_TO_ONE

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

#include "utils/vertex.h"

// Helper includes
#include "utils/Window.h"
#include "utils/Camera3D.h"
#include "utils/Transform.h"
#include "utils/cube.h"
#include "utils/sphere.h"
// Command Line Parser
#include "utils/CommandLineParser.h"


#include "Vulkan/Instance.h"
#include "Vulkan/VKDevice.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Shader.h"
#include "Vulkan/FixedPipelineFuncs.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/GraphicsPipeline.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/CmdPool.h"
#include "Vulkan/CmdBuffer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/UniformBuffer.h"
#include "Vulkan/Texture.h"
#include "Vulkan/DepthImage.h"

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// ImGuizmo
#include <ImGuizmo/ImGuizmo.h>

// SPIRV-Reflect
#include <spirv_reflect.h>

#define STRINGIZE(s) #s

const int MAX_FRAMES_IN_FLIGHT = 2;

class Application
{
public:
    void Run();
/***************************** Application Variables **************************/
public:
CommandLineParser commandLineParser;
bool enableValidationLayers = false;
int width = 800, height = 600;
private:
    Window* window;
    Camera3D camera;
    bool framebufferResized = false;
    double lastTime;
    double nbFrames = 0;
    double delta = 0;
/****************************** Application Flow ******************************/
    void InitResources();
    void InitWindow();
    void InitVulkan();
    void InitImGui();
    void MainLoop();
    void DrawFrame();
    void CleanUp();
    void OnImGui();
    void OnUpdate(double dt);
    void LoadModel(std::string path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, std::vector<uint16_t>& quadIndices, bool triangulate = true);
/***************************** Vulkan Encapsulation ***************************/
Swapchain swapchainManager;
Shader vertexShader;
Shader fragmentShader;
Shader outlineFragmentShader;
FixedPipelineFuncs fixedPipelineFuncs;
FixedPipelineFuncs wireframeFixedPipelineFuncs;
RenderPass renderPassManager;
GraphicsPipeline graphicsPipeline;
GraphicsPipeline wireframeGraphicsPipeline;
Framebuffer framebufferManager;
CmdPool cmdPoolManager;
CmdBuffer swapCmdBuffers;

std::vector<FixedPipelineFuncs> fixedTopologyPipelines;
std::vector<FixedPipelineFuncs>  wireframeFixedTopologyPipelineFuncs;

std::vector<GraphicsPipeline> graphicsPipelines;
std::vector<GraphicsPipeline> wireframeGraphicsPipelines;

VertexBuffer triVBO;
IndexBuffer triIBO;

VertexBuffer quadVBO;
IndexBuffer quadIBO;

// Happy budda model
std::vector<Vertex> buddaVertices;
std::vector<uint16_t> buddaIndices;
std::vector<uint16_t> buddaQuadIndices;
VertexBuffer buddaVBO;
IndexBuffer buddaIBO;
IndexBuffer buddaQuadIBO;

// Cube
VertexBuffer cubeVBO;

// Sphere
std::vector<Vertex> sphereVertexData;
VertexBuffer sphereVBO;
IndexBuffer sphereIBO;
IndexBuffer sphereQuadIBO;

// Descriptor and uniforms shit!
UniformBuffer mvpUniformBuffer;

// Imgui Render pass
VkRenderPass imguiRenderPass;
// VKCmdPool imguiCmdPool;
// ImGui Descriptor pool
VkDescriptorPool imguiDescriptorPool;
VkCommandBuffer imguiCmdBuffer;
CmdBuffer imguiCmdBuffers;
float clearColor[4] = {0.24, 0.24, 0.24, 1.0f};
bool enableWireframe = true;

//Grid image texure
Texture gridTexture;
Texture earthTexture;
ImTextureID imguiGridTexture;
ImTextureID imguiEarthTexture;

// Model Push contant data
struct DefaultPushConstantData
{
    alignas(16) glm::mat4 model;
}modelPCData;
Transform modelTransform;
ImGuizmo::OPERATION globalOperation = ImGuizmo::TRANSLATE;

// Depth Image
DepthImage depthImage;
/******************************************************************************/
// Light settings
glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.32f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.4, 0.5, 1.0f);
/*
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 6,
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 7,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 8,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    VK_PRIMITIVE_TOPOLOGY_PATCH_LIST = 10,
  */
uint32_t topologyPipelineID = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
uint32_t wireframeTopologyPipelineID = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
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
