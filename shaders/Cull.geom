#version 400 core
layout(points) in;
layout(points, max_vertices = 1) out;

in mat4 modelMatrix[1];
flat in int objectVisible[1];

out vec4 modelMatrixRow1;
out vec4 modelMatrixRow2;
out vec4 modelMatrixRow3;
out vec4 modelMatrixRow4;

void main()
{    
    // only emit primitive if the object is visible
    if (objectVisible[0] == 1) {
        modelMatrixRow1 = modelMatrix[0][0].xyzw;
        modelMatrixRow2 = modelMatrix[0][1].xyzw;
        modelMatrixRow3 = modelMatrix[0][2].xyzw;
        modelMatrixRow4 = modelMatrix[0][3].xyzw;
        EmitVertex();
        EndPrimitive();
    }
}
