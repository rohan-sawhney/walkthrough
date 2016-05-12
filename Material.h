#ifndef MATERIAL_H
#define MATERIAL_H

#include "Types.h"

class Material {
public:
    // constructor
    Material();
    
    // color
    Eigen::Vector3f color;
    
    // alpha
    float alpha;
    
    // texture index
    int tIndex;
};

#endif
