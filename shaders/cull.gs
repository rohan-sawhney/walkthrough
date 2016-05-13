#version 400 core
layout(points) in;
layout(points, max_vertices = 1) out;

in mat4 modelMatrix[1];
flat in int objectVisible[1];

out mat4 culledModel;

void main()
{    
   // only emit primitive if the object is visible 
   if (objectVisible[0] == 1) {
      culledModel = modelMatrix[0];
      EmitVertex();
      EndPrimitive();
   }
}
