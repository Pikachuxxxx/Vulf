#version 450

layout(vertices = 3) out;

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 0.3;
        gl_TessLevelOuter[0] = 0.3;
        gl_TessLevelOuter[1] = 0.4;
        gl_TessLevelOuter[2] = 0.5;

        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    }
}
