#ifndef MESH_H
#define MESH_H

#include "Types.h"
#include "Vertex.h"
#include "Face.h"
#include "Texture.h"
#include "RenderAttributes.h"

class Mesh {
public:
    // default constructor
    Mesh();
    
    // assignment operator
    Mesh& operator=(const Mesh& mesh);
    
    // returns cm
    Eigen::Vector3f cm();
    
    // set up
    void setup(const std::vector<Material>& materials, const std::vector<Texture>& textures,
               const std::vector<Eigen::Matrix4f>& transforms, const TransformBufferData& data);
    
    // checks for transparency
    bool hasTransparency(const std::vector<Material>& materials) const;
    
    // flip orientation
    void flipOrientation();
    
    // cull
    void cull(const Shader& shader, const TransformBufferData& data) const;
    
    // draw mesh
    void draw(const Shader& shader) const;
    
    // reset
    void reset();
    
    // member variables
    std::vector<Vertex> vertices;
    std::vector<Eigen::Vector2f> uvs;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Face> faces;
    std::vector<int> mIndices;
    std::vector<Mesh> lods;
    bool closed;
    
private:
    // member variables
    CullMesh cullMesh;
    std::vector<RenderMesh> renderMeshes;
};

#endif