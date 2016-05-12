#include "Shader.h"
#include <fstream>

Shader::Shader()
{
    
}

bool readShaderCode(const std::string& path, std::string& code)
{
    std::ifstream in(path.c_str());
    
    if (in.is_open()) {
        std::stringstream stream;
        stream << in.rdbuf();
        
        code = stream.str();
        in.close();
        
    } else {
        return false;
    }
    
    return true;
}

void Shader::setup(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
{
    std::string vertexCode, fragmentCode, geometryCode;
    
    if (readShaderCode(vertexPath, vertexCode) && readShaderCode(fragmentPath, fragmentCode)) {
        
        GLint success;
        GLchar infoLog[512];
        
        // compile vertex shader
        const GLchar *vShaderCode = vertexCode.c_str();
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "Error: " << infoLog << std::endl;
        }
        
        // compile fragment shader
        const GLchar *fShaderCode = fragmentCode.c_str();
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "Error: " << infoLog << std::endl;
        }
        
        if (readShaderCode(geometryPath, geometryCode)) {
            // compile fragment shader
            const GLchar *gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(geometry, 512, NULL, infoLog);
                std::cout << "Error: " << infoLog << std::endl;
            }
        }
        
        // compile shader program
        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        if (!geometryPath.empty()) glAttachShader(program, geometry);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "Error: " << infoLog << std::endl;
        }
    }
}

void Shader::use()
{
    glUseProgram(program);
}

void Shader::reset()
{
    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    
    glDetachShader(program, fragment);
    glDeleteShader(fragment);
    
    glDetachShader(program, geometry);
    glDeleteShader(geometry);
    
    glDeleteProgram(program);
}