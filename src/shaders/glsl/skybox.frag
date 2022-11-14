#version 450
#extension GL_ARB_separate_shader_objects : enable
//
layout(location = 0) in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_in;

layout(set = 0, binding = 1) uniform sampler2D equirectangularMap;

layout(location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vs_in.FragPos)); // make sure to normalize localPos
    vec3 envColor = texture(equirectangularMap, uv).rgb;

    outColor = vec4(envColor, 1.0);
}
