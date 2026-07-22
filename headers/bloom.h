#pragma once
#include <glad/glad.h>
#include "shader.h"

class Bloom {
public:
    Bloom(int width, int height);
    ~Bloom();

    void bindScene();
    void process();
    void render();

    unsigned int getResultTexture() const;

    bool enabled = true;
    float threshold = 0.8f;
    float strength = 0.5f;
    int blurPasses = 10;

private:
    void setupFBO(unsigned int& fbo, unsigned int& tex, unsigned int& rbo, bool withDepth);
    void setupPingpong(unsigned int& fbo, unsigned int& tex);
    void renderQuad();

    int width = 0;
    int height = 0;

    unsigned int sceneFBO = 0;
    unsigned int sceneTexture = 0;
    unsigned int sceneRBO = 0;

    unsigned int pingpongFBO[2] = {0, 0};
    unsigned int pingpongTexture[2] = {0, 0};

    unsigned int resultFBO = 0;
    unsigned int resultTexture = 0;

    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    int lastBlurredIndex = 0;

    Shader* brightExtractShader = nullptr;
    Shader* blurShader = nullptr;
    Shader* combineShader = nullptr;
};