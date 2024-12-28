#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <render/shader.h>
#include "glad/gl.h"
#include "model.h"
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"


Model::Model() {
    modelMatrix = glm::mat4(1.0f);
    programID = LoadShadersFromFile("../FinalProj/model/model.vert", "../FinalProj/model/model.frag");
    if (programID == 0) {
        std::cerr << "Failed to load model shaders." << std::endl;
    }
    checkShaderCompilation();
}

Model::~Model() {
    // Clean up buffers when model is destroyed
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

bool Model::loadModel(const std::string& path) {
    std::string err;
    std::string warn;

    // Load glTF file
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cout << "Error: " << err << std::endl;
    }
    if (!ret) {
        std::cout << "Failed to load glTF file: " << path << std::endl;
        return false;
    }

    // Process scene
    const tinygltf::Scene& scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node& node = model.nodes[scene.nodes[i]];
        if (node.mesh >= 0) {
            processMesh(model, model.meshes[node.mesh]);
        }
    }
    setupMesh();
    return true;
}

bool Model::loadTexture(const tinygltf::Model& model, int textureIndex) {
    const tinygltf::Texture& texture = model.textures[textureIndex];
    const tinygltf::Image& image = model.images[texture.source];

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = GL_RGBA;
    if (image.component == 3) format = GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, image.pixel_type, image.image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return true;
}


void Model::processMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
    // Process each primitive in mesh
    for (const auto& primitive : mesh.primitives) {
        if (primitive.material >= 0) {
            const tinygltf::Material& material = model.materials[primitive.material];

            // Check for base color texture
            if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
                if (!loadTexture(model, textureIndex)) {
                    std::cout << "Failed to load texture" << std::endl;
                }
            }
        }

        // Get accessor for vertex positions
        const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
        const float* positions = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posView.byteOffset]));

        // Get accessor for normals
        const float* normals = nullptr;
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
            const tinygltf::BufferView& normalView = model.bufferViews[normalAccessor.bufferView];
            normals = reinterpret_cast<const float*>(&(model.buffers[normalView.buffer].data[normalView.byteOffset]));
        }

        // Get accessor for texture coords
        const float* texCoords = nullptr;
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const tinygltf::BufferView& texView = model.bufferViews[texAccessor.bufferView];
            texCoords = reinterpret_cast<const float*>(&(model.buffers[texView.buffer].data[texView.byteOffset]));
        }

        // For each vertex
        for (size_t i = 0; i < posAccessor.count; i++) {
            Vertex vertex;

            // Positions
            vertex.position.x = positions[i * 3 + 0];
            vertex.position.y = positions[i * 3 + 1];
            vertex.position.z = positions[i * 3 + 2];

            // Normals
            if (normals) {
                vertex.normal.x = normals[i * 3 + 0];
                vertex.normal.y = normals[i * 3 + 1];
                vertex.normal.z = normals[i * 3 + 2];
            }

            // Texture
            if (texCoords) {
                vertex.texCoords.x = texCoords[i * 2 + 0];
                vertex.texCoords.y = texCoords[i * 2 + 1];
            }
            vertices.push_back(vertex);
        }

        // Process indices
        if (primitive.indices >= 0) {
            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
            const void* indexData = &(model.buffers[indexView.buffer].data[indexView.byteOffset]);

            // Handle formats
            switch (indexAccessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* indices = static_cast<const uint16_t*>(indexData);
                    for (size_t i = 0; i < indexAccessor.count; i++) {
                        this->indices.push_back(indices[i]);
                    }
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    const uint32_t* indices = static_cast<const uint32_t*>(indexData);
                    for (size_t i = 0; i < indexAccessor.count; i++) {
                        this->indices.push_back(indices[i]);
                    }
                    break;
                }
            }
        }
    }

    for (const auto& primitive : mesh.primitives) {
        const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
    }
}


void Model::setupMesh() {
    // Create buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Fill element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Texture
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    // Unbind VAO
    glBindVertexArray(0);
}


void Model::render(glm::mat4 viewProjectionMatrix, glm::vec3 eye_center) {
    glUseProgram(this->programID);

    // Set matrices
    GLuint modelID = glGetUniformLocation(this->programID, "model");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLuint vpID = glGetUniformLocation(this->programID, "vp");
    glUniformMatrix4fv(vpID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    // Set lighting parameters
    glm::vec3 lightPos(200.0f, 600.0f, 200.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(0.0f, 1.0f, 0.0f);    // If there's no texture, set to green

    // Set uniforms
    glUniform3fv(glGetUniformLocation(this->programID, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(this->programID, "viewPos"), 1, glm::value_ptr(eye_center));
    glUniform3fv(glGetUniformLocation(this->programID, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(this->programID, "objectColor"), 1, glm::value_ptr(objectColor));

    // Texture
    if (textureID > 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(this->programID, "modelTexture"), 0);
        glUniform1i(glGetUniformLocation(this->programID, "hasTexture"), 1);
    } else {
        glUniform1i(glGetUniformLocation(this->programID, "hasTexture"), 0);
    }

    // Draw the model
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    glUseProgram(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Model::setPosition(const glm::vec3& position) {
    modelMatrix = glm::translate(glm::mat4(1.0f), position);
}

void Model::setRotation(float angle, const glm::vec3& axis) {
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void Model::setScale(const glm::vec3& scale) {
    modelMatrix = glm::scale(modelMatrix, scale);
}

void Model::translate(glm::vec3 movement) {
    modelMatrix = glm::translate(modelMatrix, movement);
}


bool Model::checkShaderCompilation() {
    GLint success;
    char infoLog[512];
    // GLuint vertexShader, fragmentShader;

    // Get shaders
    GLsizei count;
    GLuint shaders[2];
    glGetAttachedShaders(programID, 2, &count, shaders);

    // Check vertex shader
    glGetShaderiv(shaders[0], GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shaders[0], 512, NULL, infoLog);
        std::cout << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }

    // Check fragment shader
    glGetShaderiv(shaders[1], GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shaders[1], 512, NULL, infoLog);
        std::cout << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }

    // Check program linking
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "Shader program linking failed:\n" << infoLog << std::endl;
        return false;
    }

    return true;
}