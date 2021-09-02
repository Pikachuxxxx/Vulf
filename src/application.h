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
#include "utils/sphere.h"

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
#include "Vulkan/VKDepthImage.h"

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
VKSwapchain swapchainManager;
VKShader vertexShader;
VKShader fragmentShader;
VKShader outlineFragmentShader;
VKFixedPipelineFuncs fixedPipelineFuncs;
VKFixedPipelineFuncs wireframeFixedPipelineFuncs;
VKRenderPass renderPassManager;
VKGraphicsPipeline graphicsPipeline;
VKGraphicsPipeline wireframeGraphicsPipeline;
VKFramebuffer framebufferManager;
VKCmdPool cmdPoolManager;
VKCmdBuffer swapCmdBuffers;

std::vector<VKFixedPipelineFuncs> fixedTopologyPipelines;
std::vector<VKFixedPipelineFuncs>  wireframeFixedTopologyPipelineFuncs;

std::vector<VKGraphicsPipeline> graphicsPipelines;
std::vector<VKGraphicsPipeline> wireframeGraphicsPipelines;

VKVertexBuffer triVBO;
VKIndexBuffer triIBO;

VKVertexBuffer quadVBO;
VKIndexBuffer quadIBO;

// Happy budda model
std::vector<Vertex> buddaVertices;
std::vector<uint16_t> buddaIndices;
std::vector<uint16_t> buddaQuadIndices;
VKVertexBuffer buddaVBO;
VKIndexBuffer buddaIBO;
VKIndexBuffer buddaQuadIBO;

// Cube
VKVertexBuffer cubeVBO;

// Sphere
std::vector<Vertex> sphereVertexData;
VKVertexBuffer sphereVBO;
VKIndexBuffer sphereIBO;
VKIndexBuffer sphereQuadIBO;

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
bool enableWireframe = true;

//Grid image texure
VKTexture gridTexture;
VKTexture earthTexture;
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
VKDepthImage depthImage;
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
