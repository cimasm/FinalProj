#version 330 core

in vec3 color;
in vec2 uv;

uniform sampler2D textureSampler;

out vec3 finalColor;

void main() {
    vec3 textureColor = texture(textureSampler, uv).rgb;

    // Combine texture color with vertex color
    finalColor = color * textureColor;
}