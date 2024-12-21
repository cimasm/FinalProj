#include <glad/gl.h>
#include "skybox.h"
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#include <stb/stb_image.h>
#include <iostream>
#define _USE_MATH_DEFINES

static GLuint LoadTextureSkybox(const char *texture_file_path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(texture_file_path, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        std::cerr << "Failed to load texture." << std::endl;
    }
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

// OpenGL buffers
GLuint vertexArrayIDsb;
GLuint vertexBufferIDsb;
GLuint indexBufferIDsb;
GLuint colorBufferIDsb;
GLuint uvBufferIDsb;
GLuint textureIDsb;

// Shader variable IDs
GLuint mvpMatrixIDsb;
GLuint textureSamplerIDsb;
GLuint programIDsb;

void Skybox::initialize() {
    // Set the color values to 1
    for (int i = 0; i < 72; ++i) color_buffer_data[i] = 1.0f;

    // Create a vertex array object
    glGenVertexArrays(1, &vertexArrayIDsb);
    glBindVertexArray(vertexArrayIDsb);

    // Create a vertex buffer object to store the vertex data
    glGenBuffers(1, &vertexBufferIDsb);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIDsb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the color data
    glGenBuffers(1, &colorBufferIDsb);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferIDsb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the UV data
    glGenBuffers(1, &uvBufferIDsb);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferIDsb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &indexBufferIDsb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferIDsb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    // Create and compile GLSL program from the shaders
    programIDsb = LoadShadersFromFile("../FinalProj/skybox/skybox.vert", "../FinalProj/skybox/skybox.frag");
    if (programIDsb == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }


    // Get a handle for our "MVP" uniform
    mvpMatrixIDsb = glGetUniformLocation(programIDsb, "MVP");

    // Load texture
    textureIDsb = LoadTextureSkybox("../FinalProj/skybox/Sky.png");

    // Get a handle to texture sampler
    textureSamplerIDsb = glGetUniformLocation(programIDsb,"textureSampler");
}

void Skybox::render(glm::mat4 cameraMatrix) {
    glUseProgram(programIDsb);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIDsb);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferIDsb);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferIDsb);


    // Set model-view-projection matrix
    glUniformMatrix4fv(mvpMatrixIDsb, 1, GL_FALSE, &cameraMatrix[0][0]);

    // Enable UV buffer and texture sampler
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferIDsb);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIDsb);
    glUniform1i(textureSamplerIDsb, 0);

    // Draw the box
    glDrawElements(
        GL_TRIANGLES,      // mode
        36,    			   // number of indices
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Skybox::cleanup() {
    glDeleteBuffers(1, &vertexBufferIDsb);
    glDeleteBuffers(1, &colorBufferIDsb);
    glDeleteBuffers(1, &indexBufferIDsb);
    glDeleteVertexArrays(1, &vertexArrayIDsb);
    glDeleteBuffers(1, &uvBufferIDsb);
    glDeleteTextures(1, &textureIDsb);
    glDeleteProgram(programIDsb);
}