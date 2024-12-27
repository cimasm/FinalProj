#version 330 core

// Input attributes matching our Model class vertex setup
layout (location = 0) in vec3 aPosition;  // Vertex positions
layout (location = 1) in vec3 aNormal;    // Vertex normals
layout (location = 2) in vec2 aTexCoords; // Texture coordinates

// Output data to pass to the fragment shader
out vec3 FragPos;      // Fragment position in world space
out vec3 Normal;       // Normal vector in world space
out vec2 TexCoords;    // Texture coordinates to pass through

// Transformation matrices
uniform mat4 model;         // Model matrix - positions the model in the world
uniform mat4 vp; // View projection matrix

void main() {
    // Calculate the position in clip space for final vertex position
    gl_Position = vp * model * vec4(aPosition, 1.0);

    // Calculate fragment position in world space for lighting
    FragPos = vec3(model * vec4(aPosition, 1.0));

    // Transform normal vector to world space
    // We use transpose(inverse(model)) to handle non-uniform scaling properly
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates directly to fragment shader
    TexCoords = aTexCoords;
}