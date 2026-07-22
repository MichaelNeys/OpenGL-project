#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shader.h"

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces,
           const std::string& vertPath,
           const std::string& fragPath);
    ~Skybox();

    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int VAO, VBO, cubemapTexture;
    Shader* shader = nullptr;
    unsigned int loadCubemap(const std::vector<std::string>& faces);
};