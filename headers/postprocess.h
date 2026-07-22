#pragma once
#include <glad/glad.h>
#include "shader.h"

enum class PostEffect {
    NONE,
    GAUSSIAN_BLUR,
    EDGE_DETECT
};

class PostProcessor {
public:
    PostProcessor(int width, int height);
    ~PostProcessor();
    void Bind(); 
    void Unbind();
    void Draw();
    void DrawFromTexture(unsigned int texture);
    void setEffect(PostEffect effect) { currentEffect = effect; }
    PostEffect getEffect() const { return currentEffect; }

private:
    unsigned int FBO, textureColorbuffer, RBO;
    unsigned int quadVAO, quadVBO;
    Shader* shader = nullptr;
    PostEffect currentEffect = PostEffect::NONE;
    int screenWidth, screenHeight;
};