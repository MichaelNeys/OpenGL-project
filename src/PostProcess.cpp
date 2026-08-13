#include "PostProcess.h"
#include <GeometryData.h>
#include <iostream>

/**
 * @brief constructor
 * 
 * init FBO, colortexture, RBO, quad VAO/VBO, load post-processing shader
 * 
 * @param width breedte
 * @param height hoogte
 */
PostProcessor::PostProcessor(int width, int height)
    : screenWidth(width), screenHeight(height) {

    // 1. FBO aanmaken
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // 2. Kleurenbuffer texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // 3. Renderbuffer voor depth en stencil
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Controleer of de framebuffer compleet is
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO niet compleet!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. Quad VAO/VBO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, Geometry::quadVertices.size() * sizeof(float), Geometry::quadVertices.data(), GL_STATIC_DRAW);

    // positie
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // textuurcoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 5. Shader inladen
    shader = new Shader("shaders/quad.vert", "shaders/postprocess.frag");
}

/**
 * @brief deconstructor
 * 
 * clear OpenGL buffers, textures and shader
 */
PostProcessor::~PostProcessor() {
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    delete shader;
}

/**
 * @brief bind FBO zodat rendering naar de FBO gaat
 */
void PostProcessor::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief unbind FBO and restore default framebuffer
 */
void PostProcessor::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
}

/**
 * @brief draws OpenGL texture with the active post-processing effect
 * 
 * @param texture OpenGL texture ID
 */
void PostProcessor::Draw(unsigned int texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();
    shader->setInt("screenTexture", 0);
    shader->setInt("effect", static_cast<int>(currentEffect));

    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}