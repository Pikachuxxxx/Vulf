#include <iostream>

#include "application.h"
#include "utils/VulkanCheckResult.h"

std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2"
};

std::vector<const char*> deviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

#include "VulfBase.h"

class HelloTriangle : public Vulf::VulfBase
{

    void LoadShaders() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildCommandPool() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildSwapchain() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildPipeline() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildGraphicsPipeline() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildRenderPass() override {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void BuildFramebuffer() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildCommandBuffers() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void BuildDescriptorSets() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void CreateTextureResources() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void CreateBufferResource() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void UpdateBuffers() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void Draw() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void RenderFrame() override {
        throw std::logic_error("The method or operation is not implemented.");
    }


    void PresentFrame() override {
        throw std::logic_error("The method or operation is not implemented.");
    }
};

VULF_MAIN(HelloTriangle)