#include "RenderAttributes.h"

RenderTexture::RenderTexture()
{
    
}

void RenderTexture::setup(Image& image)
{
    // generate id
    glGenTextures(1, &index);
    glBindTexture(GL_TEXTURE_2D, index);
    
    // write image
    Blob blob;
    image.write(&blob);
    glTexImage2D(GL_TEXTURE_2D, 0, image.matte() ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB,
                 (GLsizei)image.columns(), (GLsizei)image.rows(), 0,
                 image.matte() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, blob.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // set parameters
    float aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTexture::reset()
{
    glDeleteTextures(1, &index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullMesh::CullMesh(const std::vector<Eigen::Matrix4f>& transforms0):
transforms(transforms0),
count(-1)
{
    
}

void CullMesh::setup()
{
    // generate and bind vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // bind tbo and set vertex attribute pointers for tbo data
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizei)transforms.size() * sizeof(Eigen::Matrix4f),
                 &transforms[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(sizeof(Eigen::Vector4f)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(2*sizeof(Eigen::Vector4f)));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(3*sizeof(Eigen::Vector4f)));
    
    glGenBuffers(1, &culledTbo);
    glBindBuffer(GL_ARRAY_BUFFER, culledTbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizei)transforms.size() * sizeof(Eigen::Matrix4f), NULL, GL_DYNAMIC_COPY);
    
    // generate query
    glGenQueries(1, &query);
    
    // unbind vao
    glBindVertexArray(0);
}

void CullMesh::cull(const Shader& shader)
{
    // set count to -1
    count = -1;
    
    // set uniforms
    const Eigen::Vector3f& min(boundingBox.min);
    const Eigen::Vector3f& max(boundingBox.max);
    glUniform3f(glGetUniformLocation(shader.program, "boxMin"), min.x(), min.y(), min.z());
    glUniform3f(glGetUniformLocation(shader.program, "boxMax"), max.x(), max.y(), max.z());
    
    // bind culled texture buffer as the target for transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, culledTbo);
    
    // bind vao
    glBindVertexArray(vao);
    
    // start transform feedback
    glBeginTransformFeedback(GL_POINTS);
    glBeginQuery(GL_PRIMITIVES_GENERATED, query);
    glDrawArrays(GL_POINTS, 0, (GLsizei)transforms.size());
    glEndQuery(GL_PRIMITIVES_GENERATED);
    glEndTransformFeedback();
}

int CullMesh::queryCount()
{
    if (count == -1) glGetQueryObjectiv(query, GL_QUERY_RESULT, &count);
    return (int)count;
}

void CullMesh::reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &tbo);
    glDeleteBuffers(1, &culledTbo);
    glDeleteQueries(1, &query);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderMesh::RenderMesh(const int& cullIndex0, const int& mIndex0, const bool& closed0):
cullIndex(cullIndex0),
mIndex(mIndex0),
closed(closed0)
{
    
}

void RenderMesh::setup(const GLuint& tbo)
{
    // generate and bind vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // generate and bind ebo
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
    
    // generate and bind vbo
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizei)vertices.size() * sizeof(RenderVertex), &vertices[0], GL_STATIC_DRAW);
    
    // set vertex attribute pointers for vbo data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)offsetof(RenderVertex, normal));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (GLvoid*)offsetof(RenderVertex, uv));
    
    // bind tbo and set vertex attribute pointers for tbo data
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)0);
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(2*sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(5, 1);
    
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)(3*sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(6, 1);
    
    // unbind vao
    glBindVertexArray(0);
}

void RenderMesh::draw(const size_t& visibleTransforms) const
{
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0, (GLsizei)visibleTransforms);
}

void RenderMesh::reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}
