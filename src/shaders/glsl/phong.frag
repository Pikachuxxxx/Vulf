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
     vec3 viewPos;
}light;

void main() {

    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    outColor = vec4( 0.8, 0.4, 0.0, 1.0 );
    outColor = vec4(vs_in.Normal, 1.0f);

    //------------------------------------------

    // Ambient lighting
    vec3 ambient = light.ambient * 0.25;

    // Diffuse Lighting
    vec3 normal = normalize(vs_in.Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff;

    // Specular shading
    vec3 viewDir = normalize(light.viewPos - vs_in.FragPos.xyz);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = light.specular * spec ;

    // Emission shading
    // Calculate the final lighting color
    vec3 result = (ambient + diffuse + specular);
    outColor = vec4(result, 1.0f);
}
