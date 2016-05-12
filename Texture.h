#ifndef TEXTURE_H
#define TEXTURE_H

#include "Types.h"
#include "RenderAttributes.h"

class Texture {
public:
    // constructor
    Texture();
    
    // set up
    void setup();
    
    // reset
    void reset();
    
    // name
    std::string name;
    
    // image
    Image image;
    
    // render texture
    RenderTexture renderTexture;
};

#endif
