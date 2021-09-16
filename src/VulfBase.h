#pragma once

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
#include <string>
#include <numeric>
#include <array>

// Vulkan headers
#include <vulkan/vulkan.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// ImGuizmo
#include <ImGuizmo/ImGuizmo.h>

// SPIRV-Reflect
#include <spirv_reflect.h>

// Helper utilities
#include "utils/vertex.h"
#include "utils/Window.h"
#include "utils/Camera3D.h"
#include "utils/Transform.h"
#include "utils/cube.h"
#include "utils/sphere.h"
#include "utils/CommandLineParser.h"

// Vulkan Abstractions
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
#include "Vulkan/VKImage.h"
#include "Vulkan/VKTexture.h"
#include "Vulkan/VKDepthImage.h"

// Execution command
// make && cd ../ && ./build//HelloVulkan -w 1280 -h 720  && cd build

namespace Vulf {
    /* The max number of frames that will be concurrently rendered to while the presentation is in process */
    const int MAX_FRAMES_IN_FLIGHT = 2;

    /*
     * Vulf Base class with application flow abstraction
     * All Vulf Applications will derive from this to function properly
     */
    class VulfBase
    {
    public:
        CommandLineParser   commandLineParser;          /* Command Line options parser for the executable       */
        bool                enableValidationLayers;     /* Enables Vulkan validation layers in debug build      */
        int                 width   = 800;              /* The Width of the window                              */
        int                 height  = 600;              /* The Height of the window                             */

    public:
        /* Initializes the application */
        VulfBase();
        /* Destroys any resources allocated by the client */
        virtual ~VulfBase();
        /* Inits the application into runtime */
        void Run();

    protected:
        VKCmdPool           _def_CommandPool;           /* The default command pool used to allocate buffer     */

    protected:
        /**
         * Creates the Shader resources to be used by the pipeline
         * Note:- The shaders that are loaded in here must be added to the
         * pipeline by client itself to create the pipelines of their choice,
         * this method is just for styling purpose, makes code look neater,
         * (shaders must be added as members by the client thought)
         */
        virtual void LoadShaders();

        /* Builds the command pipeline for the entire application */
        virtual void BuildCommandPipeline();
        /* Build the renderpassed for the Vulf application */
        virtual void BuildRenderPass();
        /* Build the framebuffers with the proper render pass attachments and swapchain configuration */
        virtual void BuildFramebuffer();

        /* The draw commands that will be executed */
        virtual void Draw();

        /**
         * Called on every update loop
         * @param dt The delta time taken to render a frame since the last frame
         */
        virtual void OnUpdate(double dt);
        /* Client defines how the scene is renderd and it's resources are used */
        virtual void OnRender();
        /* ImGui Overlay */
        virtual void OnImGui();

    private:
    // Application flow
        Window*                     m_Window;                       /* The window abstraction                                                           */
        Camera3D                    m_Camera;                       /* The default free-fly camera in the scene                                         */
        bool                        m_FramebufferResized;           /* Boolean to identify screen resize event                                          */
        double                      m_LastTime    = 0;              /* Time taken from since the last frame was rendered                                */
        double                      m_NbFrames    = 0;              /* Number of frames rendered in a second                                            */
        double                      m_DeltaTime   = 0;              /* The time between the last and current frame                                      */
        uint32_t                    m_NextImageIndex;               /* The next image index from the swapchain images list                              */
        uint32_t                    m_CurrentFrameIndex;            /* The index of the current in-flight frame being rendered                          */
    // Vulkan Stuff
        VKInstance                  m_Instance;                     /* The Vulkan abstracted Instance                                                   */
        VKLogicalDevice             m_Device;                       /* The Vulkan physical and logical device abstraction                               */
        // TODO: Read (Rendering and presentation)[https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation] and make proper article for the website
        std::vector<VkSemaphore>    m_ImageAvailableSemaphores;     /* Semaphore to tell when an image is free to use to draw onto (GPU-GPU)            */
        std::vector<VkSemaphore>    m_RenderingFinishedSemaphores;  /* Semaphore to tell when the rendering to a particular swpachain image is done     */
        std::vector<VkFence>        m_InFlightFences;               /* Use to synchronize the GPU-CPU so that they draw onto the right image in flight  */
        std::vector<VkFence>        m_ImagesInFlight;

    private:
        /* Initializes any resources before the application starts up */
        void InitResources();
        /* Creates the application window */
        void InitWindow();
        /* Initializes basic Vulkan resources */
        void InitVulkan();
        /* Inits ImGui for UI Overlay */
        void InitImGui();

        /* Presents the rendered swapimages onto the window surface */
        void Present();

        /* Creates the Synchorinization primitives for the application */
        // Note:- The fences must be initially explicitly signalled
        void CreateSynchronizationPrimitives();
    };
}
