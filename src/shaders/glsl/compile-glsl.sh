echo Compiling shaders...
# Compiling the basic vertex buffer compitable shaders
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/shader.vert -o ../src/shaders/spir-v/vert.spv
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/shader.frag -o ../src/shaders/spir-v/frag.spv

# Outline shader
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/outline.frag -o ../src/shaders/spir-v/outline.spv

# Vignette shader
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/vignette.frag -o ../src/shaders/spir-v/vignette.spv

# Cimpilieng the tri color hard coded shaders
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/tri.vert -o ../src/shaders/spir-v/trivert.spv
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/tri.frag -o ../src/shaders/spir-v/trifrag.spv

echo Shaders compiled successfully!