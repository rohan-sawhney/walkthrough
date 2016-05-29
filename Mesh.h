#ifndef MESH_H
#define MESH_H

#include "Types.h"
#include "Vertex.h"
#include "Face.h"
#include "RenderAttributes.h"

class Mesh {
public:
    // default constructor
    Mesh();
        
    // returns cm
    Eigen::Vector3f cm();
    
    // set up
    void setRenderMeshes(std::vector<RenderMesh>& renderMeshes, const int& cullIndex);
    
    // flip orientation
    void flipOrientation();
        
    // member variables
    std::vector<Vertex> vertices;
    std::vector<Eigen::Vector2f> uvs;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Face> faces;
    std::vector<Mesh> lods;
    bool closed;
};

#endif