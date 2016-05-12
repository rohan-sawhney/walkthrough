#version 400 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out VS_OUT {
    vec3 normal;
} vs_out;

layout (std140) uniform Transform {
    mat4 projection;
    mat4 view;
};

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(inPosition, 1.0f);
    vs_out.normal = normalize(vec3(projection * vec4(mat3(transpose(inverse(view * instanceMatrix))) * inNormal, 1.0)));
}
