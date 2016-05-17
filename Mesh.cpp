#include "Mesh.h"

Mesh::Mesh():
closed(false)
{
    
}

Mesh& Mesh::operator=(const Mesh& mesh)
{
    vertices = mesh.vertices;
    uvs = mesh.uvs;
    normals = mesh.normals;
    faces = mesh.faces;
    mIndices = mesh.mIndices;
    closed = mesh.closed;
    
    return *this;
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

void Mesh::setup(const std::vector<Material>& materials, const std::vector<Texture>& textures,
                 const std::vector<Eigen::Matrix4f>& transforms)
{
    // create render meshes
    renderMeshes.reserve(mIndices.size());
    std::unordered_map<int, int> materialMap;
    std::unordered_map<int, std::unordered_map<int, int>> vertexMap;
    
    for (FaceCIter f = faces.begin(); f != faces.end(); f++) {
        // create render mesh if it doesnt exist
        if (materialMap.find(f->mIndex) == materialMap.end()) {
            materialMap[f->mIndex] = (int)renderMeshes.size();
            
            const Material& material(materials[f->mIndex]);
            renderMeshes.push_back(RenderMesh(material, textures[material.tIndex].renderTexture));
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
                renderMeshes[index].cm += renderVertex.position;
                renderMeshes[index].boundingBox.expandToInclude(renderVertex.position);
            }
            
            renderMeshes[index].indices.push_back((GLuint)vertexMap[index][vIndex]);
        }
    }
    
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        renderMeshes[i].cm /= (float)renderMeshes[i].vertices.size();
        renderMeshes[i].setup(transforms);
    }
    
    // setup lods
    for (size_t i = 0; i < lods.size(); i++) {
        lods[i].setup(materials, textures, transforms);
    }
}

bool Mesh::hasTransparency(const std::vector<Material>& materials) const
{
    for (size_t i = 0; i < mIndices.size(); i++) {
        if (materials[mIndices[i]].alpha < 1.0) {
            return true;
        }
    }
    
    return false;
}

void Mesh::flipOrientation()
{
    for (FaceIter f = faces.begin(); f != faces.end(); f++) {
        std::swap(f->vIndices[0], f->vIndices[1]);
    }
}

void Mesh::draw(Shader& shader, Shader& cullShader, const int& instanceCount) const
{
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        renderMeshes[i].draw(shader, cullShader, instanceCount, closed);
    }
}

void Mesh::reset()
{
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        renderMeshes[i].reset();
    }
}

