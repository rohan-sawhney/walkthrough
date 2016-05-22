#ifndef RENDER_ATTRIBUTES_H
#define RENDER_ATTRIBUTES_H

#include "Types.h"
#include "Material.h"
#include "Shader.h"
#include "BoundingBox.h"

struct RenderVertex {
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f uv;
};

class RenderTexture {
public:
    RenderTexture();
    
    // set up
    void setup(Image& image);
    
    // reset
    void reset();
    
    // index
    GLuint index;
};

class CullMesh {
public:
    // constructor
    CullMesh();
    
    // set up
    void setup(const std::vector<Eigen::Matrix4f>& transforms);
    
    // cull
    void cull(const Shader& shader, const int& instanceCount) const;
    
    // returns visible transform count
    int queryVisibleTransforms() const;
    
    // reset
    void reset();
    
    // member variable
    BoundingBox boundingBox;
    GLuint culledTbo;
    
private:
    // member variables
    GLuint vao;
    GLuint tbo;
    GLuint query;
};

class RenderMesh {
public:
    // constructor
    RenderMesh(const Material& material0, const RenderTexture& renderTexture0);
    
    // set up
    void setup(const GLuint& tbo);
    
    // draw
    void draw(const Shader& shader, bool cullBackFaces, const int& visibleTransforms) const;
    
    // reset
    void reset();
    
    // member variables
    std::vector<RenderVertex> vertices;
    std::vector<GLuint> indices;
    const Material& material;
    const RenderTexture& renderTexture;
    
private:
    // prepare for drawing
    void setDefaultDrawSettings(const Shader& shader, bool cullBackFaces) const;
    
    // cleanup after drawing
    void setDefaultDrawSettings() const;
    
    // member variables
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

#endif
