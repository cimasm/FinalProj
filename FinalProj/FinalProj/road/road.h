#ifndef ROAD_H
#define ROAD_H
#include <glm/detail/type_mat.hpp>
#include "glad/gl.h"

struct Road {
    GLuint vertexArrayID, vertexBufferID, indexBufferID, colorBufferID, uvBufferID, textureID;
    GLuint mvpMatrixID, textureSamplerID, programID;

    glm::vec3 position_rd;
    glm::vec3 scale_rd;

    GLfloat vertex_buffer_data[12] = {	// Vertex definition for a canonical box
    	-50.0f,  0.0f, -50.0f,
		 50.0f,  0.0f, -50.0f,
		 50.0f,  0.0f,  50.0f,
		-50.0f,  0.0f,  50.0f
	};

	GLfloat color_buffer_data[12] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	GLuint index_buffer_data[6] = {	// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3
	};

	// Define UV buffer data
	GLfloat uv_buffer_data[12] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

    void initialize(glm::vec3 position, glm::vec3 scale, const char *texture_file_path);
    void render(glm::mat4 cameraMatrix);
    void cleanup();
};

#endif //ROAD_H