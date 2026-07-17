#include "scene.h"
#include "GeometryData.h"
#include "Track/TrackPresets.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>
#include <cstdlib>

Scene::Scene() 
    : lightPos(0.0f, -1.0f, -12.0f),
      m_mainTrack(TrackPresets::MainParcours),
      m_alternativeTrack(TrackPresets::AlternativeTrack)
{
    m_currentTrack = &m_mainTrack;

    unsigned int vertexCount = 216;
    lampMesh = new Mesh(const_cast<float*>(Geometry::cubeVertices), vertexCount, true);
    terrain  = new Terrain();

    std::vector<std::string> skyFaces = {
        "../textures/indigo-re-skybox/indigo_ft.jpg",
        "../textures/indigo-re-skybox/indigo_bk.jpg",
        "../textures/indigo-re-skybox/indigo_up.jpg",
        "../textures/indigo-re-skybox/indigo_dn.jpg",
        "../textures/indigo-re-skybox/indigo_rt.jpg",
        "../textures/indigo-re-skybox/indigo_lf.jpg"
    };
    skybox = new Skybox(skyFaces, "../shaders/skybox.vert", "../shaders/skybox.frag");

    m_villageMatrix = glm::mat4(1.0f);
    m_villageMatrix = glm::translate(m_villageMatrix, glm::vec3(0.0f, -3.0f, -10.0f));
    m_villageMatrix = glm::rotate(m_villageMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_villageMatrix = glm::scale(m_villageMatrix, glm::vec3(5.0f));

    initModels();
    initPollen();
    initTrackLine();
    initCrosshair();
    initPickingFBO();
    initUniformNameCache();
}

void Scene::initUniformNameCache() {
    for (int i = 0; i < Geometry::numPointLights; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        u_plPos.push_back(base + ".position");
        u_plAmb.push_back(base + ".ambient");
        u_plDif.push_back(base + ".diffuse");
        u_plSpe.push_back(base + ".specular");
        u_plConst.push_back(base + ".constant");
        u_plLin.push_back(base + ".linear");
        u_plQuad.push_back(base + ".quadratic");
    }
}

void Scene::initPollen() {
    srand((unsigned)time(NULL));
    pollenMatrices.clear();

    std::vector<glm::vec3> fdPoints = m_currentTrack->generateVisualPath(100); 
    
    for (size_t i = 0; i < fdPoints.size(); i++) {
        glm::vec3 basePos = fdPoints[i];

        const float radius = 0.025f;
        float randX = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randY = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randZ = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;

        glm::vec3 finalPos = basePos + glm::vec3(randX, randY, randZ);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, finalPos);
        model = glm::rotate(model, finalPos.x * 15.0f, glm::vec3(1.0f, 0.5f, 0.2f));
        model = glm::rotate(model, finalPos.z * 10.0f, glm::vec3(0.2f, 1.0f, 0.5f));
        model = glm::scale(model, glm::vec3(0.005f));

        pollenMatrices.push_back(model);
    }
}

void Scene::initTrackLine() {
    // Haal de datapunten op via Forward Differencing
    std::vector<glm::vec3> linePoints = m_currentTrack->generateVisualPath(100);
    trackLineVertexCount = static_cast<unsigned int>(linePoints.size());

    // Genereer OpenGL buffers als deze nog niet bestaan
    if (trackLineVAO == 0) {
        glGenVertexArrays(1, &trackLineVAO);
        glGenBuffers(1, &trackLineVBO);
    }

    glBindVertexArray(trackLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trackLineVBO);
    
    // Stuur de berekende forward differencing punten naar de GPU
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec3), linePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Scene::initModels() {
    for (const std::string& base : { "models", "../models" }) {
        std::string path = base + "/minecraft_ville.glb";
        if (std::filesystem::exists(path)) {
            Village = new Model(path);
            std::cout << "Village geladen: " << path << std::endl;
            break;
        }
    }
    if (!Village) std::cerr << "Village model niet gevonden!" << std::endl;

    for (const std::string& base : { "models", "../models" }) {
        std::string path = base + "/minecraft_bee.glb";
        if (std::filesystem::exists(path)) {
            Bee = new Model(path);
            std::cout << "Bee geladen: " << path << std::endl;
            break;
        }
    }
    if (!Bee) std::cerr << "Bee model niet gevonden!" << std::endl;
}

void Scene::initCrosshair() {
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Geometry::crosshairVertices), Geometry::crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Scene::initPickingFBO() {
    pickingShader = new Shader("shaders/picking.vert", "shaders/picking.frag");

    glGenFramebuffers(1, &pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

    glGenTextures(1, &pickingColorTexture);
    glBindTexture(GL_TEXTURE_2D, pickingColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingColorTexture, 0);

    glGenRenderbuffers(1, &pickingDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, pickingDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pickingDepthRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::setLightUniforms(Shader& shader) {
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient", glm::vec3(0.08f, 0.08f, 0.15f));
    shader.setVec3("light.diffuse", glm::vec3(0.25f, 0.25f, 0.35f));
    shader.setVec3("light.specular", glm::vec3(0.1f,  0.1f,  0.1f));

    const glm::vec3 ambientON = glm::vec3(0.05f, 0.02f, 0.0f);
    const glm::vec3 diffuseON = glm::vec3(0.5f,  0.3f,  0.1f);
    const glm::vec3 specularON = glm::vec3(1.0f,  0.8f,  0.5f);
    const glm::vec3 colorOFF = glm::vec3(0.0f);

    for (int i = 0; i < Geometry::numPointLights; i++) {
        shader.setVec3(u_plPos[i], Geometry::pointLightPositions[i]);

        bool isStationLamp = (i == 6 || i == 7);
        glm::vec3 amb, dif, spe;

        if (redstoneLampsOn) {
            amb = ambientON; dif = diffuseON; spe = specularON;
        } else if (isStationLamp) {
            amb = ambientON; dif = glm::vec3(2.0f, 1.2f, 0.4f); spe = specularON;
        } else {
            amb = dif = spe = colorOFF;
        }

        shader.setVec3(u_plAmb[i],   amb);
        shader.setVec3(u_plDif[i],   dif);
        shader.setVec3(u_plSpe[i],   spe);
        shader.setFloat(u_plConst[i], 1.0f);
        shader.setFloat(u_plLin[i],   0.09f);
        shader.setFloat(u_plQuad[i],  0.032f);
    }
}

void Scene::Draw(Shader& lightingShader, Shader& lampShader,
                 glm::mat4& view, glm::mat4& projection,
                 glm::vec3& cameraPos) {
    skybox->Draw(view, projection);

    lightingShader.use();
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt ("texture_diffuse1", 0);

    terrain->Draw(lightingShader);
    drawVillage(lightingShader, lampShader, view, projection);

    drawBee(lightingShader);
    drawPollen(lightingShader, view, projection);
    
    // TEMP: Render de groene hulplijn die is gegenereerd via Forward Differencing
    glDisable(GL_CULL_FACE);
    lampShader.use();
    lampShader.setMat4("model", glm::mat4(1.0f));
    lampShader.setMat4("view", view);
    lampShader.setMat4("projection", projection);
    lampShader.setBool("hasDiffuseTexture", false);
    lampShader.setVec3("fallbackColor", glm::vec3(0.0f, 1.0f, 0.0f)); // Groen
    glLineWidth(4.0f);
    glBindVertexArray(trackLineVAO);
    glDrawArrays(GL_LINE_STRIP, 0, trackLineVertexCount);
    glBindVertexArray(0);
    glLineWidth(1.0f);
    glEnable(GL_CULL_FACE);
    //TEMP
    drawCrosshair(lampShader);
}

void Scene::drawVillage(Shader& lightingShader, Shader& lampShader,
                        glm::mat4& view, glm::mat4& projection) {
    if (!Village) return;

    glDisable(GL_CULL_FACE);

    bool lampActive = false;
    bool lightActive = false;

    for (unsigned int i = 0; i < Village->meshes.size(); i++) {
        bool useLamp = (i == 7 || i == 8) || (i == 5 && redstoneLampsOn);

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

void Scene::drawBee(Shader& lightingShader) {
    if (!Bee) return;

    static float lastTime = (float)glfwGetTime();
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    currentDistance += 0.8f * deltaTime;
    if (currentDistance > m_currentTrack->getTotalLength())
        currentDistance = fmod(currentDistance, m_currentTrack->getTotalLength());

    float t = m_currentTrack->getTForDistance(currentDistance);
    glm::vec3 beePos = m_currentTrack->getPoint(t);

    float tNext = m_currentTrack->getTForDistance(currentDistance + 0.1f);
    if (tNext < t) tNext = 1.0f;
    glm::vec3 nextPos = m_currentTrack->getPoint(tNext);
    glm::vec3 dir = glm::normalize(nextPos - beePos);

    currentBeePos = beePos;
    currentBeeDir = dir;

    if (!showBee) return;

    float yaw = atan2(dir.x, dir.z) + glm::radians(180.0f);
    float pitch = atan2(dir.y, sqrt(dir.x * dir.x + dir.z * dir.z));

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

void Scene::drawPollen(Shader& lightingShader, glm::mat4& view, glm::mat4& projection) {
    lightingShader.use();
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setVec3("fallbackColor", glm::vec3(1.0f, 0.8f, 0.2f));

    glDisable(GL_CULL_FACE);

    for (const glm::mat4& modelMatrix : pollenMatrices) {
        lightingShader.setMat4("model", modelMatrix);
        lampMesh->Draw();
    }

    glEnable(GL_CULL_FACE);
}

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

void Scene::Delete() {
    skybox->Delete();
    delete skybox;

    terrain->Delete();
    delete terrain;

    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);
    
    if (trackLineVAO != 0) {
        glDeleteVertexArrays(1, &trackLineVAO);
        glDeleteBuffers(1, &trackLineVBO);
    }

    lampMesh->Delete();
    delete lampMesh;

    if (Village) { Village->Delete(); delete Village; }
    if (Bee) { Bee->Delete(); delete Bee; }
}

void Scene::checkMouseClick(glm::mat4 view, glm::mat4 projection,
                            int screenWidth, int screenHeight) {
    if (!Village) return;

    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pickingShader->use();
    pickingShader->setMat4("view", view);
    pickingShader->setMat4("projection", projection);
    pickingShader->setMat4("model", m_villageMatrix);
    pickingShader->setVec3("pickingColor", glm::vec3(1.0f, 0.0f, 0.0f));
    Village->meshes[5].Draw(*pickingShader);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    unsigned char pixel[4];
    glReadPixels(screenWidth / 2, screenHeight / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
        std::cout << "INTERACTIE ADHV PICKING" << std::endl;
        toggleTrack();
        toggleLamp();
    } else {
        std::cout << "GEEN INTERACTIE ADHV PICKING" << std::endl;
    }
}

void Scene::toggleTrack() {
    if (m_currentTrack == &m_mainTrack) {
        m_currentTrack = &m_alternativeTrack;
        std::cout << "  track switched to: Alternative" << std::endl;
    } else {
        m_currentTrack = &m_mainTrack;
        std::cout << "  track switched to: Main" << std::endl;
    }
    currentDistance = 0.0f;
    
    // Update de pollen én de visuele tracklijn naar de nieuwe route via Forward Differencing!
    initPollen();
    initTrackLine();
}

void Scene::toggleLamp() {
    redstoneLampsOn = !redstoneLampsOn;
    std::cout << "  lamp switched to: " << (redstoneLampsOn ? "ON" : "OFF") << std::endl;
}

void Scene::logCameraCoordinates(const glm::vec3& cameraPos) {
    std::cout << "Camera Positie : X: " << cameraPos.x << " | Y: " << cameraPos.y << " | Z: " << cameraPos.z << std::endl;
}