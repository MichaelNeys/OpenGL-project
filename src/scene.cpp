// src/scene.cpp
#include "scene.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

static glm::vec3 calculateBezierPoint(float t, const std::vector<glm::vec3>& controlPoints) {
    if (controlPoints.empty()) return glm::vec3(0.0f);
    if (controlPoints.size() == 1) return controlPoints[0];

    std::vector<glm::vec3> temp = controlPoints;
    int n = temp.size() - 1;

    // Casteljau's iteratie
    for (int k = 1; k <= n; k++) {
        for (int i = 0; i <= n - k; i++) {
            temp[i] = (1.0f - t) * temp[i] + t * temp[i + 1];
        }
    }

    return temp[0];
}

static float cubeVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

Scene::Scene() : lightPos(1.2f, 1.0f, 2.0f) {
    unsigned int vertexCount = sizeof(cubeVertices) / sizeof(float);
    cubeMesh = new Mesh(cubeVertices, vertexCount, true);
    lampMesh  = new Mesh(cubeVertices, vertexCount, true);
    loadTexture();

    std::vector<glm::vec3> bezierPoints;
    int numSegments = 100;
    bezierPointCount = numSegments + 1;

    std::vector<glm::vec3> controlPoints = {
        glm::vec3(-30.0f,  0.0f, -3.0f), // Start (P0)
        glm::vec3(-1.0f,   4.0f, -1.0f), // P1
        glm::vec3( 1.0f,  -4.0f,  1.0f), // P2
        glm::vec3( 3.0f,   0.0f,  3.0f), // P3
        // glm::vec3( 30.0f, 10.0f, 30.0f)  // P4 (ez toe te voege nu)
    };

    // create vertices voor curve
    for (int i = 0; i <= numSegments; i++) {
        float t = (float)i / (float)numSegments;
        bezierPoints.push_back(calculateBezierPoint(t, controlPoints));
    }

    // create OpenGL buffers voor curve
    glGenVertexArrays(1, &bezierVAO);
    glGenBuffers(1, &bezierVBO);

    // aan GPU geve
    glBindVertexArray(bezierVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bezierVBO);
    glBufferData(GL_ARRAY_BUFFER, bezierPoints.size() * sizeof(glm::vec3), bezierPoints.data(), GL_STATIC_DRAW);

    // enkel x,y,z nodig voor curve
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Scene::loadTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR: Texture niet gevonden" << std::endl;
    }
    stbi_image_free(data);
}

void Scene::setMaterialUniforms(Shader& shader) {
    shader.setVec3("material.ambient",   glm::vec3(0.24725f,  0.1995f,   0.0745f));
    shader.setVec3("material.diffuse",   glm::vec3(0.75164f,  0.60648f,  0.22648f));
    shader.setVec3("material.specular",  glm::vec3(0.628281f, 0.555802f, 0.366065f));
    shader.setFloat("material.shininess", 51.2f);
}

void Scene::setLightUniforms(Shader& shader) {
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  glm::vec3(0.3f,  0.24f, 0.14f));
    shader.setVec3("light.diffuse",  glm::vec3(0.7f,  0.6f,  0.3f));
    shader.setVec3("light.specular", glm::vec3(1.0f,  0.9f,  0.7f));
}

void Scene::Draw(Shader& lightingShader, Shader& lampShader,
                 glm::mat4& view, glm::mat4& projection,
                 glm::vec3& cameraPos) {

    // --- kubus ---
    lightingShader.use();
    setMaterialUniforms(lightingShader);
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("model", glm::mat4(1.0f));
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    cubeMesh->Draw();

    // --- lamp ---
    lampShader.use();
    glm::mat4 lampModel = glm::scale(glm::translate(glm::mat4(1.0f), lightPos), glm::vec3(0.2f));
    lampShader.setMat4("model",      lampModel);
    lampShader.setMat4("view",       view);
    lampShader.setMat4("projection", projection);
    lampMesh->Draw();

    // --- bezier curve ---
    lampShader.use(); 
    lampShader.setMat4("model", glm::mat4(1.0f));
    lampShader.setMat4("view", view);
    lampShader.setMat4("projection", projection);

    glBindVertexArray(bezierVAO);
    glDrawArrays(GL_LINE_STRIP, 0, bezierPointCount); 
    glBindVertexArray(0);
}

void Scene::Delete() {
    cubeMesh->Delete();
    lampMesh->Delete();
    delete cubeMesh;
    delete lampMesh;
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &bezierVAO);
    glDeleteBuffers(1, &bezierVBO);
}