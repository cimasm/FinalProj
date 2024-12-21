#include "windmill.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include <render/shader.h>


// Generate the geometry (vertices, colors, and indices) for the cylinder
void Windmill::generateGeometry(int slices, float height, float radius) {
    vertex_buffer_data.clear();
    index_buffer_data.clear();
    color_buffer_data.clear();

    float angleStep = 2 * M_PI / slices;

    // Vertices for top and bottom circles
    for (int i = 0; i < slices; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Bottom circle
        vertex_buffer_data.push_back(x);
        vertex_buffer_data.push_back(0.0f);
        vertex_buffer_data.push_back(z);

        // Top circle
        vertex_buffer_data.push_back(x);
        vertex_buffer_data.push_back(height);
        vertex_buffer_data.push_back(z);

        // Colors (set to white for simplicity)
        for (int j = 0; j < 2; ++j) {
            color_buffer_data.push_back(1.0f);
            color_buffer_data.push_back(1.0f);
            color_buffer_data.push_back(1.0f);
        }
    }

    // Generate indices
    for (int i = 0; i < slices; ++i) {
        int bottom1 = 2 * i;
        int top1 = bottom1 + 1;
        int bottom2 = 2 * ((i + 1) % slices);
        int top2 = bottom2 + 1;

        // Side triangles
        index_buffer_data.push_back(bottom1);
        index_buffer_data.push_back(top1);
        index_buffer_data.push_back(bottom2);

        index_buffer_data.push_back(top1);
        index_buffer_data.push_back(top2);
        index_buffer_data.push_back(bottom2);
    }
}

void Windmill::initialize(glm::vec3 position, glm::vec3 scale) {
    this->position = position;
    this->scale = scale;

    generateGeometry(36, 2.0f, 1.0f);  // 36 slices, height 2, radius 1

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Vertex buffer
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(GLfloat), vertex_buffer_data.data(), GL_STATIC_DRAW);

    // Color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, color_buffer_data.size() * sizeof(GLfloat), color_buffer_data.data(), GL_STATIC_DRAW);

    // Index buffer
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), index_buffer_data.data(), GL_STATIC_DRAW);

    programID = LoadShadersFromFile("../FinalProj/box.vert", "../FinalProj/box.frag");
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
}

void Windmill::render(glm::mat4 cameraMatrix) {
    glUseProgram(programID);

    glBindVertexArray(vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    modelMatrix = glm::scale(modelMatrix, scale);

    glm::mat4 mvp = cameraMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glDrawElements(GL_TRIANGLES, index_buffer_data.size(), GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Windmill::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteProgram(programID);
}
