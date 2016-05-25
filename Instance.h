#ifndef INSTANCE_H
#define INSTANCE_H

#include "Types.h"
#include "Mesh.h"

class Instance {
public:
    // constructor
    Instance();
    
    // set up
    void setup(const std::vector<Material>& materials, const std::vector<Texture>& textures);
    
    // cull
    void cull(const Shader& shader) const;
    
    // draw
    void draw(const Shader& shader) const;
    
    // reset
    void reset();
    
    // member variables
    Mesh mesh;
    std::vector<Eigen::Matrix4f> transforms;
    TransformBufferData cullData;
    TransformBufferData renderData;
};

#endif
