// src/scene.cpp
#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include <GLFW/glfw3.h>

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

void Scene::buildLUT(int lutResolution) {
    arcLengthLUT.clear();
    arcLengthLUT.push_back(0.0f);

    glm::vec3 prevPoint = calculateBezierPoint(0.0f, m_controlPoints);
    float currentLen = 0.0f;

    for (int i = 1; i <= lutResolution; i++) {
        float t = (float)i / (float)lutResolution;
        glm::vec3 pt = calculateBezierPoint(t, m_controlPoints);
        currentLen += glm::length(pt - prevPoint);
        arcLengthLUT.push_back(currentLen);
        prevPoint = pt;
    }
    totalCurveLength = currentLen;
}

float Scene::getTForDistance(float targetDistance) {
    if (targetDistance <= 0.0f) return 0.0f;
    if (targetDistance >= totalCurveLength) return 1.0f;

    for (size_t i = 0; i < arcLengthLUT.size() - 1; i++) {
        if (targetDistance >= arcLengthLUT[i] && targetDistance <= arcLengthLUT[i+1]) {
            float segmentLength = arcLengthLUT[i+1] - arcLengthLUT[i];
            float segmentFraction = (targetDistance - arcLengthLUT[i]) / segmentLength;
            float t0 = (float)i / (float)(arcLengthLUT.size() - 1);
            float t1 = (float)(i + 1) / (float)(arcLengthLUT.size() - 1);
            return t0 + segmentFraction * (t1 - t0);
        }
    }
    return 1.0f;
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

Scene::Scene() : lightPos(0.0f, -1.0f, -12.0f) {
    unsigned int vertexCount = sizeof(cubeVertices) / sizeof(float);
    lampMesh  = new Mesh(cubeVertices, vertexCount, true);

    std::vector<glm::vec3> bezierPoints;
    int numSegments = 500;
    bezierPointCount = numSegments + 1;

    std::vector<glm::vec3> controlPoints = {
        glm::vec3(15.0f,  0.0f, -10.0f),
        glm::vec3(13.0f,  -1.0f, -10.0f),
        glm::vec3(10.0f,  0.8f, -10.0f),
        glm::vec3(5.5f,  0.0f, -10.0f),
        glm::vec3(9.0f,   0.65f, -15.0f),
        glm::vec3( 1.0f,  0.0f,  -15.0f),
        glm::vec3(0.0f, 0.2f, -20.0f),
        glm::vec3(-0.4f, 0.6f, -20.0f),
        glm::vec3(-1.5f, 2.0f, -13.5f),
        glm::vec3(-4.0f, 1.4f, -9.0f),
        glm::vec3(-6.0f, 0.7f, -9.2f),
        glm::vec3(-7.0f, 0.7f, -9.5f),
        glm::vec3(-8.5f, -1.5f, -10.0f),
        glm::vec3(-7.0f, 0.7f, -9.5f),
        glm::vec3(-12.0f, -1.2f, -10.5f),
        glm::vec3(-20.0f, -0.9f, -6.0f),
        glm::vec3(-18.0f, 0.0f, -5.0f),
        glm::vec3(-11.0f, 0.1f, -6.5f),
        glm::vec3(-13.0f, 0.0f, -15.0f),
        glm::vec3(-11.0f, 0.5f, -16.0f),
        glm::vec3(-13.0f, 0.5f, -17.5f),
        glm::vec3(-13.0f, -0.7f, -18.0f),
        glm::vec3(-11.5f, -1.5f, -20.0f),
        glm::vec3(5.0f, -1.5f, -35.0f),
        glm::vec3(5.5f, -1.8f, -38.0f),
        glm::vec3(-35.0f, 0.4f, -36.0f),
        glm::vec3(-30.0f, 1.0f, -14.0f),
        glm::vec3(-19.0f, 0.5f, -9.0f),
        glm::vec3(-14.0f, 0.0f, -18.0f),
        glm::vec3(-1.0f, 0.0f, -18.0f),
        glm::vec3(0.0f, 0.0f, -18.0f),
        glm::vec3(1.0f, 0.0f, -18.0f),
        glm::vec3(4.0f, 0.0f, -18.0f),
        glm::vec3(7.0f, 0.5f, -18.0f),
        glm::vec3(10.0f, 1.0f, -18.0f),
        glm::vec3(20.0f, 0.0f, -22.5f),
        glm::vec3(19.0f, -0.5f, -25.5f),
        glm::vec3(19.0f, -0.5f, -20.0f),
        glm::vec3(18.0f, -2.0f, -18.5f),
        glm::vec3(17.0f, -3.0f, -14.0f),
        glm::vec3(17.0f, -0.7f, -14.25f),
        glm::vec3(17.0f, 0.7f, -14.5f),
        glm::vec3(17.0f, 1.0f, -11.0f),
        glm::vec3(17.0f, 1.0f, -10.0f),
        glm::vec3(15.0f,  0.0f, -10.0f),
    };

    m_controlPoints = controlPoints;
    buildLUT(1000);

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
        Bee = nullptr;
        std::cerr << "Village model niet gevonden!" << std::endl;
    }

    // Load all exported Bee mesh parts if present.
    std::string BeePath = "";
    std::vector<std::string> BeeBases = {
        "models/Bee-1",
        "../models/Bee-1"
    };

    for (const std::string& base : BeeBases) {
        const std::string path = base + "/minecraft_bee.glb";
        if (std::filesystem::exists(path)) {
            BeePath = path;
            break;
        }
    }

    if (!BeePath.empty()) {
        Bee = new Model(BeePath);
        std::cout << "Bee geladen: " << BeePath << std::endl;
    } else {
        Bee = nullptr;
        std::cerr << "Bee model niet gevonden!" << std::endl;
    }
}

void Scene::setLightUniforms(Shader& shader) {
    // 1. GLOBAAL LICHT (Maan / Nacht lucht)
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  glm::vec3(0.05f, 0.05f, 0.1f)); // Donkerblauwige nacht
    shader.setVec3("light.diffuse",  glm::vec3(0.2f, 0.2f, 0.3f));   // Zwak maanlicht
    shader.setVec3("light.specular", glm::vec3(0.1f, 0.1f, 0.1f));

    // 2. LANTAARNS (Point Lights)
    // LET OP: Je moet deze coördinaten waarschijnlijk nog even tweaken 
    // zodat ze precies overeenkomen met de plekken van jouw Village lantaarns!
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 2.0f,  0.5f, -15.0f),  // Lantaarn 1
        glm::vec3(-11.0f,  0.5f, -15.0f),  // Lantaarn 2
        glm::vec3( 11.0f,  0.5f,  -15.0f),  // Lantaarn 3
        glm::vec3(-2.0f,  0.5f,  -15.0f)   // Lantaarn 4
    };

    // Stuur de data in een loopje naar de shader
    for(int i = 0; i < 4; i++) {
        std::string number = std::to_string(i);
        
        // Positie doorgeven
        shader.setVec3("pointLights[" + number + "].position", pointLightPositions[i]);
        
        // Kleur instellen (Mooi warm oranje/geel Minecraft licht)
        shader.setVec3("pointLights[" + number + "].ambient",  glm::vec3(0.1f, 0.05f, 0.0f));
        shader.setVec3("pointLights[" + number + "].diffuse",  glm::vec3(1.0f, 0.6f, 0.2f));
        shader.setVec3("pointLights[" + number + "].specular", glm::vec3(1.0f, 0.8f, 0.5f));
        
        // Afzwakking berekening (Deze waarden geven een licht-radius van ongeveer 15-20 meter)
        shader.setFloat("pointLights[" + number + "].constant",  1.0f);
        shader.setFloat("pointLights[" + number + "].linear",    0.09f);
        shader.setFloat("pointLights[" + number + "].quadratic", 0.032f);
    }
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
    glDisable(GL_CULL_FACE);
    if (Village != nullptr) {
        // Bereken de positie/schaal van het hele dorp
        glm::mat4 VillageModel = glm::mat4(1.0f);
        VillageModel = glm::translate(VillageModel, glm::vec3(0.0f, -3.0f, -10.0f));
        VillageModel = glm::rotate(VillageModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        VillageModel = glm::scale(VillageModel, glm::vec3(5.0f));

        for (unsigned int i = 0; i < Village->meshes.size(); i++) {
            
            if (i == 5 || i == 7 || i == 8) {
                
                lampShader.use();
                lampShader.setMat4("model", VillageModel);
                lampShader.setMat4("view", view);
                lampShader.setMat4("projection", projection);
                
                Village->meshes[i].Draw(lampShader);

            } else {
                
                lightingShader.use();
                lightingShader.setMat4("model", VillageModel);
                lightingShader.setVec3("material.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
                lightingShader.setVec3("material.diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
                lightingShader.setVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
                lightingShader.setFloat("material.shininess", 10.0f);
                
                Village->meshes[i].Draw(lightingShader);
            }
        }
    }
    glEnable(GL_CULL_FACE);
    
    // --- Bee model ---
    if (Bee != nullptr) {
        // tijd en snelheid
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        float speed = 0.8f;
        currentDistance += speed * deltaTime;

        // loop
        if (currentDistance > totalCurveLength) {
            currentDistance = fmod(currentDistance, totalCurveLength);
        }

        float t = getTForDistance(currentDistance);
        
        glm::vec3 beePos = calculateBezierPoint(t, m_controlPoints);

        float tNext = getTForDistance(currentDistance + 0.1f); 
        if (tNext < t) tNext = 1.0f;
        glm::vec3 nextPos = calculateBezierPoint(tNext, m_controlPoints);
        glm::vec3 direction = glm::normalize(nextPos - beePos);
        
        float yaw = atan2(direction.x, direction.z) + glm::radians(180.0f);

        float xzLength = sqrt(direction.x * direction.x + direction.z * direction.z); 
        float pitch = atan2(direction.y, xzLength);

        glm::mat4 BeeModel = glm::mat4(1.0f);
        BeeModel = glm::translate(BeeModel, beePos);
        BeeModel = glm::rotate(BeeModel, yaw, glm::vec3(0.0f, 1.0f, 0.0f));   
        BeeModel = glm::rotate(BeeModel, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        BeeModel = glm::scale(BeeModel, glm::vec3(0.01f));

        lightingShader.setMat4("model", BeeModel);
        lightingShader.setVec3("material.ambient",  glm::vec3(0.10f, 0.10f, 0.10f));
        lightingShader.setVec3("material.diffuse",  glm::vec3(0.35f, 0.35f, 0.38f));
        lightingShader.setVec3("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
        lightingShader.setFloat("material.shininess", 40.0f);
        Bee->Draw(lightingShader);
    }

    // --- lamp ---
    // lampShader.use();
    // glm::mat4 lampModel = glm::scale(glm::translate(glm::mat4(1.0f), lightPos), glm::vec3(0.2f));
    // lampShader.setMat4("model",      lampModel);
    // lampShader.setMat4("view",       view);
    // lampShader.setMat4("projection", projection);
    // lampMesh->Draw();

    // --- bezier curve ---
    // lampShader.use(); 
    // lampShader.setMat4("model", glm::mat4(1.0f));
    // lampShader.setMat4("view", view);
    // lampShader.setMat4("projection", projection);

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
    if (Bee != nullptr) {
        Bee->Delete();
        delete Bee;
    }
}