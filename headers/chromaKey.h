#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class ChromaKey {
public:
    explicit ChromaKey(const char* overlayImagePath);
    ~ChromaKey();

    void DrawPlane();

    bool showOverlay = false;
    bool useChromaKey = true;

private:
    void setupPlane();
    unsigned int loadTexture(const char* path);

    unsigned int planeVAO = 0;
    unsigned int planeVBO = 0;
    unsigned int overlayTexture = 0;
    bool textureLoaded = false;

    Shader* planeShader = nullptr;
    int imageWidth = 0;
    int imageHeight = 0;
};