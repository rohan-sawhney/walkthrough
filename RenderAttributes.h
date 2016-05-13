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

class RenderMesh {
public:
    // constructor
    RenderMesh(const Material& material0, const RenderTexture& renderTexture0);
    
    // set up
    void setup(const std::vector<Eigen::Matrix4f>& transforms);
        
    // prepare for drawing
    void initDrawing(const Shader& shader, bool cullBackFaces) const;

    // draw
    void draw(const int& instanceCount) const;
    
    // cleanup after drawing
    void endDrawing() const;
    
    // reset
    void reset();
    
    // member variables
    std::vector<RenderVertex> vertices;
    std::vector<GLuint> indices;
    const Material& material;
    const RenderTexture& renderTexture;
    BoundingBox boundingBox;
    
private:
    // member variables
    GLuint vao;
    GLuint vbo;
    GLuint instanceVbo;
    GLuint ebo;
    /*
    GLuint vbo;
    GLuint ebo;
    GLuint cullVao;
    GLuint drawVao;
    GLuint cullTbo;
    GLuint drawTbo;
    GLuint cullQuery;
    */
};

#endif
