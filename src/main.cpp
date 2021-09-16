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

int main(int argc, char* argv[]) {

    static std::vector<const char*> args;
    for (size_t i = 0; i < argc; i++) {
        std::cout << "Argument : " << argv[i] << std::endl;
        args.push_back(argv[i]);
    }
    Application app;
    app.commandLineParser.Parse(args); 

    // TODO: Add support for all command line arguments
    // --help : Prints all the availbale command line options
    if(app.commandLineParser.IsSet("help")) {
        app.commandLineParser.PrintHelp();
		//exit(0);
    }
    // enables valdiation layers
    if (app.commandLineParser.IsSet("validation")) {
        VK_LOG("Enabling Validation Layers");
        app.enableValidationLayers = true;
    }
    // Sets the width and height of the application
    if(app.commandLineParser.IsSet("width") || app.commandLineParser.IsSet("height"))
    {
        VK_LOG("Width : ", app.commandLineParser.GetValueAsInt("width", 800), ", Height : ", app.commandLineParser.GetValueAsInt("height", 600));
        app.width = app.commandLineParser.GetValueAsInt("width", 800);
        app.height = app.commandLineParser.GetValueAsInt("height", 600);
    }

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
