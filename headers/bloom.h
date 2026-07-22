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
    float threshold = 0.8f; // helderheidsdrempel voor bloom
    float strength  = 0.2f; // hoe sterk de bloom is

private:
    int width, height;
    unsigned int resultFBO, resultTexture;
    unsigned int blurPasses;

    // Hoofd scene FBO
    unsigned int sceneFBO, sceneTexture, sceneRBO;

    // Twee ping-pong FBOs voor blur passes
    unsigned int pingpongFBO[2], pingpongTexture[2];

    // Quad
    unsigned int quadVAO, quadVBO;

    Shader* brightExtractShader = nullptr;
    Shader* blurShader = nullptr;
    Shader* combineShader = nullptr;

    void setupFBO(unsigned int& fbo, unsigned int& tex, unsigned int& rbo, bool withDepth);
    void setupPingpong(unsigned int& fbo, unsigned int& tex);
    void renderQuad();
};