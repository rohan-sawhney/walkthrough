#include "Skybox.h"
#include "Shader.h"

GLfloat vertices[] = {
    -350.0f,  350.0f, -350.0f,
    -350.0f, -350.0f, -350.0f,
    350.0f, -350.0f, -350.0f,
    350.0f, -350.0f, -350.0f,
    350.0f,  350.0f, -350.0f,
    -350.0f,  350.0f, -350.0f,
    
    -350.0f, -350.0f,  350.0f,
    -350.0f, -350.0f, -350.0f,
    -350.0f,  350.0f, -350.0f,
    -350.0f,  350.0f, -350.0f,
    -350.0f,  350.0f,  350.0f,
    -350.0f, -350.0f,  350.0f,
    
    350.0f, -350.0f, -350.0f,
    350.0f, -350.0f,  350.0f,
    350.0f,  350.0f,  350.0f,
    350.0f,  350.0f,  350.0f,
    350.0f,  350.0f, -350.0f,
    350.0f, -350.0f, -350.0f,
    
    -350.0f, -350.0f,  350.0f,
    -350.0f,  350.0f,  350.0f,
    350.0f,  350.0f,  350.0f,
    350.0f,  350.0f,  350.0f,
    350.0f, -350.0f,  350.0f,
    -350.0f, -350.0f,  350.0f,
    
    -350.0f,  350.0f, -350.0f,
    350.0f,  350.0f, -350.0f,
    350.0f,  350.0f,  350.0f,
    350.0f,  350.0f,  350.0f,
    -350.0f,  350.0f,  350.0f,
    -350.0f,  350.0f, -350.0f,
    
    -350.0f, -350.0f, -350.0f,
    -350.0f, -350.0f,  350.0f,
    350.0f, -350.0f, -350.0f,
    350.0f, -350.0f, -350.0f,
    -350.0f, -350.0f,  350.0f,
    350.0f, -350.0f,  350.0f
};

Skybox::Skybox()
{
    
}

void Skybox::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    // load data
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    
    // set vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
}

void Skybox::setupTexture(const std::string& path)
{
    std::vector<const GLchar*> faces = {"right.jpg", "left.jpg", "top.jpg",
                                        "bottom.jpg", "back.jpg", "front.jpg"};
    
    // generate id
    glGenTextures(1, &tIndex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tIndex);
    
    // write images
    for (size_t i = 0; i < faces.size(); i++) {
        Image image(path + faces[i]);
        image.magick("RGB");
        
        Blob blob;
        image.write(&blob);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLuint)i, 0, GL_COMPRESSED_RGB,
                     (GLsizei)image.columns(), (GLsizei)image.rows(), 0,
                     GL_RGB, GL_UNSIGNED_BYTE, blob.data());
    }
    
    // set parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Skybox::load(const std::string& path)
{
    setupMesh();
    setupTexture(path);
}

void Skybox::draw(Shader& shader) const
{
    shader.use();
    glDepthFunc(GL_LEQUAL);
    
    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.program, "skybox"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tIndex);
    
    // draw skybox
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // unbind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Skybox::reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &tIndex);
}
