#ifndef VERTEX_H
#define VERTEX_H

#include "Types.h"

class Vertex {
public:
    // constructor
    Vertex(const Eigen::Vector3f& position0, const int& index0);
    
    // position
    Eigen::Vector3f position;
        
    // index
    int index;
};

#endif
