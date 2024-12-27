#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Output color
out vec4 FragColor;

// Material and light properties
uniform vec3 lightPos;        // Position of the light source
uniform vec3 viewPos;         // Camera position for specular calculation
uniform vec3 lightColor;      // Color of the light
uniform vec3 objectColor;     // Base color of the model (if no texture)
uniform sampler2D modelTexture; // Texture sampler
uniform bool hasTexture;      // Whether we're using a texture

void main() {
    // Normalize our input normal
    vec3 normal = normalize(Normal);

    // Calculate basic lighting vectors
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Ambient lighting (basic global illumination approximation)
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting (main directional lighting)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting (shiny reflections)
    float specularStrength = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Get base color either from texture or uniform
    vec3 baseColor;
    if (hasTexture) {
        baseColor = texture(modelTexture, TexCoords).rgb;
    } else {
        baseColor = objectColor;
    }

    // Combine all lighting components
    vec3 result = (ambient + diffuse + specular) * baseColor;

    // Output final color with gamma correction
    FragColor = vec4(result, 1.0);
}