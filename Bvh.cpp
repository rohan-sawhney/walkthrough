#include "Bvh.h"
#include "WorldMesh.h"
#include <stack>

struct NodeEntry {
    // parent id
    int parentId;
    
    // range of objects covered by the node
    int startId, endId;
};

struct TraversalEntry {
    // constructor
    TraversalEntry(const int id0, const float d0) : id(id0), d(d0) {}
    
    // id
    int id;
    
    // distance
    float d;
};

Bvh::Bvh(const int& leafSize0):
leafSize(leafSize0)
{
    
}

void Bvh::build(std::vector<WorldMesh> *worldMeshes0)
{
    worldMeshes = worldMeshes0;
    nodeCount = 0;
    leafCount = 0;
    flatTree.clear();
    
    int count = (int)worldMeshes->size();
    
    NodeEntry nodeEntry;
    nodeEntry.parentId = -1;
    nodeEntry.startId = 0;
    nodeEntry.endId = count;
    
    std::stack<NodeEntry> stack;
    stack.push(nodeEntry);
    
    Node node;
    flatTree.reserve(count * 2);
    
    while (!stack.empty()) {
        // pop item off the stack and create a node
        nodeEntry = stack.top();
        stack.pop();
        int startId = nodeEntry.startId;
        int endId = nodeEntry.endId;
        
        nodeCount++;
        node.startId = startId;
        node.range = endId - startId;
        node.rightOffset = 2;
        
        // calculate bounding box
        BoundingBox boundingBox((*worldMeshes)[startId].boundingBox);
        BoundingBox boundingCentroid((*worldMeshes)[startId].cm);
        for (int i = startId+1; i < endId; i++) {
            boundingBox.expandToInclude((*worldMeshes)[i].boundingBox);
            boundingCentroid.expandToInclude((*worldMeshes)[i].cm);
        }
        node.boundingBox = boundingBox;
        
        // if node is a leaf
        if (node.range <= leafSize) {
            node.rightOffset = 0;
            leafCount++;
        }
        
        flatTree.push_back(node);
        
        // compute parent's rightOffset
        if (nodeEntry.parentId != -1) {
            flatTree[nodeEntry.parentId].rightOffset--;
            
            if (flatTree[nodeEntry.parentId].rightOffset == 0) {
                flatTree[nodeEntry.parentId].rightOffset = nodeCount - 1 - nodeEntry.parentId;
            }
        }
        
        // if a leaf, no need to subdivide
        if (node.rightOffset == 0) {
            continue;
        }
        
        // find the center of the longest dimension
        int maxDimension = boundingCentroid.maxDimension();
        float splitCoord = 0.5 * (boundingCentroid.min[maxDimension] +
                                   boundingCentroid.max[maxDimension]);

        
        // partition faces
        int mid = startId;
        for (int i = startId; i < endId; i++) {
            if ((*worldMeshes)[i].cm[maxDimension] < splitCoord) {
                std::swap((*worldMeshes)[i], (*worldMeshes)[mid]);
                mid ++;
            }
        }
        
        // in case of a bad split
        if (mid == startId || mid == endId) {
            mid = startId + (endId - startId) / 2;
        }
        
        // push right child
        nodeEntry.startId = mid;
        nodeEntry.endId = endId;
        nodeEntry.parentId = nodeCount - 1;
        stack.push(nodeEntry);
        
        // push left child
        nodeEntry.startId = startId;
        nodeEntry.endId = mid;
        nodeEntry.parentId = nodeCount - 1;
        stack.push(nodeEntry);
    }
}

std::vector<int> Bvh::getIntersection(const Eigen::Vector3f& p) const
{
    int id = 0;
    int closer, further;
    bool hit0 = false;
    bool hit1 = false;
    float dist1 = 0.0, dist2 = 0.0;
    std::vector<int> intersections;
    
    TraversalEntry t(id, -INFINITY);
    BoundingBox bbox;
    
    std::stack<TraversalEntry> stack;
    stack.push(t);
    
    while (!stack.empty()) {
        TraversalEntry t = stack.top();
        id = t.id;
        stack.pop();
        
        const Node &node(flatTree[id]);
        // node is a leaf
        if (node.rightOffset == 0) {
            for (int i = 0; i < node.range; i++) {
                intersections.push_back(node.startId+i);
            }
            
        } else { // not a leaf
            // check for intersection with bounding box
            hit0 = flatTree[id+1].boundingBox.intersect(p, dist1);
            hit1 = flatTree[id+node.rightOffset].boundingBox.intersect(p, dist2);
            
            // hit both bounding boxes
            if (hit0 && hit1) {
                closer = id + 1;
                further = id + node.rightOffset;
                
                if (dist2 < dist1) {
                    std::swap(dist1, dist2);
                    std::swap(closer, further);
                }
                
                // push farther node first
                stack.push(TraversalEntry(further, dist2));
                stack.push(TraversalEntry(closer, dist1));
                
            } else if (hit0) {
                stack.push(TraversalEntry(id+1, dist1));
                
            } else if (hit1) {
                stack.push(TraversalEntry(id+node.rightOffset, dist2));
            }
        }
    }
    
    return intersections;
}
