#include "bloom.h"
#include <iostream>

static float quadVerts[] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f, 1.0f, 1.0f, 1.0f
};

Bloom::Bloom(int width, int height) : width(width), height(height) {
    setupFBO(sceneFBO, sceneTexture, sceneRBO, true);

    setupPingpong(pingpongFBO[0], pingpongTexture[0]);
    setupPingpong(pingpongFBO[1], pingpongTexture[1]);

    unsigned int dummy = 0;
    setupFBO(resultFBO, resultTexture, dummy, false);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    brightExtractShader = new Shader("../shaders/quad.vert", "../shaders/bloom_bright.frag");
    blurShader = new Shader("../shaders/quad.vert", "../shaders/bloom_blur.frag");
    combineShader = new Shader("../shaders/quad.vert", "../shaders/bloom_combine.frag");
}

Bloom::~Bloom() {
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneTexture);
    glDeleteRenderbuffers(1, &sceneRBO);

    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongTexture);

    glDeleteFramebuffers(1, &resultFBO);
    glDeleteTextures(1, &resultTexture);

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    delete brightExtractShader;
    delete blurShader;
    delete combineShader;
}

void Bloom::setupFBO(unsigned int& fbo, unsigned int& tex, unsigned int& rbo, bool withDepth) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (withDepth) {
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Bloom FBO niet compleet!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::setupPingpong(unsigned int& fbo, unsigned int& tex) {
    unsigned int dummy = 0;
    setupFBO(fbo, tex, dummy, false);
}

void Bloom::bindScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Bloom::process() {
    // Brightness extraction
    // Haal alleen heldere pixels eruit en schrijf naar pingpong[0]
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    brightExtractShader->use();
    brightExtractShader->setInt("screenTexture", 0);
    brightExtractShader->setFloat("threshold", threshold);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    renderQuad();

    // Gaussian blur via ping-pong
    // meerdere passes: eerst horizontaal, dan verticaal
    this->blurPasses = 15;
    bool horizontal = true;

    blurShader->use();
    for (int i = 0; i < blurPasses; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal ? 1 : 0]);
        glClear(GL_COLOR_BUFFER_BIT);

        blurShader->setInt("image", 0);
        blurShader->setBool("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongTexture[horizontal ? 0 : 1]);
        renderQuad();

        horizontal = !horizontal;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, resultFBO);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    // Combineer originele scene + geblurde bloom
    combineShader->use();
    combineShader->setInt("scene",   0);
    combineShader->setInt("bloomBlur", 1);
    combineShader->setFloat("bloomStrength", strength);
    combineShader->setBool("bloomEnabled", enabled);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE1);

    int finalIndex = (blurPasses % 2 == 0) ? 0 : 1;
    glBindTexture(GL_TEXTURE_2D, pingpongTexture[finalIndex]);

    renderQuad();
    glEnable(GL_DEPTH_TEST);
}

void Bloom::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int Bloom::getResultTexture() const {
    return resultTexture;
}
