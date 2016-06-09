#version 400 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 normal[];

const float MAGNITUDE = 0.2f;

void generateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + vec4(normal[index], 0.0f) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    generateLine(0);
    generateLine(1);
    generateLine(2); 
}