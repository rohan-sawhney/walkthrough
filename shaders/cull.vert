#version 400 core
layout (location = 0) in mat4 model;

out mat4 modelMatrix;
flat out int objectVisible;

layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
};
uniform vec3 boxMin;
uniform vec3 boxMax;

void main()
{
    // set model matrix
    modelMatrix = model;
   
    // calculate modelview projection matrix
    mat4 mvp = projection * view * model;
   
    // create the bounding box of the object
    vec4 bbox[8];
    bbox[0] = mvp * vec4(boxMax.x, boxMax.y, boxMax.z, 1.0);
    bbox[1] = mvp * vec4(boxMin.x, boxMax.y, boxMax.z, 1.0);
    bbox[2] = mvp * vec4(boxMax.x, boxMin.y, boxMax.z, 1.0);
    bbox[3] = mvp * vec4(boxMin.x, boxMin.y, boxMax.z, 1.0);
    bbox[4] = mvp * vec4(boxMax.x, boxMax.y, boxMin.z, 1.0);
    bbox[5] = mvp * vec4(boxMin.x, boxMax.y, boxMin.z, 1.0);
    bbox[6] = mvp * vec4(boxMax.x, boxMin.y, boxMin.z, 1.0);
    bbox[7] = mvp * vec4(boxMin.x, boxMin.y, boxMin.z, 1.0);
   
    // check if bounding box is inside the view frustum
    int outOfBound[6] = int[6](0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 8; i++) {
        if (bbox[i].x >  bbox[i].w) outOfBound[0]++;
        if (bbox[i].x < -bbox[i].w) outOfBound[1]++;
        if (bbox[i].y >  bbox[i].w) outOfBound[2]++;
        if (bbox[i].y < -bbox[i].w) outOfBound[3]++;
        if (bbox[i].z >  bbox[i].w) outOfBound[4]++;
        if (bbox[i].z < -bbox[i].w) outOfBound[5]++;
    }

    objectVisible = 1;
    for (int i = 0; i < 6; i++) {
        if (outOfBound[i] == 8) objectVisible = 0;
    }
}
