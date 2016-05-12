#include "WorldMesh.h"

WorldMesh::WorldMesh(const Mesh *mesh0, Eigen::Matrix4f *transform0):
mesh(mesh0),
transform(transform0),
cm(Eigen::Vector3f::Zero())
{
    
}

void WorldMesh::computeFeatures()
{
    for (VertexCIter v = mesh->vertices.begin(); v != mesh->vertices.end(); v++) {
        Eigen::Vector3f position = transform->block(0, 0, 3, 3) * v->position + transform->block(0, 3, 3, 1);
        cm += position;
        boundingBox.expandToInclude(position);
    }
    cm /= (float)mesh->vertices.size();
}

void WorldMesh::shift(const Eigen::Vector3f& p)
{
    transform->block(0, 3, 3, 1) += p;
    cm += p;
    boundingBox.min += p;
    boundingBox.max += p;
}
