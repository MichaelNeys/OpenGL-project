#include "Bloom.h"
#include <iostream>

/**
 * @brief fullscreen quad
 * 
 * 2 driehoeken in NDC met UV
 */
static const float quadVerts[] = {
    -1.0f,  1.0f,       0.0f, 1.0f,
    -1.0f, -1.0f,       0.0f, 0.0f,
     1.0f, -1.0f,       1.0f, 0.0f,

    -1.0f,  1.0f,       0.0f, 1.0f,
     1.0f, -1.0f,       1.0f, 0.0f,
     1.0f,  1.0f,       1.0f, 1.0f
};

/**
 * @brief constructor voor de Bloom
 * 
 * init alle benodigde FBOs; scene, ping-pong en resultaat
 * zet Vertex Buffers voor scherm-quad en laadt shaders in.
 * 
 * @param width breedte van schermbuffer
 * @param height hoogte van schermbuffer
 */
Bloom::Bloom(int width, int height) : width(width), height(height) {
    setupFBO(sceneFBO, sceneTexture, sceneRBO, true);

    setupPingpong(pingpongFBO[0], pingpongTexture[0]);
    setupPingpong(pingpongFBO[1], pingpongTexture[1]);

    unsigned int dummyRBO = 0;
    setupFBO(resultFBO, resultTexture, dummyRBO, false);

    // init Screen Quad Buffers
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // attribuut 0: Positie (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // attribuut 1: Texture Coördinaten (U, V)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // shaders inladen
    brightExtractShader = new Shader("../shaders/quad.vert", "../shaders/bloom_bright.frag");
    blurShader          = new Shader("../shaders/quad.vert", "../shaders/bloom_blur.frag");
    combineShader       = new Shader("../shaders/quad.vert", "../shaders/bloom_combine.frag");
}

/**
 * @brief destructor
 * 
 * clear FBO, RBO, Texturen, VAO, VBO & dynamically allocated shader-pointers
 */
Bloom::~Bloom() {
    if (sceneFBO != 0) glDeleteFramebuffers(1, &sceneFBO);
    if (sceneTexture != 0) glDeleteTextures(1, &sceneTexture);
    if (sceneRBO != 0) glDeleteRenderbuffers(1, &sceneRBO);

    if (pingpongFBO[0] != 0) glDeleteFramebuffers(2, pingpongFBO);
    if (pingpongTexture[0] != 0) glDeleteTextures(2, pingpongTexture);

    if (resultFBO != 0) glDeleteFramebuffers(1, &resultFBO);
    if (resultTexture != 0) glDeleteTextures(1, &resultTexture);

    if (quadVAO != 0) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO != 0) glDeleteBuffers(1, &quadVBO);

    if (brightExtractShader != nullptr) {
        delete brightExtractShader;
        brightExtractShader = nullptr;
    }
    if (blurShader != nullptr) {
        delete blurShader;
        blurShader = nullptr;
    }
    if (combineShader != nullptr) {
        delete combineShader;
        combineShader = nullptr;
    }
}

/**
 * @brief helper-functie voor opzetten van FBO met kleur en optionele diepte
 * 
 * uses GL_RGB16F float-texture voor HDR
 * 
 * @param fbo refenrence to FBO ID.
 * @param tex Referentie to Texture ID.
 * @param rbo refenrence to Renderbuffer ID.
 * @param withDepth geeft aan of er een depth renderbuffer moet worden gekoppeld
 */
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Bloom FBO niet compleet!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief helper-functie voor opzetten van ping-pong FBO
 * 
 * @param fbo refenrence to FBO ID.
 * @param tex refenrence to Texture ID.
 */
void Bloom::setupPingpong(unsigned int& fbo, unsigned int& tex) {
    unsigned int dummy = 0;
    setupFBO(fbo, tex, dummy, false);
}

/**
 * @brief bindt FBO en clear buffers
 * 
 * aanroepen voor renderen van 3D scene
 */
void Bloom::bindScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief verwerkt opgenomen scene voor bloom effect
 * 
 * 1. Bright Extract: filter pixel boven treshhold
 * 2. Gaussian Blur: switchen tussen 2 ping-pong FBOs voor blur
 */
void Bloom::process() {
    // 1. Brightness Extraction
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    brightExtractShader->use();
    brightExtractShader->setInt("screenTexture", 0);
    brightExtractShader->setFloat("threshold", threshold);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    renderQuad();

    // 2. Gaussian blur via ping-pong
    bool horizontal = true;
    bool firstIteration = true;

    blurShader->use();
    for (int i = 0; i < blurPasses; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal ? 1 : 0]);
        glClear(GL_COLOR_BUFFER_BIT);

        blurShader->setInt("image", 0);
        blurShader->setBool("horizontal", horizontal);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstIteration ? pingpongTexture[0] : pingpongTexture[horizontal ? 0 : 1]);
        
        renderQuad();

        lastBlurredIndex = horizontal ? 1 : 0;
        horizontal = !horizontal;
        firstIteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief combine original scene with blurred bright parts
 * 
 * write result to resultFBO
 */
void Bloom::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, resultFBO);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    combineShader->use();
    combineShader->setInt("scene", 0);
    combineShader->setInt("bloomBlur", 1);
    combineShader->setFloat("bloomStrength", strength);
    combineShader->setBool("bloomEnabled", enabled);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongTexture[lastBlurredIndex]);

    renderQuad();
    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief rendert fullscreen 2D-quad om shaders op toe te passen.
 * 
 * turn off culling zodat quad altijd zichtbaar is
 */
void Bloom::renderQuad() {
    glDisable(GL_CULL_FACE);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

/**
 * @brief return OpenGL Texture ID combine result
 * 
 * @return unsigned int ID van finalized result texture
 */
unsigned int Bloom::getResultTexture() const {
    return resultTexture;
}
