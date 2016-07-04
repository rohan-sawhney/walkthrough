#version 400 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 normal[];

const float OFFSET = 0.002;

void generateLine(int index1, int index2)
{
    gl_Position = gl_in[index1].gl_Position + vec4(normal[index1], 0.0) * OFFSET;
    EmitVertex();
    gl_Position = gl_in[index2].gl_Position + vec4(normal[index2], 0.0) * OFFSET;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    generateLine(0, 1);
    generateLine(1, 2);
    generateLine(2, 0);
}