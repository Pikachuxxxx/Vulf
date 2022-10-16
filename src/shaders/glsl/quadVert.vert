 #version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

layout(location = 0) out VS_OUT {
    vec2 TexCoords;
    vec3 Color;
} vs_out;

void main() {
    gl_Position     = vec4(inPosition, 1.0);

    vs_out.TexCoords    = inTexCoord;
    vs_out.Color        = inColor;
}
