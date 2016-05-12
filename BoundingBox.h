#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "Types.h"

class BoundingBox {
public:
    // default constructor
    BoundingBox();
    
    // initialize with specified components
    BoundingBox(const Eigen::Vector3f& min0, const Eigen::Vector3f& max0);
    
    // initialize with specified components
    BoundingBox(const Eigen::Vector3f& p);
    
    // expand bounding box to include point/ bbox
    void expandToInclude(const Eigen::Vector3f& p);
    void expandToInclude(const BoundingBox& b);

    // return the max dimension
    int maxDimension() const;
    
    // check if bounding box and face intersect
    bool intersect(const Eigen::Vector3f& o, const Eigen::Vector3f& d, float& dist) const;
    
    // check if point is contained in bounding box
    bool intersect(const Eigen::Vector3f& p, float& dist) const;
    
    // check if bounding boxes intersect
    bool intersect(const BoundingBox& boundingBox, float& dist) const;
    
    // returns center
    Eigen::Vector3f center() const;
    
    // member variables
    Eigen::Vector3f min;
    Eigen::Vector3f max;
    Eigen::Vector3f extent;
};

#endif