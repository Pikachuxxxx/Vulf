#version 450
#extension GL_ARB_separate_shader_objects : enable
//
layout(location = 0) in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_in;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4( vs_in.Color, 1.0 );
}
