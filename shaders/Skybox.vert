#version 400 core
layout (location = 0) in vec3 inPosition;

out vec3 texCoords;

layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
    vec2 viewport;
};

void main()
{
    gl_Position = projection * view * vec4(inPosition, 1.0);
    texCoords = inPosition;
}  