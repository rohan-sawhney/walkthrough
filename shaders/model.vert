#version 400 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 model;

out vec3 fragPosition;
out vec3 normal;
out vec2 texCoords;

layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
};

void main()
{
    gl_Position = projection * view * model * vec4(inPosition, 1.0f);
    fragPosition = vec3(model * vec4(inPosition, 1.0f));
    normal = normalize(vec3(projection * vec4(mat3(transpose(inverse(view * model))) * inNormal, 1.0)));
    texCoords = inTexCoords;
}