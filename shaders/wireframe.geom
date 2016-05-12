#version 400 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float OFFSET = 0.002f;

void GenerateLine(int index1, int index2)
{
    gl_Position = gl_in[index1].gl_Position + vec4(gs_in[index1].normal, 0.0f) * OFFSET;
    EmitVertex();
    gl_Position = gl_in[index2].gl_Position + vec4(gs_in[index2].normal, 0.0f) * OFFSET;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0, 1);
    GenerateLine(1, 2); 
    GenerateLine(2, 0); 
}