#ifndef WORLD_MESH_H
#define WORLD_MESH_H

#include "Types.h"
#include "Mesh.h"

class WorldMesh {
public:
    // constructor
    WorldMesh(const Mesh *mesh0, Eigen::Matrix4f *transform0);
    
    // compute features
    void computeFeatures();
    
    // shift
    void shift(const Eigen::Vector3f& p);
    
    // member variables
    const Mesh *mesh;
    Eigen::Matrix4f *transform;
    Eigen::Vector3f cm;
    BoundingBox boundingBox;
};

#endif
