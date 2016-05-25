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

void Model::cull(Shader& shader) const
{
    shader.use();
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].cull(shader);
    }
}

void Model::draw(Shader& shader) const
{
    shader.use();
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].draw(shader);
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

void Model::centerModel()
{
    // compute center of mass
    int totalMeshes = 0;
    Eigen::Vector3f cm = Eigen::Vector3f::Zero();
    for (size_t i = 0; i < instances.size(); i++) {
        
        Eigen::Vector3f icm = instances[i].mesh.cm();
        for (size_t j = 0; j < instances[i].transforms.size(); j++) {
            const Eigen::Matrix4f& transform(instances[i].transforms[j]);
            cm += (transform.block(0, 0, 3, 3) * icm + transform.block(0, 3, 3, 1));
            totalMeshes++;
        }
    }
    cm /= (float)totalMeshes;
    
    // center instance transforms around origin
    for (size_t i = 0; i < instances.size(); i++) {
        for (size_t j = 0; j < instances[i].transforms.size(); j++) {
            instances[i].transforms[j].block(0, 3, 3, 1) -= cm;
        }
    }
}

void Model::setupInstances()
{
    createMirroredInstances();
    separateTransparentInstances();
    centerModel();
    cullTBManager.setup(instances, true);
    renderTBManager.setup(instances, false);
    
    for (size_t i = 0; i < instances.size(); i++) {
        cullTBManager.setInstanceBufferData(i, instances[i].transforms.size(), instances[i].cullData);
        renderTBManager.setInstanceBufferData(i, instances[i].transforms.size(), instances[i].renderData);
        
        if (instances[i].transforms[0].determinant() < 0) instances[i].mesh.flipOrientation();
        instances[i].setup(materials, textures);
    }
}

void Model::reset()
{
    // reset meshes
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].reset();
    }

    // reset textures
    for (size_t i = 0; i < textures.size(); i++) {
        textures[i].reset();
    }
    
    // clear
    instances.clear();
    materials.clear();
    textures.clear();
    cullTBManager.reset();
    renderTBManager.reset();
}