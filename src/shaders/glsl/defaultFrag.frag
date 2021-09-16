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
    outColor = texture(texSampler, fragTexCoord);
    // outColor = vec4(light.lightColor * light.objectColor, 1.0);

    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = light.lightColor * vec3(texture(texSampler, fragTexCoord));

    // Diffuse Lighting
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light.lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.lightColor;

    vec3 result = (ambient + diffuse) * light.objectColor * vec3(texture(texSampler, fragTexCoord));
    outColor = vec4(result, 1.0);
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
