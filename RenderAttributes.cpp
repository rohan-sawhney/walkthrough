#include "RenderAttributes.h"
#include "Instance.h"

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

void TransformBufferManager::setup(const std::vector<Instance>& instances, const bool& loadTransforms)
{
    size_t n = 0;
    GLuint index = 0;
    std::vector<size_t> bufferSizes;
    const size_t maxBufferSize = sizeof(float) * GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
    
    // determine minimum tbos needed
    // NOTE: assumes elements4x4 * instances[i].transforms.size() < GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
    for (size_t i = 0; i < instances.size(); i++) {
        size_t tSize = sizeof(Eigen::Matrix4f) * instances[i].transforms.size();
        if (n + tSize < maxBufferSize) {
            n += tSize;
        
        } else {
            tbos.push_back(0);
            bufferSizes.push_back(n);
            n = tSize;
            index = 0;
        }
        
        tboMap[i] = std::make_pair(tbos.size(), n-tSize);
        indexMap[i] = index;
        index++;
    }
    tbos.push_back(0);
    bufferSizes.push_back(n);
    
    // generate and bind tbos
    GLenum usage = loadTransforms ? GL_STATIC_DRAW : GL_DYNAMIC_COPY;
    glGenBuffers((GLsizei)tbos.size(), &tbos[0]);
    for (size_t i = 0; i < tbos.size(); i++) {
        glBindBuffer(GL_TEXTURE_BUFFER, tbos[i]);
        glBufferData(GL_TEXTURE_BUFFER, bufferSizes[i], NULL, usage);
    }
    
    if (loadTransforms) {
        for (size_t i = 0; i < instances.size(); i++) {
            const std::vector<Eigen::Matrix4f>& transforms(instances[i].transforms);
            size_t size = sizeof(Eigen::Matrix4f) * transforms.size();
            
            // bind instance data
            glBindBuffer(GL_ARRAY_BUFFER, tbos[tboMap[i].first]);
            void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, tboMap[i].second, size, GL_MAP_WRITE_BIT);
            memcpy(ptr, &transforms[0], size);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
    }
}

void TransformBufferManager::setInstanceBufferData(const size_t& instanceIndex, const size_t& count,
                                                   TransformBufferData& data)
{
    data.tbo = tbos[tboMap[instanceIndex].first];
    data.offset = tboMap[instanceIndex].second;
    data.count = count;
    data.size = sizeof(Eigen::Matrix4f) * count;
}

void TransformBufferManager::reset()
{
    glDeleteBuffers((GLsizei)tbos.size(), &tbos[0]);
    
    tbos.clear();
    tboMap.clear();
    indexMap.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullMesh::CullMesh()
{
    
}

void CullMesh::setup(const TransformBufferData& data)
{
    // generate and bind vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // bind tbo and set vertex attribute pointers for tbo data
    glBindBuffer(GL_ARRAY_BUFFER, data.tbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)data.offset);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + sizeof(Eigen::Vector4f)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + 2*sizeof(Eigen::Vector4f)));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + 3*sizeof(Eigen::Vector4f)));
    
    // generate query
    glGenQueries(1, &query);
    
    // unbind vao
    glBindVertexArray(0);
}

int CullMesh::queryCount() const
{
    GLint count;
    glGetQueryObjectiv(query, GL_QUERY_RESULT, &count);
    
    return (int)count;
}

void CullMesh::cull(const Shader& shader, const TransformBufferData& data) const
{
    // set uniforms
    const Eigen::Vector3f& min(boundingBox.min);
    const Eigen::Vector3f& max(boundingBox.max);
    glUniform3f(glGetUniformLocation(shader.program, "boxMin"), min.x(), min.y(), min.z());
    glUniform3f(glGetUniformLocation(shader.program, "boxMax"), max.x(), max.y(), max.z());
    
    // bind culled texture buffer as the target for transform feedback
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, data.tbo, data.offset, data.size);
    
    // bind vao
    glBindVertexArray(vao);
    
    // start transform feedback
    glBeginTransformFeedback(GL_POINTS);
    glBeginQuery(GL_PRIMITIVES_GENERATED, query);
    glDrawArrays(GL_POINTS, 0, (GLsizei)data.count);
    glEndQuery(GL_PRIMITIVES_GENERATED);
    glEndTransformFeedback();
}

void CullMesh::reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteQueries(1, &query);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderMesh::RenderMesh(const Material& material0, const RenderTexture& renderTexture0):
material(material0),
renderTexture(renderTexture0)
{
    
}

void RenderMesh::setup(const TransformBufferData& data)
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
    glBindBuffer(GL_ARRAY_BUFFER, data.tbo);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f), (GLvoid*)data.offset);
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + 2*sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(5, 1);
    
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Eigen::Matrix4f),
                          (GLvoid*)(data.offset + 3*sizeof(Eigen::Vector4f)));
    glVertexAttribDivisor(6, 1);
    
    // unbind vao
    glBindVertexArray(0);
}

void RenderMesh::setDefaultDrawSettings(const Shader& shader, bool cullBackFaces) const
{
    if (material.tIndex != -1) {
        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture.index);
        glUniform1i(glGetUniformLocation(shader.program, "tex"), 0);
        glUniform1i(glGetUniformLocation(shader.program, "hasTexture"), 1);
        
    } else {
        glUniform1i(glGetUniformLocation(shader.program, "hasTexture"), 0);
    }
    
    // set diffuse color
    glUniform4f(glGetUniformLocation(shader.program, "objectColor"),
                material.color.x(), material.color.y(), material.color.z(), material.alpha);
    
    // disable depth writing for transparent meshes
    if (material.alpha < 1.0) {
        glDepthMask(GL_FALSE);
        cullBackFaces = false;
    }
    
    // cull back faces
    if (cullBackFaces) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
}

void RenderMesh::setDefaultDrawSettings() const
{
    // unbind texture
    if (material.tIndex != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    glDepthMask(GL_TRUE);
}

void RenderMesh::draw(const Shader& shader, bool cullBackFaces, const int& visibleTransforms) const
{
    glBindVertexArray(vao);
    setDefaultDrawSettings(shader, cullBackFaces);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0, (GLsizei)visibleTransforms);
    setDefaultDrawSettings();
}

void RenderMesh::reset()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}
