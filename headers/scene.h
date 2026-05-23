#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "skybox.h"
#include "terrain.h"

class Scene {
public:
    Scene();
    void Draw(Shader& lightingShader, Shader& lampShader,
              glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);
    void Delete();
    bool redstoneLampsOn = true;
    void checkMouseClick(glm::mat4 view, glm::mat4 projection, glm::vec3 cameraPos);

private:
    void setLightUniforms(Shader& shader);

    glm::vec3 lightPos;

    Skybox*  skybox  = nullptr;
    Terrain* terrain = nullptr;

    Model* Village = nullptr;
    Model* Bee     = nullptr;
    Mesh*  lampMesh = nullptr;

    unsigned int bezierVAO, bezierVBO;
    int bezierPointCount;

    std::vector<glm::vec3> m_controlPoints;
    std::vector<float>     arcLengthLUT;
    float totalCurveLength = 0.0f;
    float currentDistance  = 0.0f;

    float getTForDistance(float targetDistance);
    void  buildLUT(int lutResolution);
    std::vector<glm::vec3> pollenPositions;
    unsigned int crosshairVAO, crosshairVBO;

    std::vector<glm::vec3> redstoneLightPositions;
};