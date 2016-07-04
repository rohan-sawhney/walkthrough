#version 400 core
in vec3 fragPosition;
in vec3 normal;
in vec2 texCoords;

out vec4 color;

layout (std140) uniform Light {
    vec3 position;
    vec3 color;
} light;

uniform vec3 viewPosition;
uniform vec4 objectColor;
uniform sampler2D tex;
uniform int hasTexture;

void main()
{
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    
    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float specular = pow(max(dot(normal, halfwayDirection), 0.0), 32);
    
    float strength = 0.3 + 0.8*diffuse + 0.3*specular;
    
    color = vec4(strength * light.color, 1.0) * objectColor;
    if (hasTexture == 1) color = color * texture(tex, texCoords);
}
