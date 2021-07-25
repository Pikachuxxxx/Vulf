#include <iostream>

#include "application.h"

std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2"
};


std::vector<const char*> deviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
   //"VK_KHR_portability_subset"
};

int main() {

    Application app;

    try
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "\033[1;31m[VULKAN]\033[1;31m - ERROR : " << e.what() <<  " \033[0m\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
