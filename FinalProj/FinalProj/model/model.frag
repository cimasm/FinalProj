#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D modelTexture;
uniform bool hasTexture;

out vec4 FragColor;

void main() {
    // Normalize input normal
    vec3 normal = normalize(Normal);

    // Calculate lighting vectors
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Get base color
    vec3 baseColor;
    if (hasTexture) {
        baseColor = texture(modelTexture, TexCoords).rgb;
    } else {
        baseColor = objectColor;
    }

    // Combine lighting components
    vec3 result = (ambient + diffuse + specular) * baseColor;

    // Output final color
    FragColor = vec4(result, 1.0);
}