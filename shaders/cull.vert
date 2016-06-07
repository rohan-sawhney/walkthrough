#version 400 core
layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
    vec4 viewport;
};

layout (location = 0) in mat4 model;

uniform vec3 boxMin;
uniform vec3 boxMax;
uniform sampler2D hiZBuffer;

vec4 bbox[8];

subroutine int cullRoutineType();

subroutine(cullRoutineType)
int passThrough()
{
    // always return 1
    return 1;
}

subroutine(cullRoutineType)
int frustumCulling()
{
    // calculate modelview projection matrix
    mat4 mvp = projection * view * model;
    
    // create the bounding box of the object
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
    
    int inFrustum = 1;
    for (int i = 0; i < 6; i++) {
        if (outOfBound[i] == 8) inFrustum = 0;
    }
    
    return inFrustum;
}

subroutine(cullRoutineType)
int hiZOcclusionCulling()
{
    // do frustum culling
    if (frustumCulling() == 0) return 0;
    
    // perform perspective division for the bounding box
    for (int i = 0; i < 8; i++) bbox[i].xyz /= bbox[i].w;
    
    // calculate screen space bounding rectangle
    vec2 boundingRect[2];
    boundingRect[0].x = min(min(min(bbox[0].x, bbox[1].x),
                                min(bbox[2].x, bbox[3].x)),
                            min(min(bbox[4].x, bbox[5].x),
                                min(bbox[6].x, bbox[7].x))) / 2.0 + 0.5;
    
    boundingRect[0].y = min(min(min(bbox[0].y, bbox[1].y),
                                min(bbox[2].y, bbox[3].y)),
                            min(min(bbox[4].y, bbox[5].y),
                                min(bbox[6].y, bbox[7].y))) / 2.0 + 0.5;
    
    boundingRect[1].x = max(max(max(bbox[0].x, bbox[1].x),
                                max(bbox[2].x, bbox[3].x)),
                            max(max(bbox[4].x, bbox[5].x),
                                max(bbox[6].x, bbox[7].x))) / 2.0 + 0.5;
    
    boundingRect[1].y = max(max(max(bbox[0].y, bbox[1].y),
                                max(bbox[2].y, bbox[3].y)),
                            max(max(bbox[4].y, bbox[5].y),
                                max(bbox[6].y, bbox[7].y))) / 2.0 + 0.5;
    
    // calculate linear depth value of the front-most point
    float instanceDepth = min(min(min(bbox[0].z, bbox[1].z),
                                  min(bbox[2].z, bbox[3].z)),
                              min(min(bbox[4].z, bbox[5].z),
                                  min(bbox[6].z, bbox[7].z)));
    
    // calculate the bounding rectangle size in viewport coordinates
    float viewSizeX = (boundingRect[1].x - boundingRect[0].x) * viewport.y;
    float viewSizeY = (boundingRect[1].y - boundingRect[0].y) * viewport.z;
    
    // calculate the texture LOD used for lookup in the depth buffer texture
    float lod = ceil(log2(max(viewSizeX, viewSizeY) / 2.0));
    
    // fetch the depth texture using explicit LOD lookups
    vec4 samples;
    samples.x = textureLod(hiZBuffer, vec2(boundingRect[0].x, boundingRect[0].y), lod).x;
    samples.y = textureLod(hiZBuffer, vec2(boundingRect[0].x, boundingRect[1].y), lod).x;
    samples.z = textureLod(hiZBuffer, vec2(boundingRect[1].x, boundingRect[1].y), lod).x;
    samples.w = textureLod(hiZBuffer, vec2(boundingRect[1].x, boundingRect[0].y), lod).x;
    float maxDepth = max(max(samples.x, samples.y), max(samples.z, samples.w));
    
    // if the instance depth is bigger than the depth in the texture discard the instance
    return (instanceDepth > maxDepth) ? 0 : 1;
}

out mat4 modelMatrix;
flat out int objectVisible;
subroutine uniform cullRoutineType cullRoutine;

void main()
{
    modelMatrix = model;
    objectVisible = cullRoutine();
}
