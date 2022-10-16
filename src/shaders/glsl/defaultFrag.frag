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

layout(set = 0, binding = 1) uniform sampler2D gridSampler;
layout(set = 0, binding = 2) uniform sampler2D checkerTex;
layout(set = 0, binding = 3, rgba32f) uniform image2D storageImage;

void main() {

    outColor = vec4( 0.8, 0.4, 0.0, 1.0 );
    outColor = vec4(vs_in.Normal, 1.0f);
    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    if(gl_FragCoord.x < 640)
        outColor = texture(gridSampler, vs_in.TexCoords) * vec4(vs_in.Color, 1.0f);
    else
        outColor = texture(checkerTex, vs_in.TexCoords) * vec4(vs_in.Color, 1.0f);

    vec4 color = imageLoad(storageImage, ivec2(gl_FragCoord));

    imageStore(storageImage, ivec2(gl_FragCoord), outColor + color);
}
