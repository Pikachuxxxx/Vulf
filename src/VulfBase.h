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
#include <Tracy.hpp>

// Helper utilities
#include "utils/vertex.h"
#include "utils/Window.h"
#include "utils/Camera3D.h"
#include "utils/Transform.h"
#include "utils/cube.h"
#include "utils/sphere.h"
#include "utils/CommandLineParser.h"

// Vulkan Abstractions
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
#include "Vulkan/Image.h"
#include "Vulkan/Texture.h"
#include "Vulkan/DepthImage.h"

// Application Helper Defines
#define STRINGIZE(s) #s

// Execution command
// make && cd ../ && ./build//HelloVulkan -w 1280 -h 720  && cd build

namespace Vulf {

    using HighResClock = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using Ms = std::chrono::duration<double, std::milli>;

    /* The max number of frames that will be concurrently rendered to while the presentation is in process */
    const int MAX_FRAMES_IN_FLIGHT = 2;
    const int SWAP_IMAGES_COUNT = 3;

    /*
     * Vulf Base class with application flow abstraction
     * All Vulf Applications will derive from this to function properly
     */
    class VulfBase
    {
    public:
        CommandLineParser   commandLineParser;          /* Command Line options parser for the executable       */
        bool                enableValidationLayers = 0; /* Enables Vulkan validation layers in debug build      */
        int                 width   = 1280;             /* The Width of the window                              */
        int                 height  = 720;              /* The Height of the window                             */
        std::vector<CmdBuffer> submissionCommandBuffers;

    public:
        /* Initializes the application */
        VulfBase(std::string appName) : m_AppName(appName) {}
        /* Destroys any resources allocated by the client */
        virtual ~VulfBase();
        /* Initializes the application into runtime */
        void Run();

    protected:
        CmdPool           _def_CommandPool;           /* The default command pool used to allocate buffer     */
        Swapchain         _def_Swapchain;
    protected:
        /**
         * Creates the Shader resources to be used by the pipeline
         * Note:- The shaders that are loaded in here must be added to the
         * pipeline by client itself to create the pipelines of their choice,
         * this method is just for styling purpose, makes code look neater,
         * (shaders must be added as members by the client thought)
         */
        virtual void LoadShaders() = 0;

        /* Builds the command pool */
        virtual void BuildCommandPool();
        /* Creates the necessary buffers resources */
        virtual void BuildBufferResource() = 0;
        /* Create the Texture/image resources for the client */
        virtual void BuildTextureResources();
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
        /* Updates Uniform buffers, SSAO and other \buffer resources */
        virtual void UpdateBuffers();
        /* Default image acquire + submission and command buffer submission */
        virtual void SubmitFrame();

        /**
         * Called on every update loop
         *
         * @param dt The delta time taken to render a frame since the last frame
         */
        virtual void OnUpdate(double dt);
        /* Client defines how the scene is rendered and it's resources are used */
        virtual void OnRender();
        /* ImGui Overlay */
        virtual void OnImGui();

        uint32_t GetNextImageIndex() const { return m_NextImageIndex;  }

    private:
    // Application flow
        std::string                 m_AppName;                      /* The name of the application                                                      */
        Window*                     m_Window;                       /* The window abstraction                                                           */
        Camera3D                    m_Camera;                       /* The default free-fly camera in th e scene                                        */
        bool                        m_FramebufferResized;           /* Boolean to identify screen resize event                                          */
        uint32_t                    m_FrameCounter    = 0;          /* Number of frames rendered in a second                                            */
        Ms                          m_FrameTimer;                   /* Time taken for a single frame to render since the last frame was rendered        */
        HighResClock                m_LastTimestamp;                /* High resolution clock to measure the last time when a frame was rendered         */
        uint32_t                    m_NextImageIndex = 0;           /* The next image index from the swapchain images list                              */
        uint32_t                    m_CurrentImageIndex = 0;        /* The index of the current in-flight frame being rendered                          */

    private:
        // Vulkan Stuff
        Instance                  m_Instance;                     /* The Vulkan abstracted Instance                                                   */
        VKLogicalDevice             m_Device;                       /* The Vulkan physical and logical device abstraction                               */
        std::vector<VkSemaphore>    m_ImageAvailableSemaphores;     /* Semaphore to tell when an image is free to use to draw onto (GPU-GPU)            */
        std::vector<VkSemaphore>    m_RenderingFinishedSemaphores;  /* Semaphore to tell when the rendering to a particular swapchain image is done     */
        std::vector<VkFence>        m_InFlightFences;               /* Use to synchronize the GPU-CPU so that they draw onto the right image in flight  */
        std::vector<VkFence>        m_ImagesInFlight;

    private:
        /* The entire command pipeline that needs to be rebuilt for swapchain re-creation */
        void BuildCommandPipeline();

        /* The render loop that controls the application */
        void RenderLoop();

        /* Initializes any resources before the application starts up */
        void InitResources();
        /* Creates the application window */
        void InitWindow();
        /* Initializes basic Vulkan resources */
        void InitVulkan();
        /* Initializes ImGui for UI Overlay */
        void InitImGui();

        /* Creates the Synchronization primitives for the application */
        // Note:- The fences must be initially explicitly signaled (Remove this note)
        void CreateSynchronizationPrimitives();

        void RecreateSwapchain();
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
                                                                                                                                                                                                                \
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
