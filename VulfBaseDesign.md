# Vulf Base Design

- Includes all the project include dependencies in a single palace pre-configures by the macros for them to work
- Consists of a command line parses to take options from the command line
- Utility Macros for `main()` method for easy runnning of examples
    - This is automatically included by the base class onto the example file that derives this, this all the examples
        can be built concurrently
- Provides default vulkan objects to render simple geometry such as
    - default Command Pool to allocate command buuffers, default descriptorsets, default swapchain images and framebuffers
    - All these can be customized by overriding the virtual methods
- `Draw()` is used for recording the command buffers, renderpasses and actual draw commands. They are called everyframe
- Automatic synchronization management for swapchain and framebuffers, the primitives and image index are passed properly to the `BuildSwapchain` and `BuildFramebuffer` methods to be completely customizable. Broken further into
    - `RenderFrame` : Default image acquire + submission and command buffer submission function
    - `PresentFrame` : Presents the current image to the swap chain
- All the methods as well their entire children calls are Virtual with default fool-proof behavior, this provides way for reusable and completely customizable application

## App flow

`Initxxxx` --> `InitVulkan` --> `Prepare` (Virtual) --> `RenderLoop` (Virtual) --> Implicit resources clean up --> Client `CleanUp`

**In-detail function calls flow**

`InitVulkan`
1. `Instance`
2. `Device Init`
3. `Command Pool Init` (default)
4. **[Optional]** Shaders Reflection
5. `SetupSynchronization` primitives

`Prepare`
1. `BuildSwapchain`
2. `BuildCommandPool`
3. `Build Pipeline Fixed Functions or cache`
4. `CreateTextureResources`
5. `CreateBufferResources`
6. `BuildShaders`
7. `ReflectShaders` [Used later need to refactor this]
8. `BuildGraphicsPipeline`
9. `BuildCommandBuffers`
10. `BuildDescriptors`
11. `BuildRenderpasses`

`RenderLoop`
1. Init while loop
2. Calculation of delta time, fps etc. + Profiling
3. `ImGuiFrameInit`
4. `OnUpdate` + Camera Update
5. `OnRender` - Begins and records draw commands and render passes
    - Default Behaviour (Virtual and can be overridden by the Client application)
    1. Query for descriptorsets and Command buffers
    2. Being recording render pass and command buffer
    3. `Draw`
    4. `RenderFrame`
    5. `PresentFrame`
