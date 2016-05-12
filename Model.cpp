#include "Model.h"
#include "MeshIO.h"
#include <map>

Model::Model()
{
    
}

bool Model::load(const std::string& path)
{
    if (MeshIO::readGeometry(path, instances) && MeshIO::readMaterials(path, materials, textures)) {
        setupTextures();
        setupInstances();
        
        return true;
    }
    
    return false;
}

void Model::draw(Shader& shader) const
{
    shader.use();
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].mesh.draw(shader, (int)instances[i].transforms.size());
    }
}

void Model::setupTextures()
{
    for (size_t i = 0; i < textures.size(); i++) {
        textures[i].setup();
    }
}

void Model::createMirroredInstances()
{
    size_t instanceCount = instances.size();
    size_t index = instanceCount;
    
    for (size_t i = 0; i < instanceCount; i++) {
        bool createInstance = false;
        std::vector<Eigen::Matrix4f> transforms = instances[i].transforms;
        
        // create new mirrored instance based on determinant test
        int sign = transforms[0].determinant() > 0 ? 1 : -1;
        for (size_t j = 1; j < transforms.size(); j++) {
            if (sign != (transforms[j].determinant() > 0 ? 1 : -1)) {
                createInstance = true;
                break;
            }
        }
        
        if (createInstance) {
            instances.push_back(Instance());
            instances[index].mesh = instances[i].mesh;
            instances[i].transforms.clear();
            
            // assign transforms
            for (size_t j = 0; j < transforms.size(); j++) {
                if (transforms[j].determinant() > 0) instances[i].transforms.push_back(transforms[j]);
                else instances[index].transforms.push_back(transforms[j]);
            }
            index++;
        }
    }
}

void Model::separateTransparentInstances()
{
    // separate opaque and transparent instances
    size_t left = 0, right = instances.size()-1;
    while (left < right) {
        while (!instances[left].mesh.hasTransparency(materials) && left < right) left++;
        while (instances[right].mesh.hasTransparency(materials) && left < right) right--;
        if (left < right) {
            std::swap(instances[left], instances[right]);
            left++;
            right--;
        }
    }
}

void Model::createWorldMeshes()
{
    for (size_t i = 0; i < instances.size(); i++) {
        for (size_t j = 0; j < instances[i].transforms.size(); j++) {
            worldMeshes.push_back(WorldMesh(&instances[i].mesh, &instances[i].transforms[j]));
        }
    }
}

void Model::centerModel()
{
    // compute center of mass
    Eigen::Vector3f cm = Eigen::Vector3f::Zero();
    for (size_t i = 0; i < worldMeshes.size(); i++) {
        worldMeshes[i].computeFeatures();
        cm += worldMeshes[i].cm;
    }
    cm /= (float)worldMeshes.size();
    
    // center mesh transforms around origin
    for (size_t i = 0; i < worldMeshes.size(); i++) {
        worldMeshes[i].shift(-cm);
    }
}

void Model::setupInstances()
{
    createMirroredInstances();
    separateTransparentInstances();
    createWorldMeshes();
    centerModel();
    bvh.build(&worldMeshes);
    
    for (size_t i = 0; i < instances.size(); i++) {
        if (instances[i].transforms[0].determinant() < 0) instances[i].mesh.flipOrientation();
        instances[i].mesh.setup(materials, textures, instances[i].transforms);
    }
}

void Model::reset()
{
    // reset meshes
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].mesh.reset();
    }

    // reset textures
    for (size_t i = 0; i < textures.size(); i++) {
        textures[i].reset();
    }
    
    // clear
    worldMeshes.clear();
    instances.clear();
    materials.clear();
    textures.clear();
}