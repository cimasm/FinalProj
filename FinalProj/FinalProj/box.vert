#version 330 core

// Input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

// Output
out vec3 color;
out vec2 uv;


uniform mat4 MVP;

void main() {
    uv = inUV;

    // Pass vertex color to the fragment shader
    color = inColor;

    // Calculate position using MVP matrix
    gl_Position = MVP * vec4(inPosition, 1.0);
}