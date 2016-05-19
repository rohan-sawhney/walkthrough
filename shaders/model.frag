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
    // ambient
    float ambientStrength = 0.4f;
    
    // diffuse
    float diffuseStrength = 1.0f;
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);
    
    // specular
    float specularStrength = 0.5f;
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 32);
    
    float strength = ambientStrength + diffuseStrength * diff + specularStrength * spec;
    
    color = vec4(strength * light.color, 1.0f) * objectColor;
    if (hasTexture == 1) color = color * texture(tex, texCoords);
}
