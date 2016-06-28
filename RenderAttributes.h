#ifndef RENDER_ATTRIBUTES_H
#define RENDER_ATTRIBUTES_H

#include "Types.h"
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
    CullMesh(const std::vector<Eigen::Matrix4f>& transforms0);
    
    // set up
    void setup();
    
    // cull
    void cull(const Shader& shader);
    
    // returns query count
    int queryCount();
    
    // reset
    void reset();
    
    // member variable
    BoundingBox boundingBox;
    const std::vector<Eigen::Matrix4f>& transforms;
    GLuint culledTbo;
    
private:
    // member variables
    GLuint vao;
    GLuint tbo;
    GLuint query;
    GLint count;
};

class RenderMesh {
public:
    // constructor
    RenderMesh(const int& instanceId0, const int& mIndex0, const bool& closed0);
    
    // set up
    void setup(const GLuint& tbo);
    
    // draw
    void draw(const size_t& visibleTransforms) const;
    
    // reset
    void reset();
    
    // member variables
    std::vector<RenderVertex> vertices;
    std::vector<GLuint> indices;
    int instanceId;
    int mIndex;
    bool closed;

private:
    // member variables
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

#endif
