#ifndef MODEL_H
#define MODEL_H

#include "Types.h"
#include "Instance.h"
#include "Material.h"
#include "Texture.h"

class Model {
public:
    // constructor
    Model();
    
    // load 
    bool load(const std::string& path);
    
    // cull
    void cull(Shader& shader);
    
    // draw
    void draw(Shader& shader);
    
    // reset
    void reset();
    
private:
    // setup textures
    void setupTextures();
 
    // create mirrored instances
    void createMirroredInstances();
    
    // correct orientation
    void correctOrientation();
    
    // center model
    void centerModel();
    
    // setup render meshes
    void setupRenderMeshes();
    
    // checks if render mesh is opaque
    bool isOpaque(const RenderMesh& renderMesh);
    
    // checks if render mesh is closed
    bool isClosed(const RenderMesh& renderMesh);
    
    // separate meshes by function
    size_t separateByFunc(size_t start, size_t end, bool (Model::*func)(const RenderMesh& renderMesh));
    
    // sort meshes by material
    void materialSort();
    
    // setup instances
    void setupInstances();
    
    // setup cull meshes
    void setupCullMeshes();
    
    // setup meshes
    void setupMeshes();
    
    // sets material settings
    void setMaterialSettings(const Shader& shader, const int& index) const;
    
    // member variables
    std::vector<Instance> instances;
    std::vector<CullMesh> cullMeshes;
    std::vector<RenderMesh> renderMeshes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    size_t offsetTransparent;
    size_t offsetOpen;
};

#endif
