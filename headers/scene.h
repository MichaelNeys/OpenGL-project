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

    void Draw(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);
    void Delete();

    // Interactie
    bool redstoneLampsOn = true;
    void checkMouseClick(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight);

    // Camera getters
    glm::vec3 getBeePosition() const { return currentBeePos; }
    glm::vec3 getBeeDirection() const { return currentBeeDir; }

    bool showBee = true;

private:
    // Init helpers
    void initModels();
    void initPollen();
    void initCrosshair();
    void initPickingFBO();
    void initUniformNameCache();

    // Hulpfuncties
    void setLightUniforms(Shader& shader);
    void drawVillage(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection);
    void drawBee (Shader& lightingShader);
    void drawPollen (Shader& lightingShader, glm::mat4& view, glm::mat4& projection);
    void drawCrosshair(Shader& lampShader);

    // Scene objecten
    Skybox* skybox = nullptr;
    Terrain* terrain = nullptr;
    Model* Village = nullptr;
    Model* Bee = nullptr;
    Mesh* lampMesh = nullptr;

    // Licht
    glm::vec3 lightPos;

    // Matrices
    glm::mat4 m_villageMatrix;
    std::vector<glm::mat4> pollenMatrices;

    // Bee
    BezierPath beePath;
    float currentDistance = 0.0f;
    glm::vec3 currentBeePos;
    glm::vec3 currentBeeDir;

    // UI elementen
    unsigned int crosshairVAO = 0, crosshairVBO = 0;
    unsigned int pickingFBO = 0, pickingColorTexture = 0, pickingDepthRBO = 0;
    Shader* pickingShader = nullptr;

    // Pre-gecachede uniform namen voor point lights
    std::vector<std::string> u_plPos, u_plAmb, u_plDif, u_plSpe;
    std::vector<std::string> u_plConst, u_plLin, u_plQuad;
};