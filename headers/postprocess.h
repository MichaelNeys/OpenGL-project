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
    void Bind();           // begin scene renderen naar FBO
    void Unbind();         // stop renderen naar FBO
    void Draw();           // render quad met effect
    void DrawFromTexture(unsigned int texture);
    void Delete();
    void setEffect(PostEffect effect) { currentEffect = effect; }
    PostEffect getEffect() const { return currentEffect; }

private:
    unsigned int FBO, textureColorbuffer, RBO;
    unsigned int quadVAO, quadVBO;
    Shader* shader = nullptr;
    PostEffect currentEffect = PostEffect::NONE;
    int screenWidth, screenHeight;
};