#ifndef SHADER_H
#define SHADER_H

#include "Types.h"

class Shader {
public:
    // constructor
    Shader(const std::string& dir0);
    
    // set up
    void setup(const std::string& vertexFile,
               const std::string& geometryFile,
               const std::string& fragmentFile);
    
    // link
    void link();
    
    // use
    void use();
    
    // reset
    void reset();
    
    // member variable
    GLuint program;
    
private:
    // reads shader code
    bool readShaderCode(const std::string& file, std::string& code);
    
    // compiles shader
    GLuint compileShader(const std::string& file, GLenum type);
    
    // member variables
    std::string dir;
    GLuint vertex;
    GLuint fragment;
    GLuint geometry;
};

#endif
