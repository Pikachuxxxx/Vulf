#pragma once

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

// Per frame data that is used for managing swapchain image synchronization and triple buffering
struct FrameData
{
    // CmdBuffer + ImageIndex to render to + Sync primitives(Fences and Semaphores)
};

// Properties of the swapchain
struct SwapchainProperties
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class Swapchain
{
public:
    Swapchain() = default;

    void Init(GLFWwindow* window);
    void Destroy();

    // acquire the image from the swaphain
    bool acquire_image(VkSemaphore semaphore, VkFence fence);
    // Presenting the swapchain
    bool present_swapchain(VkQueue presentQueue, std::vector<VkSemaphore> waitSemaphores);

    inline const VkSwapchainKHR& get_handle() { return m_SwapchainKHR; }
    inline const uint32_t& get_image_count() { return m_SwapchainImageCount; }
    inline const VkFormat& get_format() { return m_SurfaceFormat.format; }
    inline const VkExtent2D& get_extent() { return m_SwapchainExtent; }
    inline const std::vector<VkImageView>& get_image_views() {return m_SwapchainImageViews; }
    inline const std::vector<VkImage>& get_images() {return m_SwapchainImages; }
    inline VkImageView& get_image_view_at(int index) {return m_SwapchainImageViews[index]; }
    inline VkImage& get_image_at(int index) {return m_SwapchainImages[index]; }

private:
    GLFWwindow*                 m_Window;
    VkSwapchainKHR              m_SwapchainKHR;
    SwapchainProperties         m_SurfaceProperties;
    VkSurfaceFormatKHR          m_SurfaceFormat;
    VkExtent2D                  m_SwapchainExtent;
    VkPresentModeKHR            m_PresentMode;
    uint32_t                    m_SwapchainImageCount = 0;
    std::vector<VkImage>        m_SwapchainImages;
    std::vector<VkImageView>    m_SwapchainImageViews;
    uint32_t                    m_ImageIndex; /* The image image to which we can render to from the list of m_SwapchainImages */

private:
    void query_swapchain_properties();
    VkExtent2D get_swapchain_extent();
    VkSurfaceFormatKHR choose_surface_formats();
    VkPresentModeKHR choose_present_mode();
    void retrieve_swapchain_images();
    void create_swapchain_image_views();
};
