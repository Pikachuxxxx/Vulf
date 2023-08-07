#version 450
#extension GL_ARB_separate_shader_objects : enable
//
layout(location = 0) in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_in;

layout (set = 0, binding = 2) uniform LightConstantData{
    vec4 viewPos;
}lightData;

layout(set = 0, binding = 1) uniform sampler2D cubeMapSampler;

layout(location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {

    outColor = vec4( 0.8, 0.4, 0.0, 1.0 );
    outColor = vec4(vs_in.Normal, 1.0f);
    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    outColor = texture(cubeMapSampler, vs_in.TexCoords);
    //--------------------------------------------------------------------------
    vec3 view_vector = vs_in.FragPos - lightData.viewPos.xyz;

    float angle = smoothstep( 0.3, 0.7, dot( normalize(-view_vector ), vs_in.Normal ) );

    vec3 reflect_vector = reflect( view_vector, vs_in.Normal );
    vec4 reflect_color = texture( cubeMapSampler, SampleSphericalMap(normalize(reflect_vector)));

    vec3 refrac_vector = refract( view_vector, vs_in.Normal, 0.3 );
    vec4 refract_color = texture( cubeMapSampler, SampleSphericalMap(normalize(refrac_vector)));

    outColor = mix( reflect_color, refract_color, angle );

    outColor = vec4(vs_in.Normal.xyz, 1.0f);
}
