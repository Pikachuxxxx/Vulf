echo Compiling shaders...
# Compiling the basic vertex buffer compitable shaders
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/shader.vert -o ../src/shaders/spir-v/vert.spv
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/shader.frag -o ../src/shaders/spir-v/frag.spv

# Cimpilieng the tri color hard coded shaders
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/tri.vert -o ../src/shaders/spir-v/trivert.spv
$VULKAN_SDK/macOS/bin/glslc ../src/shaders/glsl/tri.frag -o ../src/shaders/spir-v/trifrag.spv

echo Shaders compiled successfully!
