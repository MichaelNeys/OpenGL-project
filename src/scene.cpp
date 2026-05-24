#include "scene.h"
#include "GeometryData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>
#include <cstdlib>

Scene::Scene() : lightPos(0.0f, -1.0f, -12.0f) {
    unsigned int vertexCount = 216;
    lampMesh = new Mesh(const_cast<float*>(Geometry::cubeVertices), vertexCount, true);
    terrain = new Terrain();

    std::vector<std::string> skyFaces = {
        "../models/indigo-re-skybox/indigo_ft.jpg",
        "../models/indigo-re-skybox/indigo_bk.jpg",
        "../models/indigo-re-skybox/indigo_up.jpg",
        "../models/indigo-re-skybox/indigo_dn.jpg",
        "../models/indigo-re-skybox/indigo_rt.jpg",
        "../models/indigo-re-skybox/indigo_lf.jpg"
    };
    skybox = new Skybox(skyFaces, "../shaders/skybox.vert", "../shaders/skybox.frag");

    // vaste matrixen
    m_villageMatrix = glm::mat4(1.0f);
    m_villageMatrix = glm::translate(m_villageMatrix, glm::vec3(0.0f, -3.0f, -10.0f));
    m_villageMatrix = glm::rotate(m_villageMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_villageMatrix = glm::scale(m_villageMatrix, glm::vec3(5.0f));

    // helper initialisators
    initModels();
    initPollen();
    initCrosshair();
    initPickingFBO();
}

void Scene::initPollen() {
    srand((unsigned)time(NULL));

    // aantal pollen
    int amountOfPollen = 6000; 

    for (int i = 0; i < amountOfPollen; i++) {
        float targetDist = (beePath.getTotalLength() / amountOfPollen) * i;
        float t = beePath.getTForDistance(targetDist);
        glm::vec3 basePos = beePath.getPoint(t);

        // verpreiden in een straal rond de curve
        float radius = 0.025f;
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

void Scene::initModels() {
    // Village
    for (const std::string& base : {"models/Minecraft ville", "../models/Minecraft ville"}) {
        std::string path = base + "/minecraft_ville.glb";
        if (std::filesystem::exists(path)) {
            Village = new Model(path);
            std::cout << "Village geladen: " << path << std::endl;
            break;
        }
    }
    if (!Village) std::cerr << "Village model niet gevonden!" << std::endl;

    // Bee
    for (const std::string& base : {"models/Bee-1", "../models/Bee-1"}) {
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

    // Vertel de GPU hoe de array is opgebouwd (komt exact overeen met je kubus)
    // 0 = Positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 1 = Normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 2 = TexCoords
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
    // 1. GLOBAAL LICHT (Maan)
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  glm::vec3(0.08f, 0.08f, 0.15f));
    shader.setVec3("light.diffuse",  glm::vec3(0.25f, 0.25f, 0.35f));
    shader.setVec3("light.specular", glm::vec3(0.1f,  0.1f,  0.1f));

    // 2. ALLE LANTAARNS
    // We definiëren de standaard "AAN" kleuren...
    glm::vec3 ambientON  = glm::vec3(0.05f, 0.02f, 0.0f);
    glm::vec3 diffuseON  = glm::vec3(0.5f,  0.3f,  0.1f);
    glm::vec3 specularON = glm::vec3(1.0f,  0.8f,  0.5f);
    
    // ... en de "UIT" kleur (volledig zwart)
    glm::vec3 colorOFF   = glm::vec3(0.0f);

    for (int i = 0; i < Geometry::numPointLights; i++) {
        std::string n = std::to_string(i);
        shader.setVec3("pointLights[" + n + "].position", Geometry::pointLightPositions[i]);
        
        glm::vec3 currentAmbient, currentDiffuse, currentSpecular;

        // Controleer of dit een stationslamp is (index 6 of 7)
        bool isStationLamp = (i == 6 || i == 7);

        if (redstoneLampsOn) {
            currentAmbient  = ambientON;
            currentDiffuse  = diffuseON;
            currentSpecular = specularON;
        } else if (isStationLamp) { 
            currentAmbient  = ambientON;
            currentDiffuse  = glm::vec3(2.0f,  1.2f,  0.4f);
            currentSpecular = specularON;
        } else { 
            currentAmbient  = colorOFF;
            currentDiffuse  = colorOFF;
            currentSpecular = colorOFF;
        }
        
        // Stuur de berekende kleur van DEZE specifieke lantaarn naar de shader
        shader.setVec3 ("pointLights[" + n + "].ambient",   currentAmbient);
        shader.setVec3 ("pointLights[" + n + "].diffuse",   currentDiffuse);
        shader.setVec3 ("pointLights[" + n + "].specular",  currentSpecular);
        
        shader.setFloat("pointLights[" + n + "].constant",  1.0f);
        shader.setFloat("pointLights[" + n + "].linear",    0.09f);
        shader.setFloat("pointLights[" + n + "].quadratic", 0.032f);
    }
}

void Scene::Draw(Shader& lightingShader, Shader& lampShader,
                 glm::mat4& view, glm::mat4& projection,
                 glm::vec3& cameraPos) {
    // Skybox
    skybox->Draw(view, projection);

    // Lighting setup
    lightingShader.use();
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt("texture_diffuse1", 0);

    // Terrein
    terrain->Draw(lightingShader);
    drawVillage(lightingShader, lampShader, view, projection);

    drawBee(lightingShader);
    drawPollen(lightingShader, view, projection);

    drawCrosshair(lightingShader);
}

void Scene::drawVillage(Shader &lightingShader, Shader &lampShader, glm::mat4 &view, glm::mat4 &projection)
{
    glDisable(GL_CULL_FACE);
    if (Village) {
        for (unsigned int i = 0; i < Village->meshes.size(); i++) {
            
            // Bepaal of we de glow shader moeten gebruiken
            bool useLampShader = false;
            if (i == 7 || i == 8) {
                useLampShader = true; // Gewone lantaarns zijn altijd AAN
            }
            if (i == 5 && redstoneLampsOn) {
                useLampShader = true; // Redstone lampen alleen als de schakelaar AAN is!
            }

            if (useLampShader) {
                lampShader.use();
                lampShader.setMat4("model", m_villageMatrix);
                lampShader.setMat4("view", view);
                lampShader.setMat4("projection", projection);
                Village->meshes[i].Draw(lampShader);
            } else {
                lightingShader.use();
                lightingShader.setMat4("model", m_villageMatrix);
                
                // Als de redstone lamp UIT is, wordt hij net als de muren getekend (donker)
                lightingShader.setVec3("material.ambient",  glm::vec3(0.15f, 0.15f, 0.15f));
                lightingShader.setVec3("material.diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
                lightingShader.setVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
                lightingShader.setFloat("material.shininess", 10.0f);
                Village->meshes[i].Draw(lightingShader);
            }
        }
    }
}

void Scene::drawBee(Shader &lightingShader)
{
    if (Bee) {
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        float deltaTime   = currentTime - lastTime;
        lastTime = currentTime;

        currentDistance += 0.8f * deltaTime;
        
        if (currentDistance > beePath.getTotalLength())
            currentDistance = fmod(currentDistance, beePath.getTotalLength());

        float t           = beePath.getTForDistance(currentDistance);
        glm::vec3 beePos  = beePath.getPoint(t);
        
        float tNext       = beePath.getTForDistance(currentDistance + 0.1f);
        if (tNext < t) tNext = 1.0f;
        glm::vec3 nextPos = beePath.getPoint(tNext);
        
        glm::vec3 dir     = glm::normalize(nextPos - beePos);
        currentBeePos = beePos;
        currentBeeDir = dir;

        if (!showBee)
            return;

        float yaw   = atan2(dir.x, dir.z) + glm::radians(180.0f);
        float pitch = atan2(dir.y, sqrt(dir.x*dir.x + dir.z*dir.z));

        glm::mat4 BeeModel = glm::mat4(1.0f);
        BeeModel = glm::translate(BeeModel, beePos);
        BeeModel = glm::rotate(BeeModel, yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
        BeeModel = glm::rotate(BeeModel, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        BeeModel = glm::scale(BeeModel, glm::vec3(0.01f));

        lightingShader.use();
        lightingShader.setMat4("model", BeeModel);
        lightingShader.setVec3("material.ambient",  glm::vec3(0.10f, 0.10f, 0.10f));
        lightingShader.setVec3("material.diffuse",  glm::vec3(0.35f, 0.35f, 0.38f));
        lightingShader.setVec3("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
        lightingShader.setFloat("material.shininess", 40.0f);
        Bee->Draw(lightingShader);
    }
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

void Scene::drawCrosshair(Shader &lightingShader)
{
    glDisable(GL_DEPTH_TEST);
    
    lightingShader.use(); 
    
    // 2. MAGIE: Door de matrices op "Identity" (1.0f) te zetten, wordt het 
    // perspectief genegeerd en tekenen we direct plat op het 2D-scherm.
    lightingShader.setMat4("model", glm::mat4(1.0f));
    lightingShader.setMat4("view", glm::mat4(1.0f));
    lightingShader.setMat4("projection", glm::mat4(1.0f));
    
    // 3. Maak het kruisje mooi spierwit
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setVec3("fallbackColor", glm::vec3(1.0f, 1.0f, 1.0f)); 

    // tekenen
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
}

void Scene::Delete() {
    skybox->Delete();
    delete skybox;

    terrain->Delete();
    delete terrain;

    // Verwijder de crosshair VAO en VBO
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);

    lampMesh->Delete();
    delete lampMesh;

    if (Village) { Village->Delete(); delete Village; }
    if (Bee)     { Bee->Delete();     delete Bee;     }
}

void Scene::checkMouseClick(glm::mat4 view, glm::mat4 projection, int screenWidth, int screenHeight) {
    if (!Village) return;
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
    
    // Maak het "scherm" helemaal zwart (ID = 0)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Teken ALLEEN de objecten waar we op kunnen klikken
    pickingShader->use();
    pickingShader->setMat4("view", view);
    pickingShader->setMat4("projection", projection);
    pickingShader->setMat4("model", m_villageMatrix);
    
    // redstone lamp de kleur rood geven
    pickingShader->setVec3("pickingColor", glm::vec3(1.0f, 0.0f, 0.0f));
    
    // mesh 5 = redstone lampen
    Village->meshes[5].Draw(*pickingShader);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    unsigned char pixel[3];
    glReadPixels(screenWidth / 2, screenHeight / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 5. Kijk wat we hebben geraakt!
    // pixel[0] is Rood, pixel[1] is Groen, pixel[2] is Blauw
    if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
        // We hebben ROOD geraakt!
        redstoneLampsOn = !redstoneLampsOn;
        std::cout << "Redstone Lamp ge-PICKED! Status: " << (redstoneLampsOn ? "AAN" : "UIT") << std::endl;
    } else {
        std::cout << "Mis! We raakten kleur: " << (int)pixel[0] << "," << (int)pixel[1] << "," << (int)pixel[2] << std::endl;
    }
}
