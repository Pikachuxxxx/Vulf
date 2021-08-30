#version 450
#extension GL_ARB_separate_shader_objects : enable

////////////////////////////////////////////////////////////////////////////////
// Uncecessayt stuff
// TODO: Create a separate descriptro sets that use these vertex-fragment shaders pair 
layout(binding = 2) uniform LightUniformBufferObject {
    vec3 objectColor;
    vec3 lightColor;
    vec3 lightPos;
} light;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;
////////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 outColor;

void main() {

    outColor = vec4(1.0f, 0.66f, 0.0f, 1.0);
}
