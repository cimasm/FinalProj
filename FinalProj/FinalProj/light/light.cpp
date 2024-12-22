#include "light.h"
#include <iostream>

void Light::configureShader(GLuint programID){
    GLint lightPosLocation = glGetUniformLocation(programID, "lightPosition");
    GLint lightColorLocation = glGetUniformLocation(programID, "lightColor");
    GLint lightIntensityLocation = glGetUniformLocation(programID, "lightIntensity");

    if (lightPosLocation == -1 || lightColorLocation == -1 || lightIntensityLocation == -1) {
        std::cerr << "Warning: One or more lighting uniforms not found in shader program." << std::endl;
        return;
    }

    glUniform3fv(lightPosLocation, 1, &lightPosition[0]);
    glUniform3fv(lightColorLocation, 1, &lightColor[0]);
    glUniform1f(lightIntensityLocation, intensity);
}
