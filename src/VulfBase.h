#pragma once

#define OPTICM_MSVC

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

// Profiling
//#define TRACY_ENABLE
#include <Tracy.hpp>
// Optick
#define USE_OPTICK 0
#define OPTICK_ENABLE_GPU
//#include <optick.h>

// Helper utilities
#include "utils/VulkanCheckResult.h"
#include "utils/vertex.h"
#include "utils/mesh.h"
#include "utils/Window.h"
#include "utils/Camera3D.h"
#include "utils/Transform.h"
#include "utils/cube.h"
#include "utils/sphere.h"
#include "utils/CommandLineParser.h"
#include "utils/LoadObjModel.h"

// Vulkan Abstractions
#include "Barriers.h"
#include "Vulkan/Instance.h"
#include "Vulkan/Device.h"
#include "Vulkan/DescriptorSet.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Shader.h"
#include "Vulkan/FixedPipelineFuncs.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/GraphicsPipeline.h"
#include "Vulkan/ComputePipeline.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/CmdPool.h"
#include "Vulkan/CmdBuffer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/UniformBuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Texture.h"
#include "Vulkan/DepthImage.h"
#include "Vulkan/ImGuiOverlay.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Semaphore.h"
#include "Vulkan/StorageImage.h"

// Application Helper Defines
#define STRINGIZE(s) #s

// Execution command
// make && cd ../ && ./build//HelloVulkan -w 1280 -h 720  && cd build

namespace Vulf {

    using HighResClock = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using Ms = std::chrono::duration<double, std::milli>;

    /* The max number of frames that will be concurrently rendered to while the presentation is in process */
    const int MAX_FRAMES_IN_FLIGHT = 3;

    /*
     * Vulf Base class with application flow abstraction
     * All Vulf Applications will derive from this to function properly
     */
    class VulfBase
    {
    public:
        CommandLineParser   commandLineParser;          /* Command Line options parser for the executable       */
        bool                enableValidationLayers = 1; /* Enables Vulkan validation layers in debug build      */
        int                 width = 1280;             /* The Width of the window                              */
        int                 height = 720;              /* The Height of the window                             */

    public:
        /* Initializes the application */
        VulfBase(std::string appName) : m_AppName(appName) {}
        /* Destroys any resources allocated by the client */
        virtual ~VulfBase();
        /* Initializes the application into runtime */
        void Run();

        Camera3D& getCamera() { return m_Camera; }
        const Window* getWindow() { return m_Window; }

    protected:
        // App setting to customize (this is BS!)
        bool        waitOnAllFences = true;     /* Whether or not to wail on all fences for reset                   */
        CmdPool     graphicsCommandPool;        /* Command pool used to allocate graphics command buffers from      */
        CmdPool     computeCommandPool;         /* Command pool to allocate compute command buffer from             */
        // TODO: Move this to a new class called Context or Instance as we have only one Swapchain (default framebuffer will still be handled by user as offline rendering is possible)
        Swapchain   baseSwapchain;              /* The base swapchain from which the images are used to render      */
        // TODO: Remove!!! this from here!!! This is at the mercy of the use to create and use
        RenderPass  baseRenderPass;             /* The base renderpass used to transition the images                */

    protected:
        /**
         * Creates the Shader resources to be used by the pipeline
         * Note:- The shaders that are loaded in here must be added to the
         * pipeline by client itself to create the pipelines of their choice,
         * this method is just for styling purpose, makes code look neater,
         * (shaders must be added as members by the client thought)
         */
        virtual void LoadShaders() = 0;

        virtual void LoadAssets() {}

        /* Builds the command pool */
        virtual void BuildCommandPool();
        /* Create the Texture/image resources for the client */
        virtual void BuildTextureResources();
        /* Creates the necessary buffers resources */
        virtual void BuildBufferResource();
        /* Build swapchain */
        virtual void BuildSwapchain();
        /* Build the fixed pipelines and pipeline cache*/
        virtual void BuildFixedPipeline();
        /* Build the render passes for the Vulf application */
        virtual void BuildRenderPass();
        /* Builds the command pipeline for the entire application */
        virtual void BuildGraphicsPipeline();
        /* Build the framebuffers with the proper render pass attachments and swapchain configuration */
        virtual void BuildFramebuffer();
        /* Builds the command buffers that will be send to the graphics queue */
        virtual void BuildCommandBuffers();
        /* Build descriptor sets for Uniform buffers */
        virtual void BuildDescriptorSets();

        /* Cleans Up the resources and also while recreating the swapchain */
        virtual void CleanUpPipeline();

        /* The draw commands that will be executed */
        virtual void DrawFrame();
        /* Default image acquire + submission and command buffer submission */
        virtual void SubmitFrame();

        /* Called once before the app begins rendering and after the pipeline has been built */
        virtual void OnStart();
        /**
         * Called on every update loop
         *
         * @param dt The delta time taken to render a frame since the last frame
         */
        virtual void OnUpdate(double dt);
        /* Client defines how the scene is rendered and it's resources are used */
        virtual void OnRender(CmdBuffer dcb, CmdBuffer ccb);
        /* Updates Uniform buffers, SSAO and other buffer resources */
        virtual void OnUpdateBuffers(uint32_t frameIdx);
        /* ImGui Overlay */
        virtual void OnImGui();

        inline const uint64_t& get_fps() { return m_FPS; }
        inline const std::string& get_app_name() { return m_AppName; }
        inline const uint32_t& get_image_idx() const { return m_ImageIndex; }
        inline const uint32_t& get_frame_idx() const { return m_CurrentFrame; }
        inline ImGuiOverlay& get_ui_overlay() { return m_ImGuiOVerlay; }
        inline float get_dt() { return m_FrameTimer.count() * 1000.0f; }
    public:
        // Application flow
        std::string                 m_AppName;                      /* The name of the application                                                      */
        Window* m_Window;                       /* The window abstraction                                                           */
        Camera3D                    m_Camera;                       /* The default free-fly camera in th e scene                                        */
        bool                        m_FramebufferResized;           /* Boolean to identify screen resize event                                          */
        Ms                          m_FrameTimer;                   /* Time taken for a single frame to render since the last frame was rendered        */
        uint32_t                    m_FrameCounter = 0;
        HighResClock                m_LastTimestamp;                /* High resolution clock to measure the last time when a frame was rendered         */
        uint32_t                    m_ImageIndex = 0;        /* The next image index from the swapchain images list to which we can render to    */
        uint32_t                    m_CurrentFrame = 0;        /* The index of the frame/swap image index that is being presented on the screen    */
        uint64_t                    m_FPS = 0;        /* The frame per second                                                             */
        // FPS
        int64_t avgQnt = 1;
        int64_t minFPS = 1000.0f, maxFPS = 0, avgFPS = 0;

    private:
        ImGuiOverlay                m_ImGuiOVerlay;                 /* ImGui overlay for the application                                                */
        std::vector<CmdBuffer>      m_DrawCmdBuffers;
        std::vector<CmdBuffer>      m_ComputeCmdBuffers;
        std::vector<Semaphore>      m_RenderFinishedSemaphores;
        std::vector<Semaphore>      m_ImageAvailableSemaphores;
        std::vector<Semaphore>      m_GraphicsSemaphores;
        std::vector<Semaphore>      m_ComputeSemaphores;
        std::vector<Fence>          m_InFlightFences;

    private:
        // Init, render loop & build occur right after run in naming and then followed by protected and remaining private methods

        /* Initializes any resources before the application starts up */
        void InitResources();
        /* Creates the application window */
        void InitWindow();
        /* Initializes basic Vulkan resources */
        void InitVulkan();
        /* Initializes ImGui for UI Overlay */
        void InitImGui();

        /* The render loop that controls the application */
        void RenderLoop();

        /* The entire command pipeline that needs to be rebuilt for swapchain re-creation */
        void BuildCommandPipeline();

        void InitSyncPrimitives();
        void RecreateSwapchain();
        void QuitApp();
    };


#define VULF_MAIN(appName)                                                                                                                                                                                      \
    int main(int argc, char* argv[]) {                                                                                                                                                                          \
        static std::vector<const char*> args;                                                                                                                                                                   \
        for (size_t i = 0; i < argc; i++) {                                                                                                                                                                     \
            std::cout << "Argument : " << argv[i] << std::endl;                                                                                                                                                 \
            args.push_back(argv[i]);                                                                                                                                                                            \
        }                                                                                                                                                                                                       \
        Vulf##appName exampleVulfapp_##appName;                                                                                                                                                                 \
        exampleVulfapp_##appName.commandLineParser.Parse(args);                                                                                                                                                 \
        if (exampleVulfapp_##appName.commandLineParser.IsSet("help")) {                                                                                                                                         \
            exampleVulfapp_##appName.commandLineParser.PrintHelp();                                                                                                                                             \
            exit(0);                                                                                                                                                                                            \
        }                                                                                                                                                                                                       \
        if (exampleVulfapp_##appName.commandLineParser.IsSet("validation")) {                                                                                                                                   \
            VK_LOG("Enabling Validation Layers");                                                                                                                                                               \
            exampleVulfapp_##appName.enableValidationLayers = true;                                                                                                                                             \
        }                                                                                                                                                                                                       \
        if (exampleVulfapp_##appName.commandLineParser.IsSet("width") || exampleVulfapp_##appName.commandLineParser.IsSet("height")) {                                                                          \
            VK_LOG("Width : ", exampleVulfapp_##appName.commandLineParser.GetValueAsInt("width", 800), ", Height : ", exampleVulfapp_##appName.commandLineParser.GetValueAsInt("height", 600));                 \
            exampleVulfapp_##appName.width = exampleVulfapp_##appName.commandLineParser.GetValueAsInt("width", 800);                                                                                            \
            exampleVulfapp_##appName.height = exampleVulfapp_##appName.commandLineParser.GetValueAsInt("height", 600);                                                                                          \
        }                                                                                                                                                                                                       \
        try {                                                                                                                                                                                                   \
            exampleVulfapp_##appName.Run();                                                                                                                                                                     \
        }                                                                                                                                                                                                       \
        catch (const std::exception& e) {                                                                                                                                                                       \
            std::cerr << "\033[1;31m[VULKAN]\033[1;31m - ERROR : " << e.what() << " \033[0m\n";                                                                                                                 \
            return EXIT_FAILURE;                                                                                                                                                                                \
        }                                                                                                                                                                                                       \
        return EXIT_SUCCESS;                                                                                                                                                                                    \
    }
}
//{VK_LOG("Application Name : ", STRINGIZE(exampleVulfapp_##appName));    }                                                                                                                               \                                                          \
