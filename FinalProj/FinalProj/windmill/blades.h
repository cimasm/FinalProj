#ifndef BLADES_H
#define BLADES_H
#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>

struct Blades {
    GLuint vertexArrayID, vertexBufferID, indexBufferID, colorBufferID, programID, mvpMatrixID;

    glm::vec3 position;
    glm::vec3 scale;

    std::vector<GLfloat> vertex_buffer_data;
    std::vector<GLuint> index_buffer_data;
    std::vector<GLfloat> color_buffer_data;

    void generateGeometry();
    void initialize(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix);
    void cleanup();
};

#endif // BLADES_H
