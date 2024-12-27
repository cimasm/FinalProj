#version 330 core

in vec3 fragColor;
in vec2 UV;
in vec3 fragNormal;
in vec3 fragPosition;
in vec4 fragPosLightSpace;

uniform sampler2D textureSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 viewPosition;
uniform sampler2D depthMap;

out vec3 color;


float ShadowCalculation(vec4 fragPosLightSpace) {
    // Transform fragment position into light space
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range

    // Check if projCoords are within the depth texture bounds
    if (fragPosition.y > lightPosition.y)
        return 1.0; // No shadow if outside light's view frustum     // this is why everything above lights y pos is bright

    // Get closest depth from light's perspective
    float closestDepth = texture(depthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Shadow calculation with small bias to prevent acne
    float bias = 0.67; // 1e-3;
    return (currentDepth > closestDepth + bias) ? 0.2 : 1.0;
}



void main() {
    // Calculate shadow factor
    float shadow = ShadowCalculation(fragPosLightSpace);

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
    color = result; // * shadow;
}