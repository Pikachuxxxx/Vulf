#include <iostream>

#include <VulfBase.h>
#include "utils/VulkanCheckResult.h"

// Load the Instance extensions/layers and device Extensions
std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2"
};

std::vector<const char*> g_DeviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

#include "VulfBase.h"

class VulfPBR : public Vulf::VulfBase
{
public:
    VulfPBR() : VulfBase("PBR demo") {}

    ~VulfPBR() {
        _def_CommandPool.Destroy();
        CleanUpPipeline();
        Device::Get()->Destroy();
        Instance::Get()->Destroy();
    }

// Types
private:

// Pipeline
private:


private:
    void LoadShaders() override {

    }

    void BuildTextureResources() override {

    }

    void BuildBufferResource() override {

    }

    void BuildFixedPipeline() override {

    }

    // default
    void BuildRenderPass() override {

    }

    void BuildGraphicsPipeline() override {

    }

    // default
    void BuildFramebuffer() override {

    }

    // default
    void BuildCommandBuffers() override {

    }

    void UpdateBuffers(uint32_t imageIndex) override {

    }

    void CleanUpPipeline() override {
        ZoneScopedC(0xffffff);

        _def_Swapchain.Destroy();
    }


private:

    void OnStart() override
    {

    }

    void OnRender() override
    {

    }
};

VULF_MAIN(PBR)
