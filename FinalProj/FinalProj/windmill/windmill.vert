#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

uniform mat4 MVP;

out vec3 color;
out vec2 uv;

void main() {
    uv = inUV;

    // Pass vertex color to the fragment shader
    color = inColor;

    // Calculate position using MVP matrix
    gl_Position = MVP * vec4(inPosition, 1.0);
}