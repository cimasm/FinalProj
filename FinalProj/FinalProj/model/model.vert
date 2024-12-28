#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

uniform mat4 model;
uniform mat4 vp;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    // Calculate the position in clip space
    gl_Position = vp * model * vec4(inPosition, 1.0);

    // Calculate fragment position in world space
    FragPos = vec3(model * vec4(inPosition, 1.0));

    // Transform normal vector to world space
    Normal = mat3(transpose(inverse(model))) * inNormal;

    // Pass to frag shader
    TexCoords = inTexCoords;
}