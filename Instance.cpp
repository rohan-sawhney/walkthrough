#include "Instance.h"

Instance::Instance()
{
    
}

void Instance::setup(const std::vector<Material>& materials, const std::vector<Texture>& textures)
{
    mesh.setup(materials, textures, transforms, cullData, renderData);
}

void Instance::cull(const Shader& shader) const
{
    mesh.cull(shader, renderData);
}

void Instance::draw(const Shader& shader) const
{
    mesh.draw(shader);
}

void Instance::reset()
{
    mesh.reset();
}
