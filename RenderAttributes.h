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

struct TransformBufferData {
    GLuint tbo;
    size_t offset;
    size_t count;
    size_t size;
};

class TransformBufferManager {
public:
    // set up
    static void setup(const std::vector<Instance>& instances);
    
    // sets instance tbo and offset
    static void setInstanceBufferData(const size_t& instanceIndex, const size_t& count, TransformBufferData& data);
    
    // reset
    static void reset();
    
private:
    // member variables
    static std::vector<GLuint> tbos;
    static std::unordered_map<size_t, std::pair<size_t, size_t>> tboMap;
    static std::unordered_map<size_t, GLuint> indexMap;
};

class CullMesh {
public:
    // constructor
    CullMesh();
    
    // set up
    void setup(const std::vector<Eigen::Matrix4f>& transforms);
    
    // returns query count
    int queryCount() const;
    
    // cull
    void cull(const Shader& shader, const TransformBufferData& data) const;
    
    // reset
    void reset();
    
    // member variable
    BoundingBox boundingBox;
    
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
    void setup(const TransformBufferData& data);
    
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
