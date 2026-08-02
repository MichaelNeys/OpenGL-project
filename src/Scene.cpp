#include "Scene.h"
#include "GeometryData.h"
#include "Track/TrackPresets.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>

/**
 * @brief constructor
 * 
 * init submodules, geometrie, crosshair en overige FBO's
 */
Scene::Scene() 
    : lightPos(0.0f, -1.0f, -12.0f),
      m_mainTrack(TrackPresets::MainParcours),
      m_alternativeTrack(TrackPresets::AlternativeTrack)
{
    m_currentTrack = &m_mainTrack;

    // Instantieer sub-modules
    m_lightManager  = new LightManager();
    m_trackRenderer = new TrackRenderer();

    constexpr unsigned int vertexCount = 216;
    lampMesh = new Mesh(Geometry::cubeVertices.data(), vertexCount, true);
    terrain  = new Terrain();

    m_trackRenderer->initialize(lampMesh);
    m_trackRenderer->updateTrackData(m_currentTrack);

    const std::vector<std::string> skyFaces = {
        "../textures/skybox/front.jpg",
        "../textures/skybox/back.jpg",
        "../textures/skybox/up.jpg",
        "../textures/skybox/down.jpg",
        "../textures/skybox/right.jpg",
        "../textures/skybox/left.jpg"
    };
    skybox = new Skybox(skyFaces, "../shaders/skybox.vert", "../shaders/skybox.frag");

    // transformatie van village
    m_villageMatrix = glm::mat4(1.0f);
    m_villageMatrix = glm::translate(m_villageMatrix, glm::vec3(0.0f, -3.0f, -10.0f));
    m_villageMatrix = glm::rotate(m_villageMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_villageMatrix = glm::scale(m_villageMatrix, glm::vec3(5.0f));

    initModels();
    initCrosshair();
    initPickingFBO();
}

/**
 * @brief destructor
 * 
 * clear allocated shaders en OpenGL buffers
 */
Scene::~Scene() {
    delete skybox;          skybox = nullptr;
    delete terrain;         terrain = nullptr;
    delete lampMesh;        lampMesh = nullptr;
    delete Village;         Village = nullptr;
    delete Bee;             Bee = nullptr;
    delete m_trackRenderer; m_trackRenderer = nullptr;
    delete m_lightManager;  m_lightManager = nullptr;
    delete pickingShader;   pickingShader = nullptr;

    // OpenGL buffers
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);

    // picking FBO buffers
    if (pickingFBO != 0) glDeleteFramebuffers(1, &pickingFBO);
    if (pickingColorTexture != 0) glDeleteTextures(1, &pickingColorTexture);
    if (pickingDepthRBO != 0) glDeleteRenderbuffers(1, &pickingDepthRBO);
}

/**
 * @brief load 3D models
 */
void Scene::initModels() {
    for (const std::string& base : { "models", "../models" }) {
        std::string path = base + "/minecraft_ville.glb";
        if (std::filesystem::exists(path)) { Village = new Model(path); break; }
    }
    if (!Village) std::cerr << "ERROR::SCENE: Village model niet gevonden!" << std::endl;

    for (const std::string& base : { "models", "../models" }) {
        std::string path = base + "/minecraft_bee.glb";
        if (std::filesystem::exists(path)) { Bee = new Model(path); break; }
    }
    if (!Bee) std::cerr << "ERROR::SCENE: Bee model niet gevonden!" << std::endl;
}

/**
 * @brief buffers voor crosshair
 */
void Scene::initCrosshair() {
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Geometry::crosshairVertices), Geometry::crosshairVertices.data(), GL_STATIC_DRAW);

    // positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

/**
 * @brief FBO for color based object picking
 */
void Scene::initPickingFBO() {
    pickingShader = new Shader("shaders/picking.vert", "shaders/picking.frag");

    glGenFramebuffers(1, &pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

    // texture buffer voor kleur
    glGenTextures(1, &pickingColorTexture);
    glBindTexture(GL_TEXTURE_2D, pickingColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingColorTexture, 0);

    // renderbuffer voor depth test
    glGenRenderbuffers(1, &pickingDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, pickingDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pickingDepthRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief rendert alles
 * 
 * @param lightingShader lighting shader
 * @param lampShader lamp shader
 * @param view current view matrix
 * @param projection current projection matrix
 * @param cameraPos current camera pos
 */
void Scene::Draw(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos) {
    // 1. skybox
    skybox->Draw(view, projection);

    // 2. belichting
    lightingShader.use();
    m_lightManager->setLightUniforms(lightingShader, lightPos, redstoneLampsOn);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt ("texture_diffuse1", 0);

    // 3. terrein & models
    terrain->Draw(lightingShader);
    drawVillage(lightingShader, lampShader, view, projection);
    drawBee(lightingShader);
    
    // 4. path
    m_trackRenderer->Draw(lightingShader, lampShader, view, projection);

    // 5. crosshair
    drawCrosshair(lampShader);
}

/**
 * @brief rendert village maar onderscheidt tussen lampen en andere meshes voor verschillende shaders
 * 
 * @param lightingShader lighting shader
 * @param lampShader lamp shader
 * @param view current view matrix
 * @param projection current projection matrix
 */
void Scene::drawVillage(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection) {
    if (!Village) return;
    
    glDisable(GL_CULL_FACE);
    bool lampActive = false;
    bool lightActive = false;

    for (unsigned int i = 0; i < Village->meshes.size(); i++) {
        const bool useLamp = (i == 7 || i == 8) || (i == 5 && redstoneLampsOn);
        if (useLamp) {
            if (!lampActive) {
                lampShader.use();
                lampShader.setMat4("model", m_villageMatrix);
                lampShader.setMat4("view", view);
                lampShader.setMat4("projection", projection);
                lampActive = true;
                lightActive = false;
            }
            Village->meshes[i].Draw(lampShader);
        } else {
            if (!lightActive) {
                lightingShader.use();
                lightingShader.setMat4 ("model", m_villageMatrix);
                lightingShader.setVec3 ("material.ambient", glm::vec3(0.15f));
                lightingShader.setVec3 ("material.diffuse", glm::vec3(0.8f));
                lightingShader.setVec3 ("material.specular", glm::vec3(0.2f));
                lightingShader.setFloat("material.shininess", 10.0f);
                lightActive = true;
                lampActive = false;
            }
            Village->meshes[i].Draw(lightingShader);
        }
    }
    glEnable(GL_CULL_FACE);
}

/**
 * @brief update positie en oriëntatie van de bij
 * 
 * @param lightingShader lighting shader
 */
void Scene::drawBee(Shader& lightingShader) {
    if (!Bee) return;

    static float lastTime = static_cast<float>(glfwGetTime());
    const float currentTime = static_cast<float>(glfwGetTime());
    const float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // afstand tov begin van het spoor
    currentDistance += 0.8f * deltaTime;
    if (currentDistance > m_currentTrack->getTotalLength()) {
        currentDistance = fmod(currentDistance, m_currentTrack->getTotalLength());
    }

    // positie en oriëntatie op basis vd t-parameter
    const float t = m_currentTrack->getTForDistance(currentDistance);
    const glm::vec3 beePos = m_currentTrack->getPoint(t);
    
    float tNext = m_currentTrack->getTForDistance(currentDistance + 0.1f);
    if (tNext < t) tNext = 1.0f;
    const glm::vec3 nextPos = m_currentTrack->getPoint(tNext);
    const glm::vec3 dir = glm::normalize(nextPos - beePos);

    currentBeePos = beePos;
    currentBeeDir = dir;

    if (!showBee) return;

    const float yaw   = atan2(dir.x, dir.z) + glm::radians(180.0f);
    const float pitch = atan2(dir.y, sqrt(dir.x * dir.x + dir.z * dir.z));

    glm::mat4 beeModel = glm::mat4(1.0f);
    beeModel = glm::translate(beeModel, beePos);
    beeModel = glm::rotate(beeModel, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    beeModel = glm::rotate(beeModel, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    beeModel = glm::scale(beeModel, glm::vec3(0.01f));

    lightingShader.use();
    lightingShader.setMat4 ("model", beeModel);
    lightingShader.setVec3 ("material.ambient", glm::vec3(0.10f, 0.10f, 0.10f));
    lightingShader.setVec3 ("material.diffuse", glm::vec3(0.35f, 0.35f, 0.38f));
    lightingShader.setVec3 ("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
    lightingShader.setFloat("material.shininess", 40.0f);
    Bee->Draw(lightingShader);
}

/**
 * @brief render crosshair
 */
void Scene::drawCrosshair(Shader& lampShader) {
    glDisable(GL_DEPTH_TEST);
    lampShader.use();
    lampShader.setMat4("model", glm::mat4(1.0f));
    lampShader.setMat4("view", glm::mat4(1.0f));
    lampShader.setMat4("projection", glm::mat4(1.0f));
    lampShader.setBool("hasDiffuseTexture", false);
    lampShader.setVec3("fallbackColor", glm::vec3(1.0f, 1.0f, 1.0f));

    glLineWidth(3.0f);
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief check of het midden van het scherm een redstone lamp is na een mouse click
 * 
 * GPU Color Picking, Picking FBO with unique color
 */
void Scene::checkMouseClick(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight) {
    if (!Village || !pickingShader) return;

    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
    glViewport(0, 0, 1920, 1080);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pickingShader->use();
    pickingShader->setMat4("view", view);
    pickingShader->setMat4("projection", projection);
    pickingShader->setMat4("model", m_villageMatrix);
    // rode kleur voor specifieke mesh
    pickingShader->setVec3("pickingColor", glm::vec3(1.0f, 0.0f, 0.0f));
    
    // interactieve mesh
    Village->meshes[5].Draw(*pickingShader);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    unsigned char pixel[4];
    // check pixel in het midden
    glReadPixels(screenWidth / 2, screenHeight / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);

    // check of de kleur rood is
    if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
        std::cout << "INTERACTIE ADHV PICKING" << std::endl;
        toggleTrack();
        toggleLamp();
    } else {
        std::cout << "GEEN INTERACTIE ADHV PICKING" << std::endl;
    }
}

/**
 * @brief wissel main track en alternative track
 */
void Scene::toggleTrack() {
    m_currentTrack = (m_currentTrack == &m_mainTrack) ? &m_alternativeTrack : &m_mainTrack;
    std::cout << "  track switched to: " << (m_currentTrack == &m_mainTrack ? "Main" : "Alternative") << std::endl;
    currentDistance = 0.0f;
    
    m_trackRenderer->updateTrackData(m_currentTrack);
}

/**
 * @brief toggle redstone lampen aan/uit
 */
void Scene::toggleLamp() {
    redstoneLampsOn = !redstoneLampsOn;
    std::cout << "  lamp switched to: " << (redstoneLampsOn ? "ON" : "OFF") << std::endl;
}

/**
 * @brief print camera coords naar CLI
 */
void Scene::logCameraCoordinates(const glm::vec3& cameraPos) {
    std::cout << "Camera Positie : X: " << cameraPos.x << " | Y: " << cameraPos.y << " | Z: " << cameraPos.z << std::endl;
}