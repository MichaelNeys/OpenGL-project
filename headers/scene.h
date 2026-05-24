#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "skybox.h"
#include "terrain.h"
#include "BezierPath.h"

class Scene {
public:
    Scene();
    void Draw(Shader& lightingShader, Shader& lampShader,
              glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);
    void Delete();
    
    // interactie
    bool redstoneLampsOn = true;
    void checkMouseClick(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight);
    
    // camera getters
    glm::vec3 getBeePosition() const { return currentBeePos; }
    glm::vec3 getBeeDirection() const { return currentBeeDir; }

    bool showBee = true;

private:
    // init helpers
    void initModels();
    void initPollen();
    void initCrosshair();
    void initPickingFBO();

    // hulp functies
    void setLightUniforms(Shader& shader);
    void drawVillage(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection);
    void drawBee(Shader& lightingShader);
    void drawPollen(Shader& lightingShader, glm::mat4& view, glm::mat4& projection);
    void drawCrosshair(Shader& lightingShader);

    // scene objecten
    Skybox* skybox  = nullptr;
    Terrain* terrain = nullptr;
    Model* Village = nullptr;
    Model* Bee     = nullptr;
    Mesh* lampMesh = nullptr;

    // globale lichtposities
    glm::vec3 lightPos;

    // matrices
    glm::mat4 m_villageMatrix;
    std::vector<glm::mat4> pollenMatrices;

    // bei stuff
    BezierPath beePath;
    float currentDistance = 0.0f;
    glm::vec3 currentBeePos;
    glm::vec3 currentBeeDir;

    // UI elementen
    unsigned int crosshairVAO, crosshairVBO;
    unsigned int pickingFBO, pickingColorTexture, pickingDepthRBO;
    Shader* pickingShader = nullptr;
};