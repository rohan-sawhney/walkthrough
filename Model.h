#ifndef MODEL_H
#define MODEL_H

#include "Types.h"
#include "Instance.h"

class Model {
public:
    // constructor
    Model();
    
    // load 
    bool load(const std::string& path);
    
    // cull
    void cull(Shader& shader) const;
    
    // draw
    void draw(Shader& shader) const;
    
    // reset
    void reset();
    
private:
    // setup textures
    void setupTextures();
 
    // create mirrored instances
    void createMirroredInstances();
    
    // separates transparent instances
    void separateTransparentInstances();
    
    // center model
    void centerModel();
    
    // setup instances
    void setupInstances();
    
    // member variables
    std::vector<Instance> instances;
    std::vector<Material> materials;
    std::vector<Texture> textures;
};

#endif
