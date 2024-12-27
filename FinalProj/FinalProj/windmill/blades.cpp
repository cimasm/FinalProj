#include "blades.h"
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

void Blades::generateGeometry() {
    vertex_buffer_data = {
        // Blade 1 (Top Triangle)
        0.1f, 0.0f, 0.0f,   // Right base
        -0.1f, 0.0f, 0.0f,  // Left base
        0.0f, 3.0f, 0.0f,   // Top tip

        // Blade 2 (Right Triangle)
        0.0f, 0.1f, 0.0f,   // Top base
        0.0f, -0.1f, 0.0f,  // Bottom base
        3.0f, 0.0f, 0.0f,   // Right tip

        // Blade 3 (Bottom Triangle)
        0.1f, 0.0f, 0.0f,   // Right base
        -0.1f, 0.0f, 0.0f,  // Left base
        0.0f, -3.0f, 0.0f,  // Bottom tip

        // Blade 4 (Left Triangle)
        0.0f, 0.1f, 0.0f,   // Top base
        0.0f, -0.1f, 0.0f,  // Bottom base
        -3.0f, 0.0f, 0.0f,  // Left tip
    };

    index_buffer_data = {
        0, 1, 2,   // Blade 1
        3, 4, 5,   // Blade 2
        6, 7, 8,   // Blade 3
        9, 10, 11  // Blade 4
    };

    for (int i = 0; i < 12; ++i) {
        color_buffer_data.push_back(0.8f); // R
        color_buffer_data.push_back(0.8f); // G
        color_buffer_data.push_back(0.8f); // B
    }
}

void Blades::initialize(glm::vec3 position, glm::vec3 scale) {
    this->position = position;
    this->scale = scale;

    generateGeometry();

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(GLfloat), vertex_buffer_data.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, color_buffer_data.size() * sizeof(GLfloat), color_buffer_data.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), index_buffer_data.data(), GL_STATIC_DRAW);

    programID = LoadShadersFromFile("../FinalProj/windmill/windmill.vert", "../FinalProj/windmill/windmill.frag");
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
}

void Blades::render(glm::mat4 cameraMatrix) {
    static float rotationAngle = 0.0f; // Persistent angle of rotation
    rotationAngle += 0.2f;            // Increment the angle to make it spin (adjust speed here)

    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Apply transformations: translate to position, rotate, then scale
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // Z-axis rotation
    modelMatrix = glm::scale(modelMatrix, scale);

    // Calculate final MVP matrix
    glm::mat4 mvp = cameraMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Draw the blades
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glDrawElements(GL_TRIANGLES, index_buffer_data.size(), GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}


void Blades::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteProgram(programID);
}