#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "glad/gl.h"
#include "tiny_gltf.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Model {
public:
    GLuint programID;

private:
    GLuint textureID;
    GLuint VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    glm::mat4 modelMatrix;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    bool loadTexture(const tinygltf::Model& model, int textureIndex);

public:
    Model();
    ~Model();

    bool loadModel(const std::string& path);
    void processMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh);
    void setupMesh();
    void render(glm::mat4 viewProjectionMatrix, glm::vec3 eye_center);
    void setPosition(const glm::vec3& position);
    void setRotation(float angle, const glm::vec3& axis);
    void setScale(const glm::vec3& scale);
    void translate(glm::vec3 movement);
    bool checkShaderCompilation();
};

#endif