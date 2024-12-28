#ifndef SKYBOX_H
#define SKYBOX_H
#include <glm/detail/type_mat.hpp>
#include "glad/gl.h"

struct Skybox {
    GLuint vertexArraySkyboxID, vertexBufferSkyboxID, indexBufferSkyboxID, colorBufferSkyboxID, uvBufferSkyboxID, textureSkyboxID;
    GLuint mvpMatrixSkyboxID, textureSamplerSkyboxID, programSkyboxID;


    GLfloat vertex_buffer_data[72] = {
        // Front face
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        // Back face
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,

        // Left face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        // Right face
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,

        // Top face
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
    };

    GLfloat color_buffer_data[72] = {
        // Front, red
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // Back, yellow
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        // Left, green
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // Right, cyan
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        // Top, blue
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Bottom, magenta
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
    };

    GLuint index_buffer_data[36] = {
        0, 3, 2,
        0, 2, 1,

        4, 7, 6,
        4, 6, 5,

        8, 11, 10,
        8, 10, 9,

        12, 15, 14,
        12, 14, 13,

        16, 19, 18,
        16, 18, 17,

        20, 23, 22,
        20, 22, 21
    };

    GLfloat uv_buffer_data[48] = {
        // Z+ face
        0.5f,  0.66f,
        0.25f, 0.66f,
        0.25f, 0.34f,
        0.5f,  0.34f,

        // Z- face
        1.0f,  0.66f,
        0.75f, 0.66f,
        0.75f, 0.34f,
        1.0f,  0.34f,

        // X- face
        0.75f, 0.66f,
        0.5f,  0.66f,
        0.5f,  0.34f,
        0.75f, 0.34f,

        // X+ face
        0.25f, 0.66f,
        0.0f,  0.66f,
        0.0f,  0.34f,
        0.25f, 0.34f,

        // Y+ face
        0.5f,  0.34f,
        0.25f, 0.34f,
        0.25f, 0.0f,
        0.5f,  0.0f,

        // Y- face
        0.5f,  1.0f,
        0.25f, 1.0f,
        0.25f, 0.67f,
        0.5f,  0.67f
    };

    void initialize();
    void render(glm::mat4 cameraMatrix);
    void cleanup();
};

#endif //SKYBOX_H