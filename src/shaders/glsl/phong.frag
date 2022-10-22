#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_in;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform LightingDataUBO
{
     vec3 direction;
     vec3 ambient;
     vec3 diffuse;
     vec3 specular;
     vec4 _padding;
};

void main() {

    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    outColor = vec4( 0.8, 0.4, 0.0, 1.0 );
    outColor = vec4(vs_in.Normal, 1.0f);
}
