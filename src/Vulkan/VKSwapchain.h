#pragma once

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

struct SwapchainProperties
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VKSwapchain
{
public:
    VKSwapchain() = default;
    void Init(GLFWwindow* window);
    void Destroy();
private:
    VkSwapchainKHR m_SwapchainKHR;
    SwapchainProperties m_SurfaceProperties;
    VkSurfaceFormatKHR m_SurfaceFormat;
    VkExtent2D m_SwapchainExtent;
    VkPresentModeKHR m_PresentMode;
    GLFWwindow* m_Window;
    uint32_t m_SwapchainImageCount = 0;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
private:
    void querySwapchainProperties();
    VkExtent2D getSwapchainExtent();
    VkSurfaceFormatKHR chooseSurfaceFormats();
    VkPresentModeKHR choosePresentMode();
    void RetrieveSwapchainImages();
    void CreateSwapchainImageViews();
};
