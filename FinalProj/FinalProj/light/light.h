#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include "glad/gl.h"

struct Light {
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    float intensity;

    void configureShader(GLuint programID);
};