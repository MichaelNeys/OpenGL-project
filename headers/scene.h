#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>
#include <mesh.h>
#include <model.h>
#include <vector>
#include <string>
#include <deque>

class Scene {
public:
    glm::vec3 lightPos;

    Scene();
    void Draw(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);
    void Delete();

private:
    Mesh* lampMesh;
    void setLightUniforms(Shader& shader);

    unsigned int bezierVAO;
    unsigned int bezierVBO;
    unsigned int bezierPointCount;

    Model* Village = nullptr;
    unsigned int VillageTexture = 0;
    Model* Bee;
};

#endif