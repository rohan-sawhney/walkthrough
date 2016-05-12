#ifndef INSTANCE_H
#define INSTANCE_H

#include "Types.h"
#include "Mesh.h"

class Instance {
public:
    // constructor
    Instance();
    
    // member variables
    Mesh mesh;
    std::vector<Eigen::Matrix4f> transforms;
};

#endif
