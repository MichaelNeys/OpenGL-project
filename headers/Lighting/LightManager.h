#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../shader.h"

class LightManager {
public:
    LightManager();
    void setLightUniforms(Shader& shader, const glm::vec3& globalLightPos, bool redstoneLampsOn);

private:
    std::vector<std::string> u_plPos, u_plAmb, u_plDif, u_plSpe;
    std::vector<std::string> u_plConst, u_plLin, u_plQuad;
};