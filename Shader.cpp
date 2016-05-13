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

GLuint compileShader(const std::string& path, GLenum type)
{
    GLint success;
    GLchar infoLog[512];
    std::string code;
    GLuint shader = 0;
    
    if (readShaderCode(path, code)) {
        const GLchar *shaderCode = code.c_str();
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderCode, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "Error: " << infoLog << std::endl;
            shader = 0;
        }
    }
    
    return shader;
}

void Shader::setup(const std::string& vertexPath,
                   const std::string& geometryPath,
                   const std::string& fragmentPath)
{
    // compile shaders
    vertex = compileShader(vertexPath, GL_VERTEX_SHADER);
    geometry = compileShader(geometryPath, GL_GEOMETRY_SHADER);
    fragment = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    // create program and attach shaders
    program = glCreateProgram();
    if (vertex) glAttachShader(program, vertex);
    if (geometry) glAttachShader(program, geometry);
    if (fragment) glAttachShader(program, fragment);
}

void Shader::link()
{
    GLint success;
    GLchar infoLog[512];
    
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Error: " << infoLog << std::endl;
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