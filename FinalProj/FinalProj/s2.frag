#version 330 core

in vec3 fragColor;
in vec2 UV;
in vec3 fragNormal;
in vec3 fragPosition;

uniform sampler2D textureSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 viewPosition;

out vec4 color;

void main() {
    //vec3 normal = normalize(fragNormal);
    //vec3 lightDir = normalize(lightPosition - fragPosition);
    //float diff = max(dot(normal, lightDir), 0.0); // Diffuse only
    //color = vec4(fragPosition * 0.01, 1.0); // Scale to fit within [0, 1]


    // Normalize input vectors
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPosition - fragPosition);
    vec3 viewDir = normalize(viewPosition - fragPosition);

    // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightIntensity;

    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightIntensity;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightIntensity;



    // Combine results
    vec3 result = (ambient + diffuse + specular) * texture(textureSampler, UV).rgb;
    color = vec4(result, 1.0);
}
