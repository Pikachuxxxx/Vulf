#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Std. Libraries
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>

#include "vertex.h"
#include "utils/prettytable.h"
#include "utils/VulkanCheckResult.h"

/* project defines */
#define VKST(structName)    VK_STRUCTURE_TYPE_##structName

/* Global variables */
// Dimensions of the window
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#ifndef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2"
};

const std::vector<const char*> deviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
   "VK_KHR_portability_subset"
};

// The dynamics states of the graphis Pipeline
const VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_LINE_WIDTH
};

// Stores the grpahics families and their indices in the physical device
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// Stores the details about the type of swapchain
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class Application
{
public:
    void run();
private:
    GLFWwindow*                     window;                         // The GLFW window handle that manages the indowing and input system
    size_t                          currentFrame = 0;               // Tracking variable to use the right number of semaphores every frame
    bool                            framebufferResized = false;     // Boolean flag to known whether the framebuffer was resized
    /***************************** Vulkan Variables ***************************/
    VkInstance                      instance;                       // The handle for Vulkan instance
    VkDebugUtilsMessengerEXT        debugMessenger;                 // The handle for the debug callback messaging
    VkPhysicalDevice                gpu;                            // The physical GPU handle
    VkDevice                        device;                         // The logical abstraction of the GPU
    VkQueue                         graphicsQueue;                  // The queue to which all the graphics commands are submitted
    VkQueue                         presentQueue;                   // The queue to which all the presentation commands are submitted
    VkSurfaceKHR                    surface;                        // The surface to which the images are displayed
    VkSwapchainKHR                  swapchain;                      // The swapchain handle to manage the swapchain images
    std::vector<VkImage>            swapChainImages;                // The images to wich we render and present
    VkFormat                        swapchainFormat;                // The format of the swapchain
    VkExtent2D                      swapchainExtent;                // The swapchain image extent
    std::vector<VkImageView>        swapchainImageViews;            // The views to display the swapchain images
    VkRenderPass                    renderpass;                     // The handle to maintain the framebuffer render pass
    VkPipelineLayout                pipelineLayout;                 // The puipeline object to access the uniforms and the dynamic parts of the pipeline
    VkPipeline                      graphicsPipeline;               // The handle to the gra[hics pipeline object
    std::vector<VkFramebuffer>      swapChainFramebuffers;          // The framebuffers to display the frambuffers
    VkCommandPool                   commandPool;                    // The pool from which command buffers are allocated from
    std::vector<VkCommandBuffer>    commandBuffers;                 // The list of command buffers for each swapchain image onto which rendering command are recorded
    std::vector<VkSemaphore>        imageAcquiredSemaphores;        // Signals that the image has been acquired for rendering
    std::vector<VkSemaphore>        renderFinishedSemaphores;       // Signals that the rendering has finished and is ready to be presented
    std::vector<VkFence>            inFlightFences;                 // For CPU-GPU synchronization
    std::vector<VkFence>            imagesInFlight;                 // Track for each swap chain image if a frame in flight is currently using it
    VkBuffer                        rainbowTriangleVertexBuffer;    // Vertex buffer handle for the vertex data
    VkDeviceMemory                  vertexBufferMemory;             // The memory allocation handle for the Vertex buffer is use
    /********************************* Misc ***********************************/
private:
    /* Application Lifecycle */
    // Initialzes the window object
    void initWindow();
    // Initialzes the  Vulkan subsystems
    void initVulkan();
    // The main loop of the application
    void mainLoop();
    // Cleans up the resources allocated during the lifetime of the application
    void cleanup();
    /************************* Vulkan Initialization **************************/
    // Creates a Vulkan Instance
    void createInstance();
    // setup the debug messenger handle
    void setupDebugMessenger();
    // Creates the sruface to render onto
    void createSurface();
    // Picks the Physical GPU to use
    void pickPhysicalDevice();
    // Create a logical device handle to interact with the GPU selected
    void createLogicalDevice();
    // Creates the swapchain and it's images to present to the screen
    void createSwapChain();
    // Creates the image views to display the swapchain images
    void createImageViews();
    // Creates the render pass for the grpahics pipeline
    void createRenderPass();
    // Create the graphics pipeline
    void createGraphicsPipeline();
    // Creates the framebuffers to manage the data on screen
    void createFrameBuffers();
    // Create command pool to record the command buffers
    void createCommandPool();
    // Creates vertex buffer to hold the vertex data
    void createVertexBuffer(VkBuffer& buffer);
    // Creates command buffers to record the renderig commands
    void createCommandBuffers();
    // Creates synchronization objects such as semaphores, fences
    void createSynchronozationObjects();
    // Recreate the swapchain if the window is resized
    void recreateSwapchain();
    // Clean up any remaining swapchain resources
    void cleanUpSwapchain();
    /******************************* Draw Frame *******************************/
    void drawFrame();
    /****************************** Vulkan Helper *****************************/
    // checks which of the requested layers are supported by vulkan
    bool checkValidationLayersSupport();
    // retrieves the list of extensions to be loaded
    // TODO: Enumerate and print the available vs required instance extensions
    std::vector<const char*> getRequiredExtensions();
    // determines whether a particular GPU is suitable for use or not
    bool isDeviceSuitable(VkPhysicalDevice gpu);
    // finds the families and their indices in the GPU
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice gpu);
    // Checks against the available device extensions supported by the GPU
    bool checkDeviceExtensionSupport(VkPhysicalDevice gpu);
    // Queries for the swapchain format and other details
    SwapChainSupportDetails querySwapChainDetails(VkPhysicalDevice gpu);
    // Select the best surface format
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats);
    // Choose the swapchain presentation mode
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    // Select the swapchain extent
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    // Wraps the shader code into a shader module object
    VkShaderModule createShaderModule(const std::vector<char>& code);
    // helps find the right memory to allocate for the buffers
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    /****************************** Function pointers *************************/
    // Creates the debug utils messenger handle object (importing the symbol as it is EXT)
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    // Destroys the debug utils messehnger handle (importing as it is an extension)
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    /************************* Event callback Functions ***********************/
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    // TODO: Add callback for names of vk handles using VkDebugUtilsObjectNameInfoEXT handle
    // Framebuffer resize callback
    static void framebuffer_resize_callback(GLFWwindow* window, int height, int width);
};
