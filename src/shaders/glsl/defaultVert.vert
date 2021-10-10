#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

/*
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    mat4 _padding1;
    mat4 _padding2;
} ubo;
*/


vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// TODO: Add this to a VOUT struct
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

layout (push_constant) uniform ModelPushConstantData{
    mat4 model;
}modelPushConstantData;

void main() {
    gl_PointSize    = 5.0;
    // gl_Position     = vec4(inPosition, 1.0);
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);


    // Outout variables from the vertex shader
    fragPos         = inPosition;
    fragNormal      = inNormal;
    fragTexCoord    = inTexCoord;
}
