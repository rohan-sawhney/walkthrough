#include "Model.h"
#include "MeshIO.h"
#include <map>

Model::Model():
offsetTransparent(0),
offsetOpen(0)
{
    
}

bool Model::load(const std::string& path)
{
    if (MeshIO::readGeometry(path, instances) && MeshIO::readMaterials(path, materials, textures)) {
        setupTextures();
        setupInstances();
        setupMeshes();
        
        return true;
    }
    
    return false;
}

void Model::cull(Shader& shader, const GLuint& mode)
{
    shader.use();
    glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &mode);
    glEnable(GL_RASTERIZER_DISCARD);
    
    for (size_t i = 0; i < cullMeshes.size(); i++) {
        cullMeshes[i].cull(shader);
    }
    
    glDisable(GL_RASTERIZER_DISCARD);
}

void Model::setMaterialSettings(const Shader& shader, const int& index) const
{
    static int mIndex = -1;
    
    if (index != mIndex) {
        const Material& material(materials[index]);
        
        // set color
        glUniform4f(glGetUniformLocation(shader.program, "objectColor"),
                    material.color.x(), material.color.y(), material.color.z(), material.alpha);
    
        if (material.tIndex == -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(glGetUniformLocation(shader.program, "hasTexture"), 0);
            
        } else {
            // bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[material.tIndex].renderTexture.index);
            glUniform1i(glGetUniformLocation(shader.program, "hasTexture"), 1);
        }
        
        mIndex = index;
    }
}

void Model::draw(Shader& shader, const bool& useMaterials)
{
    shader.use();
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        if (i == offsetTransparent) glDepthMask(GL_FALSE);
        if (i == offsetOpen) glDisable(GL_CULL_FACE);
        
        int visibleTransforms = cullMeshes[renderMeshes[i].cullIndex].queryCount();
        if (visibleTransforms > 0) {
            if (useMaterials) setMaterialSettings(shader, renderMeshes[i].mIndex);
            renderMeshes[i].draw(visibleTransforms);
        }
    }
    
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
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
            instances[index].mesh.flipOrientation();
            instances[i].transforms.clear();
            
            // assign transforms
            for (size_t j = 0; j < transforms.size(); j++) {
                if (transforms[j].determinant() > 0) instances[i].transforms.push_back(transforms[j]);
                else instances[index].transforms.push_back(transforms[j]);
            }
            index++;
        
        } else if (sign == -1) {
            instances[i].mesh.flipOrientation();
        }
    }
}

void Model::centerModel()
{
    // compute center of mass
    int totalMeshes = 0;
    Eigen::Vector3f cm = Eigen::Vector3f::Zero();
    
    for (size_t i = 0; i < instances.size(); i++) {
        const Eigen::Vector3f icm = instances[i].mesh.cm();
        
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
    centerModel();
}

bool Model::isOpaque(const RenderMesh& renderMesh)
{
    return materials[renderMesh.mIndex].alpha == 1.0;
}

bool Model::isClosed(const RenderMesh& renderMesh)
{
    return renderMesh.closed;
}

size_t Model::separateByFunc(size_t start, size_t end, bool (Model::*func)(const RenderMesh& renderMesh))
{
    while (start < end) {
        while ((this->*func)(renderMeshes[start]) && start < end) start++;
        while (!(this->*func)(renderMeshes[end]) && start < end) end--;
        
        if (start < end) {
            std::swap(renderMeshes[start], renderMeshes[end]);
            start++;
            end--;
        }
    }
    
    return end;
}

bool comparator(const RenderMesh& rm1, const RenderMesh& rm2)
{
    return rm1.mIndex < rm2.mIndex;
}

void Model::materialSort()
{
    std::sort(renderMeshes.begin(), renderMeshes.begin() + offsetOpen, comparator);
    std::sort(renderMeshes.begin() + offsetOpen, renderMeshes.begin() + offsetTransparent, comparator);
    std::sort(renderMeshes.begin() + offsetTransparent, renderMeshes.end(), comparator);
}

void Model::setupCullMeshes()
{
    for (size_t i = 0; i < cullMeshes.size(); i++) {
        cullMeshes[i].setup();
        
        // compute bounding box
        for (size_t j = 0; j < instances[i].mesh.vertices.size(); j++) {
            cullMeshes[i].boundingBox.expandToInclude(instances[i].mesh.vertices[j].position);
        }
    }
}

void Model::setupRenderMeshes()
{
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        renderMeshes[i].setup(cullMeshes[renderMeshes[i].cullIndex].culledTbo);
    }
}

void Model::setupMeshes()
{
    // create cull and render meshes
    for (size_t i = 0; i < instances.size(); i++) {
        cullMeshes.push_back(CullMesh(instances[i].transforms));
        instances[i].mesh.setRenderMeshes(renderMeshes, (int)i);
    }
    
    offsetTransparent = separateByFunc(0, renderMeshes.size()-1, &Model::isOpaque) + 1;
    offsetOpen = separateByFunc(0, offsetTransparent-1, &Model::isClosed) + 1;
    materialSort();
    setupCullMeshes();
    setupRenderMeshes();
}

void Model::reset()
{
    // reset cull meshes
    for (size_t i = 0; i < cullMeshes.size(); i++) {
        cullMeshes[i].reset();
    }
    
    // reset render meshes
    for (size_t i = 0; i < renderMeshes.size(); i++) {
        renderMeshes[i].reset();
    }

    // reset textures
    for (size_t i = 0; i < textures.size(); i++) {
        textures[i].reset();
    }
    
    // clear
    instances.clear();
    cullMeshes.clear();
    renderMeshes.clear();
    materials.clear();
    textures.clear();
    offsetTransparent = 0;
    offsetOpen = 0;
}
