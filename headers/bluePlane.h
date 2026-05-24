#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class BluePlane {
public:
    BluePlane(const char* overlayImagePath,
              glm::vec3 position = glm::vec3(0.0f, 0.0f, -15.0f),
              glm::vec2 size = glm::vec2(8.0f, 6.0f));

    // Teken vlak in 3D wereld
    void DrawPlane(glm::mat4& view, glm::mat4& projection);

    void Delete();

    bool enabled = true;
    bool showOverlay = false;

private:
    unsigned int planeVAO, planeVBO;
    unsigned int overlayTexture = 0;
    bool textureLoaded = false;
    Shader* planeShader = nullptr;

    glm::vec3 position;
    glm::vec2 size;

    void setupPlane();
    unsigned int loadTexture(const char* path);

    int imageWidth = 0;
    int imageHeight = 0;
};