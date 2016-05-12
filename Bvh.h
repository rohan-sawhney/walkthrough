#ifndef BVH_H
#define BVH_H

#include "Types.h"
#include "BoundingBox.h"

class Node {
public:
    // member variables
    BoundingBox boundingBox;
    int startId, range, rightOffset;
};

class Bvh {
public:
    // constructor
    Bvh(const int& leafSize0 = 1);
    
    // builds the bvh
    void build(std::vector<WorldMesh> *worldMeshes0);
    
    // returns meshes containing point
    std::vector<int> getIntersection(const Eigen::Vector3f& p) const;
    
private:
    // member variables
    int nodeCount, leafCount, leafSize;
    std::vector<WorldMesh> *worldMeshes;
    std::vector<Node> flatTree;
};

#endif
