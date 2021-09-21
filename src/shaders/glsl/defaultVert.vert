/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;
}
*/


#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    // mat4 model;
    mat4 view;
    mat4 proj;
    mat4 _padding1;
    mat4 _padding2;
} ubo;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

layout (push_constant) uniform ModelPushConstantData{
    mat4 model;
}modelPushConstantData;

void main() {
    gl_PointSize = 5.0;
    gl_Position = ubo.proj * ubo.view * modelPushConstantData.model * vec4(inPosition, 1.0);
    fragNormal = mat3(transpose(inverse(modelPushConstantData.model))) * inNormal;;
    fragTexCoord = inTexCoord;
    fragPos = vec3(modelPushConstantData.model * vec4(inPosition, 1.0));
}
