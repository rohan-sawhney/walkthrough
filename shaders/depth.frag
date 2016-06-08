#version 400 core
in vec2 texCoords;
out vec4 color;

uniform sampler2D depthTexture;
uniform float mipLevel;

float linearizeDepth()
{
    float n = 1.0;          // camera z near
    float f = 1000.0;		// camera z far
    float z = textureLod(depthTexture, texCoords, mipLevel).r;
    return (2.0 * n) / (f + n - z * (f - n));
}

void main(void)
{
    float depth = linearizeDepth();
	color = vec4(1.0, 1.0, 1.0, 1.0) * depth;
}

