#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 objectColor;
uniform sampler2D texture_diffuse1; // Assumes a single diffuse texture
uniform bool hasTexture;

void main()
{
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (hasTexture ? texture(texture_diffuse1, TexCoords).rgb : objectColor) * diffuse;
    FragColor = vec4(result, 1.0);
}