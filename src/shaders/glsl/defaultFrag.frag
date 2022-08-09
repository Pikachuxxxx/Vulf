#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_in;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D checkerTex;

void main() {

    outColor = vec4(vs_in.Normal, 1.0f);
    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    if(gl_FragCoord.x < 700)
        outColor = texture(checkerTex, vs_in.TexCoords) * vec4(vs_in.Color, 1.0f);
    else
        outColor = texture(texSampler, vs_in.TexCoords) * vec4(vs_in.Color, 1.0f);

    outColor = vec4(vs_in.FragPos, 1.0f);

}


/*
// Simple light shader
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 2) uniform LightUniformBufferObject {
    vec3 objectColor;
    vec3 lightColor;
    vec3 lightPos;
} light;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {

    // outColor = vec4(fragNormal, 1.0);
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
    // outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(light.lightColor * light.objectColor, 1.0);

    // Ambient lighting
    float ambientStrength = 0.1;
    // vec3 ambient = light.lightColor * vec3(texture(texSampler, fragTexCoord));
    vec3 ambient = light.lightColor * ambientStrength;

    // Diffuse Lighting
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light.lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.lightColor;

    // vec3 result = (ambient + diffuse) * light.objectColor * vec3(texture(texSampler, fragTexCoord));
    vec3 result = (ambient + diffuse) * light.objectColor;
    outColor = vec4(result, 1.0);
}

*/
