#version 400 core
layout (location = 0) in mat4 model;

out mat4 modelMatrix;
flat out int objectVisible;

layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
};
uniform vec4 position;
uniform vec3 extent;

void main(void)
{
    // set model matrix
    modelMatrix = model;
   
    // calculate modelview projection matrix
    mat4 mvp = projection * view * model;
   
    // create the bounding box of the object
    vec4 bbox[8];
    bbox[0] = mvp * (position + vec4( extent.x, extent.y, extent.z, 1.0));
    bbox[1] = mvp * (position + vec4(-extent.x, extent.y, extent.z, 1.0));
    bbox[2] = mvp * (position + vec4( extent.x,-extent.y, extent.z, 1.0));
    bbox[3] = mvp * (position + vec4(-extent.x,-extent.y, extent.z, 1.0));
    bbox[4] = mvp * (position + vec4( extent.x, extent.y,-extent.z, 1.0));
    bbox[5] = mvp * (position + vec4(-extent.x, extent.y,-extent.z, 1.0));
    bbox[6] = mvp * (position + vec4( extent.x,-extent.y,-extent.z, 1.0));
    bbox[7] = mvp * (position + vec4(-extent.x,-extent.y,-extent.z, 1.0));
   
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

    bool inFrustum = true;
    for (int i = 0; i < 6; i++) {
        if (outOfBound[i] == 8) {
            inFrustum = false;
        }
    }
    
    objectVisible = inFrustum ? 1 : 0;
}
