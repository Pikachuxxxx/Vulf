#version 450

layout (set = 0, binding = 0) uniform UBOMatrices {
	mat4 view;
	mat4 projection;
	mat4 _padding1;
    mat4 _padding2;
} uboMatrices;

layout (push_constant) uniform ModelPushConstantData{
    mat4 model;
}modelPushConstantData;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	gl_Position = uboMatrices.projection * uboMatrices.view * vec4(gridPlane[gl_VertexIndex].xyz, 1.0);
}