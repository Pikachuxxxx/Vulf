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
#include "Vulkan/VKTexture.h"
#include "Vulkan/VKDepthImage.h"

// Execution command
// make && cd ../ && ./build//HelloVulkan -w 1280 -h 720  && cd build

namespace Vulf {
    /*
     * Vulf Base class with application flow abstraction
     * All Vulf Applications will derive from this to function properly
     */
    class VulfBase
    {
    //  Public Variables
    public:
        CommandLineParser   commandLineParser;      /* Command Line options parser for the executable       */
        bool                enableValidationLayers; /* Enables Vulkan validation layers in debug build      */
        int                 width;                  /* The Width of the window                              */
        int                 height;                 /* The Height of the window                             */
    //  Public Functions
    public:
        /* Initializes the application */
        VulfBase();
        /* Destroys any resources allocated by the client */
        virtual ~VulfBase();
        /* Inits the application into runtime */
        void Run();
        /* ImGui Overlay */
        //virtual void OnImGui();
        //void OnUpdate(double dt);
    // Private Variables
    private:
    // Application flow
        Window*            m_Window;                /* The window abstraction                               */
        Camera3D           m_Camera;                /* The default free-fly camera in the scene             */
        bool               m_FramebufferResized;    /* Boolean to identify screen resize event              */
        double             m_LastTime    = 0;       /* Time taken from since the last frame was rendered    */
        double             m_NbFrames    = 0;       /* Number of frames rendered in a second                */
        double             m_DeltaTime   = 0;       /* The time between the last and current frame          */
    // Private Functions
    private:
        /* Initializes any resources before the application starts up */
        // void InitResources();
        /* Creates the application window */
        // void InitWindow();
        /* Initializes basic Vulkan resources */
        // void InitVulkan();
        /* Inits ImGui for UI Overlay */
        // void InitImGui();
    };
}
