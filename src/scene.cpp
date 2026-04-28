// src/scene.cpp
#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <algorithm>

namespace {
constexpr std::size_t kInitialDronePartsToLoad = 3;
constexpr std::size_t kMaxDronePartsToLoad = 18;
constexpr std::size_t kDronePartsLoadedPerFrame = 1;
}

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
    lampMesh  = new Mesh(cubeVertices, vertexCount, true);

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

    // Load all exported Atomium mesh parts if present.
    std::vector<std::string> atomiumBases = {
        "models/Atomium",
        "../models/Atomium"
    };
    const std::unordered_set<std::string> sphereFiles = {
        "model_1.obj",
        "model_2.obj",
        "model_4.obj",
        "model_5.obj",
        "model_6.obj",
        "model_11.obj",
        "model_14.obj",
        "model_15.obj"
    };

    for (const std::string& base : atomiumBases) {
        bool foundAny = false;
        for (int i = 0; i < 64; ++i) {
            const std::string path = base + "/model_" + std::to_string(i) + ".obj";
            if (!std::filesystem::exists(std::filesystem::path(path))) {
                if (i > 0) {
                    break;
                }
                continue;
            }
            Model* loadedModel = new Model(path);
            const std::string fileName = std::filesystem::path(path).filename().string();
            const bool isSphere = sphereFiles.find(fileName) != sphereFiles.end();
            atomiumParts.push_back({loadedModel, isSphere, path});
            foundAny = true;
        }
        if (foundAny) {
            break;
        }
    }
    if (atomiumParts.empty()) {
        std::cerr << "Geen Atomium OBJ-bestanden gevonden in models/Atomium" << std::endl;
    } else {
        std::cout << "Atomium delen geladen: " << atomiumParts.size() << std::endl;
        for (size_t i = 0; i < atomiumParts.size(); ++i) {
            std::cout
                << "  [" << i << "] "
                << atomiumParts[i].filePath
                << " -> "
                << (atomiumParts[i].isSphere ? "SPHERE" : "BAR")
                << std::endl;
        }
    }

    // Load all exported Drone mesh parts if present.
    std::vector<std::string> droneBases = {
        "models/Drone",
        "../models/Drone"
    };
    struct DroneCandidate {
        std::string path;
        std::uintmax_t sizeBytes;
    };
    std::vector<DroneCandidate> droneCandidates;
    for (const std::string& base : droneBases) {
        for (int i = 0; i < 256; ++i) {
            const std::string path = base + "/model_" + std::to_string(i) + ".obj";
            const std::filesystem::path fsPath(path);
            if (!std::filesystem::exists(fsPath)) {
                if (i > 0) {
                    break;
                }
                continue;
            }

            std::error_code fileSizeError;
            const std::uintmax_t fileSize = std::filesystem::file_size(fsPath, fileSizeError);
            droneCandidates.push_back({path, fileSizeError ? 0 : fileSize});
        }
        if (!droneCandidates.empty()) {
            break;
        }
    }

    std::sort(droneCandidates.begin(), droneCandidates.end(),
        [](const DroneCandidate& a, const DroneCandidate& b) {
            return a.sizeBytes > b.sizeBytes;
        });

    const std::size_t initialParts = std::min(kInitialDronePartsToLoad, droneCandidates.size());
    const std::size_t maxParts = std::min(kMaxDronePartsToLoad, droneCandidates.size());
    for (std::size_t i = 0; i < initialParts; ++i) {
        Model* loadedModel = new Model(droneCandidates[i].path);
        droneParts.push_back({loadedModel, droneCandidates[i].path});
    }
    for (std::size_t i = initialParts; i < maxParts; ++i) {
        pendingDronePaths.push_back(droneCandidates[i].path);
    }
    if (droneParts.empty()) {
        std::cerr << "Geen Drone OBJ-bestanden gevonden in models/Drone" << std::endl;
    } else {
        std::cout << "Drone direct geladen: " << droneParts.size() << std::endl;
        std::cout << "Drone in wachtrij: " << pendingDronePaths.size() << std::endl;
        std::cout << "Drone bronbestanden gevonden: " << droneCandidates.size() << std::endl;
    }
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
    for (std::size_t i = 0; i < kDronePartsLoadedPerFrame && !pendingDronePaths.empty(); ++i) {
        const std::string nextPath = pendingDronePaths.front();
        pendingDronePaths.pop_front();
        Model* loadedModel = new Model(nextPath);
        droneParts.push_back({loadedModel, nextPath});
    }

    lightingShader.use();
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt("texture_diffuse1", 0);

    // --- atomium model ---
    glm::mat4 atomiumModel = glm::mat4(1.0f);
    atomiumModel = glm::translate(atomiumModel, glm::vec3(0.0f, -0.5f, -4.0f));
    atomiumModel = glm::scale(atomiumModel, glm::vec3(0.35f));
    lightingShader.setMat4("model", atomiumModel);
    for (const AtomiumPart& part : atomiumParts) {
        if (part.isSphere) {
            // Shiny aluminum-like spheres.
            lightingShader.setVec3("material.ambient",  glm::vec3(0.30f, 0.30f, 0.32f));
            lightingShader.setVec3("material.diffuse",  glm::vec3(0.92f, 0.92f, 0.95f));
            lightingShader.setVec3("material.specular", glm::vec3(0.95f, 0.95f, 0.98f));
            lightingShader.setFloat("material.shininess", 140.0f);
        } else {
            // Matte white/gray bars.
            lightingShader.setVec3("material.ambient",  glm::vec3(0.24f, 0.24f, 0.24f));
            lightingShader.setVec3("material.diffuse",  glm::vec3(0.90f, 0.90f, 0.90f));
            lightingShader.setVec3("material.specular", glm::vec3(0.04f, 0.04f, 0.04f));
            lightingShader.setFloat("material.shininess", 10.0f);
        }
        part.model->Draw(lightingShader);
    }

    // --- drone model ---
    glm::mat4 droneModel = glm::mat4(1.0f);
    droneModel = glm::translate(droneModel, glm::vec3(1.0f, 1.8f, -3.0f));
    droneModel = glm::scale(droneModel, glm::vec3(0.03f));
    lightingShader.setMat4("model", droneModel);
    lightingShader.setVec3("material.ambient",  glm::vec3(0.10f, 0.10f, 0.10f));
    lightingShader.setVec3("material.diffuse",  glm::vec3(0.35f, 0.35f, 0.38f));
    lightingShader.setVec3("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
    lightingShader.setFloat("material.shininess", 40.0f);
    for (const DronePart& part : droneParts) {
        part.model->Draw(lightingShader);
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
    for (const AtomiumPart& part : atomiumParts) {
        part.model->Delete();
        delete part.model;
    }
    atomiumParts.clear();
    for (const DronePart& part : droneParts) {
        part.model->Delete();
        delete part.model;
    }
    droneParts.clear();
}