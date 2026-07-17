#include "Lighting/LightManager.h"
#include "GeometryData.h"

LightManager::LightManager() {
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

void LightManager::setLightUniforms(Shader& shader, const glm::vec3& globalLightPos, bool redstoneLampsOn) {
    shader.setVec3("light.position", globalLightPos);
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