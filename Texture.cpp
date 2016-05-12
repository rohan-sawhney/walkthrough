#include "Texture.h"

Texture::Texture()
{
    
}

void Texture::setup()
{
    renderTexture.setup(image);
}

void Texture::reset()
{
    renderTexture.reset();
}