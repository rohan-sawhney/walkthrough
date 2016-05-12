#ifndef FACE_H
#define FACE_H

#include "Types.h"

class Face {
public:
    // constructor
    Face();
    
    // overriden constructor
    Face(const Eigen::Vector3i& vIndices0, const int& mIndex0, const int& index0);
    
    // vertex indices
    Eigen::Vector3i vIndices;
    
    // material index
    int mIndex;
    
    // index
    int index;
};

#endif
