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

class Swapchain
{
public:
    Swapchain() = default;
    void Init(GLFWwindow* window);
    void Destroy();
    const VkSwapchainKHR& GetSwapchainKHR() { return m_SwapchainKHR; }
    const VkExtent2D& GetSwapExtent() { return m_SwapchainExtent; }
    const VkFormat& GetSwapFormat() { return m_SurfaceFormat.format; }
    std::vector<VkImageView> GetSwapImageViews() {return m_SwapchainImageViews; }
    VkImageView& GetSwapImageViewAt(int index) {return m_SwapchainImageViews[index]; }
    std::vector<VkImage> GetSwapImages() {return m_SwapchainImages; }
    VkImage& GetSwapImageAt(int index) {return m_SwapchainImages[index]; }
    uint32_t GetSwapImageCount() { return m_SwapchainImageCount; }

    VkDescriptorSet* getDescSetAtIndex(uint32_t index) { return &m_DescriptorSets[index]; }
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

    VkDescriptorSetLayout g_DescriptorSetLayout;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet>                m_DescriptorSets;
    VkSampler m_ImageSampler;
private:
    void querySwapchainProperties();
    VkExtent2D getSwapchainExtent();
    VkSurfaceFormatKHR chooseSurfaceFormats();
    VkPresentModeKHR choosePresentMode();
    void RetrieveSwapchainImages();
    void CreateSwapchainImageViews();

    void create_sets();
    void update_sets();
};
