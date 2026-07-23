#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Skybox.h"
#include "Terrain.h"
#include "Track/BezierPath.h"
#include "Track/TrackRenderer.h"
#include "Lighting/LightManager.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Draw(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);

    // Interactie & Toggles
    void checkMouseClick(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight);
    void toggleTrack();
    void toggleLamp();
    void logCameraCoordinates(const glm::vec3& cameraPos);

    // Camera getters
    glm::vec3 getBeePosition() const { return currentBeePos; }
    glm::vec3 getBeeDirection() const { return currentBeeDir; }

    bool redstoneLampsOn = true;
    bool showBee = true;

private:
    // Init helpers
    void initModels();
    void initCrosshair();
    void initPickingFBO();

    // Hulpfuncties
    void drawVillage(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection);
    void drawBee(Shader& lightingShader);
    void drawCrosshair(Shader& lampShader);

    // Gedelegeerde sub-modules (Houdt deze file structuur proper)
    TrackRenderer* m_trackRenderer = nullptr;
    LightManager*  m_lightManager = nullptr;
    
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

    // Wiskundige banen
    BezierPath  m_mainTrack;
    BezierPath  m_alternativeTrack;
    BezierPath* m_currentTrack = nullptr;

    float currentDistance = 0.0f;
    glm::vec3 currentBeePos;
    glm::vec3 currentBeeDir;

    // UI & Picking Buffers
    unsigned int crosshairVAO = 0, crosshairVBO = 0;
    unsigned int pickingFBO = 0, pickingColorTexture = 0, pickingDepthRBO = 0;
    Shader* pickingShader = nullptr;
};