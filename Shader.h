#ifndef SHADER_H
#define SHADER_H

#include "Types.h"

class Shader {
public:
    // constructor
    Shader();
    
    // set up
    void setup(const std::string& vertexPath, const std::string& fragmentPath,
               const std::string& geometryPath = "");
    
    // use
    void use();
    
    // reset
    void reset();
    
    // member variable
    GLuint program;
    
private:
    GLuint vertex;
    GLuint fragment;
    GLuint geometry;
};

#endif
