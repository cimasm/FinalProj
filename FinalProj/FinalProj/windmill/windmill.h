#ifndef WINDMILL_H
#define WINDMILL_H
#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>

struct Windmill {
    GLuint vertexArrayID, vertexBufferID, indexBufferID, colorBufferID, programID, mvpMatrixID;

    glm::vec3 position;
    glm::vec3 scale;

    std::vector<GLfloat> vertex_buffer_data;
    std::vector<GLuint> index_buffer_data;
    std::vector<GLfloat> color_buffer_data;

    void generateGeometry(int slices, float height, float radius);
    void initialize(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix);
    void cleanup();
};

#endif // WINDMILL_H
