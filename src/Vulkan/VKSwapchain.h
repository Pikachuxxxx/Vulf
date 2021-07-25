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
    const VkSwapchainKHR& GetSwapchainKHR() { return m_SwapchainKHR; }
    const VkExtent2D& GetSwapExtent() { return m_SwapchainExtent; }
    const VkFormat& GetSwapFormat() { return m_SurfaceFormat.format; }
    std::vector<VkImageView> GetSwapImageViews() {return m_SwapchainImageViews; }
    VkImageView& GetSwapImageViewAt(int index) {return m_SwapchainImageViews[index]; }
    std::vector<VkImage> GetSwapImages() {return m_SwapchainImages; }
    VkImage& GetSwapImageAt(int index) {return m_SwapchainImages[index]; }
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