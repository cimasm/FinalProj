#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#include <stb/stb_image.h>
#include <iostream>
#include "building.h"

static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}

void Building::initialize(glm::vec3 position, glm::vec3 scale, const char *texture_file_path) {

	// Set the color values to 1
	for (int i = 0; i < 72; ++i) color_buffer_data[i] = 1.0f;

	// Define scale of the building geometry
	this->position = position;
	this->scale = scale;

	// Create a vertex array object
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create a vertex buffer object to store the vertex data
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the color data
	glGenBuffers(1, &colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

	// scale up the UV coordinates to a range greater than 1
	for (int i = 0; i < 24; ++i) uv_buffer_data[2*i+1] *= 5;

	// Create a vertex buffer object to store the UV data
	glGenBuffers(1, &uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the vertex normals
	glGenBuffers(1, &normalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);


	// Create an index buffer object to store the index data that defines triangle faces
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	programID = LoadShadersFromFile("../FinalProj/s2.vert", "../FinalProj/s2.frag");
	if (programID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Get a handle for our "MVP" uniform
	mvpMatrixID = glGetUniformLocation(programID, "MVP");

    // Load a texture
	textureID = LoadTextureTileBox(texture_file_path);

    // Get a handle to texture sampler
	textureSamplerID = glGetUniformLocation(programID,"textureSampler");
}

void Building::render_first_pass(glm::mat4 cameraMatrix) {

}

void Building::render_second_pass(glm::mat4 cameraMatrix) {
	glUseProgram(programID);

	// Vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Colour
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Normals
	glEnableVertexAttribArray(3); // Attribute location 3 for normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	// Model transform
    glm::mat4 modelMatrix = glm::mat4();
	modelMatrix = translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);

	GLuint modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

	///////
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));


	// Set model-view-projection matrix
	glm::mat4 mvp = cameraMatrix * modelMatrix;
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	// Set the normal matrix
	GLuint normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
	glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);

	//////

	// // Set model-view-projection matrix
	// glm::mat4 mvp = cameraMatrix * modelMatrix;
	// glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	// Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glActiveTexture(GL_TEXTURE0); // Set textureSampler to use texture unit 0
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(textureSamplerID, 0);

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

void Building::cleanup() {
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &colorBufferID);
	glDeleteBuffers(1, &indexBufferID);
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteBuffers(1, &uvBufferID);
	glDeleteTextures(1, &textureID);
	glDeleteProgram(programID);
}