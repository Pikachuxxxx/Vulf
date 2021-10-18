#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{

	outFragColor =  vec4(1.0f, 0.0f, 1.0f, 1.0f);
	outFragColor = texture(samplerColorMap, inUV) * vec4(inColor, 1.0);
	outFragColor =  vec4(inColor, 1.0);

}