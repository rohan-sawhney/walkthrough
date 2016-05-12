#ifndef SKYBOX_H
#define SKYBOX_H

#include "Types.h"

class Skybox {
public:
    // constructor
    Skybox();
    
    // load
    void load(const std::string& path);
    
    // draw
    void draw(Shader& shader) const;
    
    // reset
    void reset();
    
private:
    // setup mesh
    void setupMesh();
    
    // setup texture
    void setupTexture(const std::string& path);
    
    // member variables
    GLuint vao;
    GLuint vbo;
    GLuint tIndex;
};

#endif
