// src/scene.cpp
#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <algorithm>

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

Scene::Scene() : lightPos(0.0f, 50.0f, 50.0f) {
    unsigned int vertexCount = sizeof(cubeVertices) / sizeof(float);
    lampMesh  = new Mesh(cubeVertices, vertexCount, true);

    std::vector<glm::vec3> bezierPoints;
    int numSegments = 300;
    bezierPointCount = numSegments + 1;

    std::vector<glm::vec3> controlPoints = {
        glm::vec3(15.0f,  0.0f, -10.0f),
        glm::vec3(15.0f,  0.0f, -10.0f),
        glm::vec3(5.5f,  0.0f, -10.0f),
        glm::vec3(9.0f,   0.65f, -15.0f),
        glm::vec3( 1.0f,  0.0f,  -15.0f),
        glm::vec3(0.0f, 0.2f, -20.0f),
        glm::vec3(-0.4f, 0.6f, -20.0f),
        glm::vec3(-1.5f, 1.8f, -13.5f),
        glm::vec3(-4.0f, 1.4f, -9.0f),
        glm::vec3(-6.0f, 0.7f, -9.2f),
        glm::vec3(-7.0f, 0.2f, -9.5f),
        glm::vec3(-9.0f, -1.0f, -10.0f),
        glm::vec3(-12.0f, -1.2f, -9.75f),
        glm::vec3(-21.0f, -0.9f, -4.0f),
        glm::vec3(-18.0f, 0.0f, -4.0f),
        glm::vec3(-11.0f, 0.1f, -6.5f),
        glm::vec3(-13.0f, 0.0f, -15.0f),
        glm::vec3(-5.0f, -1.5f, -38.0f),
        glm::vec3(5.5f, -0.8f, -40.0f),
        glm::vec3(-26.0f, 0.5f, -31.0f),
        glm::vec3(-20.0f, 1.0f, -18.0f),
        glm::vec3(-17.0f, 0.5f, -8.0f),
        glm::vec3(-14.0f, 0.0f, -18.0f),
        glm::vec3(-1.0f, 0.0f, -18.0f),
        glm::vec3(0.0f, 0.0f, -18.0f),
        glm::vec3(1.0f, 0.0f, -18.0f),
        glm::vec3(4.0f, 0.0f, -18.0f),
        glm::vec3(7.0f, 0.5f, -18.0f),
        glm::vec3(10.0f, 1.0f, -18.0f),
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

    // Load exported Mincraft village mesh parts if present.
    std::string VillageBase = "";
    std::vector<std::string> VillageBases = {
        "models/Minecraft ville",
        "../models/Minecraft ville"
    };

    for (const std::string& base : VillageBases) {
        const std::string path = base + "/minecraft_ville.glb";
        if (std::filesystem::exists(path)) {
            VillageBase = path;
            break;
        }
    }

    if (!VillageBase.empty()) {
        Village = new Model(VillageBase);
        std::cout << "Village geladen: " << VillageBase << std::endl;
    } else {
        Elytra = nullptr;
        std::cerr << "Village model niet gevonden!" << std::endl;
    }

    // Load all exported Elytra mesh parts if present.
    std::string ElytraPath = "";
    std::vector<std::string> ElytraBases = {
        "models/Bee-1",
        "../models/Bee-1"
    };

    for (const std::string& base : ElytraBases) {
        const std::string path = base + "/minecraft_bee.glb";
        if (std::filesystem::exists(path)) {
            ElytraPath = path;
            break;
        }
    }

    if (!ElytraPath.empty()) {
        Elytra = new Model(ElytraPath);
        std::cout << "Elytra geladen: " << ElytraPath << std::endl;
    } else {
        Elytra = nullptr;
        std::cerr << "Elytra model niet gevonden!" << std::endl;
    }
}

void Scene::setLightUniforms(Shader& shader) {
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  glm::vec3(0.8f, 0.8f, 0.8f));
    shader.setVec3("light.diffuse",  glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
}

void Scene::Draw(Shader& lightingShader, Shader& lampShader,
                 glm::mat4& view, glm::mat4& projection,
                 glm::vec3& cameraPos) {
    lightingShader.use();
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt("texture_diffuse1", 0);

    // --- Minecraft village ---
    if (Village != nullptr) {
        glm::mat4 VillageModel = glm::mat4(1.0f);
        VillageModel = glm::translate(VillageModel, glm::vec3(0.0f, -3.0f, -10.0f));
        VillageModel = glm::rotate(VillageModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        VillageModel = glm::scale(VillageModel, glm::vec3(5.0f));
        lightingShader.setMat4("model", VillageModel);
        lightingShader.setVec3("material.ambient",  glm::vec3(0.8f, 0.8f, 0.8f));
        lightingShader.setVec3("material.diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
        lightingShader.setVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
        lightingShader.setFloat("material.shininess", 10.0f);
        Village->Draw(lightingShader);
    }

    // --- Elytra model ---
    if (Elytra != nullptr) {
        glm::mat4 ElytraModel = glm::mat4(1.0f);
        ElytraModel = glm::translate(ElytraModel, glm::vec3(1.0f, 1.8f, -3.0f));
        ElytraModel = glm::scale(ElytraModel, glm::vec3(0.01f));
        lightingShader.setMat4("model", ElytraModel);
        lightingShader.setVec3("material.ambient",  glm::vec3(0.10f, 0.10f, 0.10f));
        lightingShader.setVec3("material.diffuse",  glm::vec3(0.35f, 0.35f, 0.38f));
        lightingShader.setVec3("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
        lightingShader.setFloat("material.shininess", 40.0f);
        Elytra->Draw(lightingShader);
    }

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
    lampMesh->Delete();
    delete lampMesh;
    glDeleteVertexArrays(1, &bezierVAO);
    glDeleteBuffers(1, &bezierVBO);
    if (Village != nullptr) {
        Village->Delete();
        delete Village;
    }
    if (Elytra != nullptr) {
        Elytra->Delete();
        delete Elytra;
    }
}