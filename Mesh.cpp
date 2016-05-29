#include "Mesh.h"

Mesh::Mesh():
closed(false)
{
    
}

Eigen::Vector3f Mesh::cm()
{
    Eigen::Vector3f cm = Eigen::Vector3f::Zero();
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        cm += v->position;
    }
    cm /= (float)vertices.size();
    
    return cm;
}

void Mesh::setRenderMeshes(std::vector<RenderMesh>& renderMeshes, const int& cullIndex)
{
    // create render meshes
    std::unordered_map<int, int> materialMap;
    std::unordered_map<int, std::unordered_map<int, int>> vertexMap;
    
    for (FaceCIter f = faces.begin(); f != faces.end(); f++) {
        // create render mesh if it doesnt exist
        if (materialMap.find(f->mIndex) == materialMap.end()) {
            materialMap[f->mIndex] = (int)renderMeshes.size();
            renderMeshes.push_back(RenderMesh(cullIndex, f->mIndex, closed));
        }
        
        // set vertices and indices
        const int& index(materialMap[f->mIndex]);
        for (int i = 0; i < 3; i++) {
            
            const int& vIndex(f->vIndices[i]);
            if (vertexMap[index].find(vIndex) == vertexMap[index].end()) {
                vertexMap[index][vIndex] = (int)renderMeshes[index].vertices.size();
                
                // insert vertex struct
                RenderVertex renderVertex;
                renderVertex.position = vertices[vIndex].position;
                renderVertex.normal = normals[vIndex];
                renderVertex.uv = uvs[vIndex];
                renderMeshes[index].vertices.push_back(renderVertex);
            }
            
            renderMeshes[index].indices.push_back(vertexMap[index][vIndex]);
        }
    }
}

void Mesh::flipOrientation()
{
    for (FaceIter f = faces.begin(); f != faces.end(); f++) {
        std::swap(f->vIndices[0], f->vIndices[1]);
    }
}
