#version 450 core
layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 localPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    mat4 _padding1;
    mat4 _padding2;
} ubo;

void main()
{
    localPos = inPos;
    gl_Position =  ubo.proj * ubo.view * vec4(inPos, 1.0);
}
