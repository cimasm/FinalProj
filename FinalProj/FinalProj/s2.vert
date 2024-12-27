#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexNormal;

uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 lightSpaceMatrix;

out vec3 fragColor;
out vec2 UV;
out vec3 fragNormal;
out vec3 fragPosition;
out vec4 fragPosLightSpace;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1.0);
    fragColor = vertexColor;
    UV = vertexUV;
    fragPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    fragNormal = mat3(normalMatrix) * vertexNormal; // Transform the normal to world space

    fragPosLightSpace = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}