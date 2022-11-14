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
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 viewPos;
}light;

void main() {

    outColor = vec4(vs_in.TexCoords, 0.0f, 1.0f);
    outColor = vec4( 0.8, 0.4, 0.0, 1.0 );
    outColor = vec4(vs_in.Normal, 1.0f);

    //------------------------------------------
    //  Directio
    // Ambient lighting
    vec3 ambient = light.ambient.rgb;

    // Diffuse Lighting
    vec3 normal = normalize(vs_in.Normal);
    vec3 lightDir = normalize(-light.direction.rgb);
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.diffuse.rgb * diff;

    // Specular shading
    vec3 viewDir = normalize(light.viewPos.xyz - vs_in.FragPos.xyz);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 64);
    vec3 specular = light.specular.rgb * spec ;

    // Emission shading
    // Calculate the final lighting color
    vec3 result = ambient + diffuse + specular;
    outColor = vec4(result, 1.0f);

}
