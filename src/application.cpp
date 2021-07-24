#include "application.h"

/**************************** Application Flow ********************************/
void Application::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
}

void Application::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // Create the window
    window = glfwCreateWindow(800, 600, "Hello Vulkan again!", nullptr, nullptr);
    // glfwSetKeyCallback(window, key_callback);
}

void Application::InitVulkan()
{
    VKInstance::GetInstanceManager()->Init("Hello Vulkan", window);

    VKLogicalDevice::GetDeviceManager()->Init();

    swapchainManager.Init(window);

    vertexShader.CreateShader("./src/shaders/spir-v/trivert.spv", ShaderType::VERTEX_SHADER);
    fragmentShader.CreateShader("./src/shaders/spir-v/trifrag.spv", ShaderType::FRAGMENT_SHADER);

    fixedPipelineFuncs.SetVertexInputSCI();
    fixedPipelineFuncs.SetInputAssemblyStageInfo(Topology::TRIANGLES);
    fixedPipelineFuncs.SetViewportSCI(swapchainManager.GetSwapExtent());
    fixedPipelineFuncs.SetRasterizerSCI();
    fixedPipelineFuncs.SetMultiSampleSCI();
    fixedPipelineFuncs.SetDepthStencilSCI();
    fixedPipelineFuncs.SetColorBlendSCI();
    fixedPipelineFuncs.SetDynamicSCI();
    fixedPipelineFuncs.SetPipelineLayout();
}

void Application::MainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawFrame();
    }
}

void Application::DrawFrame()
{

}

void Application::CleanUp()
{
    fixedPipelineFuncs.DestroyPipelineLayout();
    vertexShader.DestroyModule();
    fragmentShader.DestroyModule();
    swapchainManager.Destroy();
    VKLogicalDevice::GetDeviceManager()->Destroy();
    VKInstance::GetInstanceManager()->Destroy();
}
/******************************************************************************/
/******************************* GLFW Callbacks *******************************/
